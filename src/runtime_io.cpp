/***************************************************************************
 *            runtime_io.cpp
 *
 *  Copyright  2021  Luca Geretti
 *
 ****************************************************************************/

/*
 * This file is part of Opera, under the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "runtime_io.tpl.hpp"
#include "deserialisation.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

RuntimeReceiver::RuntimeReceiver(Pair<BrokerAccess,BodyPresentationTopic> const& bp_subscriber, Pair<BrokerAccess,HumanStateTopic> const& hs_subscriber, Pair<BrokerAccess,RobotStateTopic> const& rs_subscriber,
                                 LookAheadJobFactory const& factory, TimestampType const& history_retention, TimestampType  const& history_purge_period,
                                 BodyRegistry& registry, SynchronisedQueue<LookAheadJob>& waiting_jobs, SynchronisedQueue<LookAheadJob>& sleeping_jobs) :
    _factory(factory), _history_retention(history_retention), _history_purge_period(history_purge_period),
    _bp_subscriber(bp_subscriber.first.make_body_presentation_subscriber([&](auto const& msg){
        if (not registry.contains(msg.id())) {
            CONCLOG_PRINTLN_AT(2,"Registering body " << msg.id())
            if (msg.is_human()) for (auto const& rid : registry.robot_ids()) _pending_human_robot_pairs.push_back({msg.id(), rid});
            else for (auto const& hid : registry.human_ids()) _pending_human_robot_pairs.push_back({msg.id(), hid});
            registry.insert(msg);
        }
    },bp_subscriber.second)),
    _hs_subscriber(hs_subscriber.first.make_human_state_subscriber([&](auto const& msg){
        std::lock_guard<std::mutex> lock(_state_received_mux);
        for (auto const& bd : msg.bodies()) {
            auto const& hid = bd.first;
            if (registry.contains(hid)) {
                CONCLOG_PRINTLN_AT(2,"Received human state for " << hid << " from message at " << msg.timestamp())
            } else {
                CONCLOG_PRINTLN_AT(2,"Received human state for unknown " << hid << " from message at " << msg.timestamp() << ", registering it using the default human")
                for (auto const& rid : registry.robot_ids()) _pending_human_robot_pairs.push_back({hid, rid});
                auto pr = Deserialiser<BodyPresentationMessage>(Resources::path("json/default_human.json")).make();
                registry.insert_human(hid,pr.segment_pairs(),pr.thicknesses());
            }
        }
        registry.acquire_state(msg);
        _remove_old_history(registry,msg);
        _remove_unresponding_humans(msg.timestamp(),registry,sleeping_jobs);
        _move_sleeping_jobs_to_waiting_jobs(registry, sleeping_jobs, waiting_jobs);
        _promote_pairs_to_jobs(registry, sleeping_jobs, waiting_jobs);
        ++_num_state_messages_received;
    },hs_subscriber.second)),
    _rs_subscriber(rs_subscriber.first.make_robot_state_subscriber([&](auto const& msg){
        std::lock_guard<std::mutex> lock(_state_received_mux);
        if (registry.contains(msg.id())) {
            CONCLOG_PRINTLN_AT(2,"Received robot state for " << msg.id() << " from message at " << msg.timestamp())
            registry.acquire_state(msg);
            _remove_old_history(registry,msg);
            _remove_unresponding_humans(msg.timestamp(),registry,sleeping_jobs);
            _move_sleeping_jobs_to_waiting_jobs(registry, sleeping_jobs, waiting_jobs);
            _promote_pairs_to_jobs(registry, sleeping_jobs, waiting_jobs);
        } else {
            CONCLOG_PRINTLN_AT(2,"Discarded robot state message for " << msg.id() << " since the body is not registered")
        }
        ++_num_state_messages_received;
    },rs_subscriber.second))
{
}

RuntimeReceiver::~RuntimeReceiver() noexcept {
    delete _bp_subscriber;
    delete _hs_subscriber;
    delete _rs_subscriber;
}

SizeType RuntimeReceiver::num_pending_human_robot_pairs() const {
    std::lock_guard<std::mutex> lock(_pairs_mux);
    return _pending_human_robot_pairs.size();
}

TimestampType RuntimeReceiver::oldest_history_time() const {
    return _oldest_history_time;
}

void RuntimeReceiver::_remove_old_history(BodyRegistry& registry, HumanStateMessage const& msg) {
    for (auto const& bd : msg.bodies()) {
        auto& history = registry.human_history(bd.first);
        if (msg.timestamp() - history.earliest_time() > 1000*(_history_retention+_history_purge_period)) {
            history.remove_older_than(msg.timestamp()-_history_retention*1000);
            _oldest_history_time = history.earliest_time();
        }
    }
}

void RuntimeReceiver::_remove_old_history(BodyRegistry& registry, RobotStateMessage const& msg) {
    auto& history = registry.robot_history(msg.id());
    if (msg.timestamp() - history.earliest_time() > 1000*(_history_retention+_history_purge_period)) {
        history.remove_older_than(msg.timestamp()-_history_retention*1000);
        _oldest_history_time = history.earliest_time();
    }
}

void RuntimeReceiver::_promote_pairs_to_jobs(BodyRegistry const& registry, SynchronisedQueue<LookAheadJob>& sleeping_jobs, SynchronisedQueue<LookAheadJob>& waiting_jobs) {
    List<HumanRobotIdPair> new_pairs;
    std::lock_guard<std::mutex> lock(_pairs_mux);
    for (auto const& p : _pending_human_robot_pairs) {
        auto const& robot_latest_time = registry.robot_history(p.robot).latest_time();
        if (registry.has_human_instances_within(p.human, robot_latest_time)) {
            auto human_latest_instance = registry.latest_human_instance_within(p.human, robot_latest_time);
            auto const& timestamp = human_latest_instance.timestamp();
            auto robot_history_snapshot = registry.robot_history(p.robot).snapshot_at(timestamp);
            if (robot_history_snapshot.can_look_ahead(timestamp)) {
                auto const& human = registry.human(p.human);
                auto const& robot = registry.robot(p.robot);
                auto const& mode = registry.robot_history(p.robot).mode_at(timestamp);
                for (SizeType i=0; i<human.num_segments(); ++i)
                    for (SizeType j=0; j<robot.num_segments(); ++j) {
                        auto job = _factory.create_new_job({human.id(), human.segment(i).index(), robot.id(),
                                                            robot.segment(j).index()}, timestamp, human_latest_instance.samples().at(
                                human.segment(i).index()), ModeTrace().push_back(mode), LookAheadJobPath());
                        if (job.human_sample().is_empty()) sleeping_jobs.enqueue(job);
                        else waiting_jobs.enqueue(job);
                    }
                CONCLOG_PRINTLN("Human-robot pair {" << human.id() << "," << robot.id() << "} inserted as " << human.num_segments()*robot.num_segments() << " new jobs at " << timestamp)
            } else new_pairs.emplace_back(p);
        } else new_pairs.emplace_back(p);
    }
    _pending_human_robot_pairs = new_pairs;
}

void RuntimeReceiver::_remove_unresponding_humans(TimestampType const& latest_msg_timestamp, BodyRegistry& registry, SynchronisedQueue<LookAheadJob>& sleeping_jobs) {

    auto hids = registry.human_ids();
    List<BodyIdType> hids_to_remove;
    for (auto const& hid : hids) {
        if (registry.human_history_size(hid) > 0) {
            const TimestampType latest_human_timestamp = registry.latest_human_timestamp(hid);
            if (latest_msg_timestamp > latest_human_timestamp and (latest_msg_timestamp - latest_human_timestamp) > HUMAN_RETENTION_TIMEOUT) {
                registry.remove(hid);
                CONCLOG_PRINTLN("Removed human " << hid << " due to no state messages received in the last " << HUMAN_RETENTION_TIMEOUT << " ms")
                hids_to_remove.push_back(hid);
            }
        }
    }

    if (not hids_to_remove.empty()) {
        {
            std::lock_guard<std::mutex> lock(_pairs_mux);
            List<HumanRobotIdPair> new_pending_human_robot_pairs;
            for (auto const& pair : _pending_human_robot_pairs) {
                if (find(hids_to_remove.cbegin(),hids_to_remove.cend(),pair.human) == hids_to_remove.cend())
                    new_pending_human_robot_pairs.push_back(pair);
            }
            _pending_human_robot_pairs = new_pending_human_robot_pairs;
        }

        List<LookAheadJob> new_jobs;
        while (sleeping_jobs.size() > 0) {
            sleeping_jobs.reserve();
            auto job = sleeping_jobs.dequeue();
            if (find(hids_to_remove.cbegin(),hids_to_remove.cend(),job.id().human()) == hids_to_remove.cend())
                new_jobs.emplace_back(job);
        }
        for (auto const& job : new_jobs) sleeping_jobs.enqueue(job);
    }
}

void RuntimeReceiver::_move_sleeping_jobs_to_waiting_jobs(BodyRegistry const& registry, SynchronisedQueue<LookAheadJob>& sleeping_jobs, SynchronisedQueue<LookAheadJob>& waiting_jobs) {
    List<LookAheadJob> jobs_to_keep, jobs_to_move;
    while(sleeping_jobs.size() > 0) {
        sleeping_jobs.reserve();
        auto job = sleeping_jobs.dequeue();
        auto const& robot_history = registry.robot_history(job.id().robot());
        auto robot_latest_time = robot_history.latest_time();
        auto human_latest_instance = registry.latest_human_instance_within(job.id().human(),robot_latest_time);
        auto const& timestamp = human_latest_instance.timestamp();
        auto instance_distance = registry.instance_distance(job.id().human(),job.initial_time(),timestamp);
        auto robot_history_snapshot = robot_history.snapshot_at(job.snapshot_time());
        if (instance_distance > 0 and robot_history_snapshot.can_look_ahead(timestamp)) {
            auto woken = _factory.awaken(job, timestamp, human_latest_instance.samples().at(job.id().human_segment()),
                                         robot_history);
            for (auto const& wj : woken) {
                if (wj.second == JobAwakeningResult::DIFFERENT) { jobs_to_move.emplace_back(wj.first); }
                else { jobs_to_keep.emplace_back(wj.first); }
            }
        } else { jobs_to_keep.emplace_back(job); }
    }
    for (auto const& job : jobs_to_keep) { sleeping_jobs.enqueue(std::move(job)); }
    for (auto const& job : jobs_to_move) { waiting_jobs.enqueue(std::move(job)); }
}

RuntimeSender::RuntimeSender(Pair<BrokerAccess,CollisionNotificationTopic> const& publisher) :
    _collision_notifications(),
    _publisher(publisher.first.make_collision_notification_publisher(publisher.second)),
    _stop(false),
    _thr([&]{
        while (true) {
            std::unique_lock<std::mutex> lock(_availability_mutex);
            _availability_condition.wait(lock, [=, this] { return _stop or _collision_notifications.size() > 0; });
            if (_stop) return;
            _collision_notifications.reserve();
            _publisher->put(_collision_notifications.dequeue());
        }
    },"rt_send") { }

RuntimeSender::~RuntimeSender() noexcept {
    _stop = true;
    _availability_condition.notify_one();
    delete _publisher;
}

}