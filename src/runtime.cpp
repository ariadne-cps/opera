/***************************************************************************
 *            runtime.cpp
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

#include <cmath>
#include "runtime.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

String construct_thread_name(String prefix, SizeType number, SizeType max_number) {
    std::ostringstream ss;
    ss << prefix;
    if (max_number > 9 and number <= 9) ss << "0";
    ss << number;
    return ss.str();
}

RuntimeConfiguration::RuntimeConfiguration() :
    _job_factory(ReuseLookAheadJobFactory(AddWhenDifferentMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG)),
    _history_retention(3600),
    _history_purge_period(300),
    _concurrency(std::thread::hardware_concurrency()) { }

LookAheadJobFactory const& RuntimeConfiguration::get_job_factory() const {
    return _job_factory;
}

TimestampType const& RuntimeConfiguration::get_history_retention() const {
    return _history_retention;
}

TimestampType const& RuntimeConfiguration::get_history_purge_period() const {
    return _history_purge_period;
}

SizeType const& RuntimeConfiguration::get_concurrency() const {
    return _concurrency;
}

RuntimeConfiguration& RuntimeConfiguration::set_job_factory(LookAheadJobFactory const& factory) {
    _job_factory = factory;
    return *this;
}

RuntimeConfiguration& RuntimeConfiguration::set_history_retention(TimestampType const& retention) {
    OPERA_PRECONDITION(retention > _history_purge_period)
    _history_retention = retention;
    return *this;
}

RuntimeConfiguration& RuntimeConfiguration::set_history_purge_period(TimestampType const& purge_period) {
    OPERA_PRECONDITION(purge_period < _history_retention)
    _history_purge_period = purge_period;
    return *this;
}

RuntimeConfiguration& RuntimeConfiguration::set_concurrency(SizeType const& concurrency) {
    OPERA_PRECONDITION(concurrency > 0)
    OPERA_PRECONDITION(concurrency <= std::thread::hardware_concurrency())
    _concurrency = concurrency;
    return *this;
}

Runtime::Runtime(BrokerAccess const& access, LookAheadJobFactory const& factory, SizeType const& concurrency) :
    Runtime({access,BodyPresentationTopic::DEFAULT},{access,HumanStateTopic::DEFAULT},{access,RobotStateTopic::DEFAULT},{access,CollisionNotificationTopic::DEFAULT},factory,concurrency) { }

Runtime::Runtime(Pair<BrokerAccess,BodyPresentationTopic> const& bp_subscriber, Pair<BrokerAccess,HumanStateTopic> const& hs_subscriber,
                 Pair<BrokerAccess,RobotStateTopic> const& rs_subscriber, Pair<BrokerAccess,CollisionNotificationTopic> const& cn_publisher,
                 LookAheadJobFactory const& factory, SizeType const& concurrency) :
    _waiting_jobs([&]{ _availability_condition.notify_one(); }),
    _sleeping_jobs([]{}),
    _stop(false),
    _receiver(bp_subscriber,hs_subscriber,rs_subscriber,factory,_registry,_waiting_jobs, _sleeping_jobs),
    _sender(cn_publisher),
    _num_processing(0),
    _num_processed(0),
    _num_completed(0),
    _num_collisions(0)
{
    for (SizeType i=0; i<concurrency; ++i)
        _threads.emplace_back(new Thread([&]{
            CONCLOG_SCOPE_CREATE
            while(true) {
                {
                    std::unique_lock<std::mutex> lock(_availability_mutex);
                    _availability_condition.wait(lock, [=, this] {
                        bool can_reserve = _waiting_jobs.can_reserve();
                        if (can_reserve) { ++_num_processing; _waiting_jobs.reserve(); }
                        return _stop or can_reserve; });
                }
                if (_stop) return;
                _process_one_working_job();
                --_num_processing;
                CONCLOG_SCOPE_PRINTHOLD("#w=" << _waiting_jobs.size() << ", #s=" << _sleeping_jobs.size())
            }
        }, construct_thread_name("la-",i,concurrency)));
}

SizeType Runtime::num_segment_pairs() const {
    return _registry.num_segment_pairs();
}

SizeType Runtime::num_pending_human_robot_pairs() const {
    return _receiver.num_pending_human_robot_pairs();
}

SizeType Runtime::num_waiting_jobs() const {
    std::unique_lock<std::mutex> lock(_availability_mutex);
    return _waiting_jobs.size();
}

SizeType Runtime::num_sleeping_jobs() const {
    return _sleeping_jobs.size();
}

void Runtime::__test__process_one_working_job() {
    _waiting_jobs.reserve();
    _process_one_working_job();
}

void Runtime::_process_one_working_job() {
    CONCLOG_SCOPE_CREATE
    auto job = _waiting_jobs.dequeue();
    auto const& robot_history = _registry.robot_history(job.id().robot());
    auto human_keypoint_data = _registry.get_human_keypoint_ids(job.id().human(),job.id().human_segment());
    if (not get<0>(human_keypoint_data)) {
        CONCLOG_PRINTLN("Aborting working job since human has been removed")
        return;
    }
    auto const& robot = _registry.robot(job.id().robot());
    auto const& message_frequency = _registry.robot(job.id().robot()).message_frequency();

    ++_num_processed;

    CONCLOG_PRINTLN("Processing job {" << job.id() << ":" << job.path() << "} at " << job.initial_time() << " with trace of size " << job.prediction_trace().size() <<
                                           " from " << job.prediction_trace().at(0).mode << " to " << job.prediction_trace().ending_mode())

    auto earliest_collision_idx = job.earliest_collision_index(robot_history);
    if (earliest_collision_idx >= 0) {
        auto robot_history_snapshot = robot_history.snapshot_at(job.snapshot_time());
        Interval<SizeType> samples_between_modes(static_cast<SizeType>(earliest_collision_idx));

        auto sample_index = robot_history_snapshot.checked_sample_index(job.prediction_trace().starting_mode(),job.initial_time());
        auto initial_range = robot_history_snapshot.range_of_num_samples_in(job.prediction_trace().starting_mode());
        if (job.prediction_trace().size() == 1) samples_between_modes = samples_between_modes - sample_index;
        else if (sample_index > initial_range.lower()) samples_between_modes = samples_between_modes + Interval<SizeType>(0,initial_range.upper()-sample_index);
        else samples_between_modes = samples_between_modes + initial_range - sample_index;
        for (SizeType i=1; i<job.prediction_trace().size()-1; ++i)
            samples_between_modes = samples_between_modes + robot_history_snapshot.range_of_num_samples_in(job.prediction_trace().at(i).mode);

        auto lower_collision_distance = static_cast<TimestampType>(std::round(static_cast<FloatType>(1000*samples_between_modes.lower())/message_frequency));
        auto upper_collision_distance = static_cast<TimestampType>(std::round(static_cast<FloatType>(1000*samples_between_modes.upper())/message_frequency));

        Pair<KeypointIdType,KeypointIdType> human_segment = {get<1>(human_keypoint_data),get<2>(human_keypoint_data)};
        Pair<KeypointIdType,KeypointIdType> robot_segment = {robot.segment(job.id().robot_segment()).head_id(),robot.segment(job.id().robot_segment()).tail_id()};

        _sender.put(CollisionNotificationMessage(job.id().human(), human_segment, job.id().robot(), robot_segment, job.initial_time(), {lower_collision_distance,upper_collision_distance}, job.prediction_trace().ending_mode(), job.prediction_trace().likelihood()));

        std::ostringstream collision_ss;
        if (lower_collision_distance == upper_collision_distance) {
            if (lower_collision_distance == 0) collision_ss << " immediately in " << job.prediction_trace().ending_mode();
            else collision_ss << " after " << ((FloatType)lower_collision_distance)/1000 << " seconds in trace of size " << job.prediction_trace().size() << " ending with " << job.prediction_trace().ending_mode();
        } else collision_ss << " after [" << ((FloatType)lower_collision_distance)/1000 << ":" << ((FloatType)upper_collision_distance)/1000 << "] seconds in trace of size " << job.prediction_trace().size() << " ending with " << job.prediction_trace().ending_mode();

        CONCLOG_PRINTLN("Notification sent for {" << job.id() << ":" << job.path() << "} from " << job.initial_time() << collision_ss.str() << " (~" << job.prediction_trace().likelihood() << ")")
        ++_num_completed;
        ++_num_collisions;
        if (_registry.has_human(job.id().human()))
            _sleeping_jobs.enqueue(job);
    } else if (_registry.has_human(job.id().human())) {
        auto next_jobs = _receiver.factory().create_next_jobs(job, robot_history);
        CONCLOG_PRINTLN("No collision found, handling " << next_jobs.size() << " next jobs")
        if (next_jobs.empty()) { ++_num_completed; _sleeping_jobs.enqueue(job); }
        else for (auto const& nj : next_jobs) {
            if (nj.path().size() <= job.path().size() or
               (nj.path().size() > job.path().size() and not _receiver.factory().has_registered(nj.initial_time(),nj.id(),nj.path())))
                    _waiting_jobs.enqueue(nj);
        }
    }
}

Runtime::~Runtime() noexcept {
    _stop = true;
    _availability_condition.notify_all();
}

}