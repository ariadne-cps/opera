/***************************************************************************
 *            lookahead_job_factory.cpp
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

#include "lookahead_job_factory.hpp"
#include "body_registry.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

List<LookAheadJob> LookAheadJobFactoryBase::create_next(LookAheadJob const& job, RobotStateHistory const& robot_history) const {
    List<LookAheadJob> result;
    auto const& prediction_trace = job.prediction_trace();
    if (prediction_trace.has_looped()) return {};

    // We still use the snapshot at the initial time to properly merge the traces
    auto full_trace = merge(robot_history.snapshot_at(job.initial_time()).mode_trace(),prediction_trace);
    auto next_modes = full_trace.next_modes();
    OPERA_ASSERT_MSG(not next_modes.empty(), "The next modes of a proper trace can never be empty.")
    auto const num_modes = next_modes.size();

    LookAheadJobPath::PriorityType priority = 0;
    for (auto const& next : next_modes) {
        auto trace = prediction_trace;
        trace.push_back(next.first,next.second);
        auto new_path = job.path();
        if (num_modes > 1) new_path.add(priority++,trace.size()-1);
        result.emplace_back(create_from_existing(job, trace, new_path));
    }
    return result;
}

LookAheadJob DiscardLookAheadJobFactory::create_new(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& mode_trace, LookAheadJobPath const& path) const {
    return DiscardLookAheadJob(id, initial_time, human_sample, mode_trace, path);
}

LookAheadJob DiscardLookAheadJobFactory::create_from_existing(LookAheadJob const& job, ModeTrace const& new_mode_trace, LookAheadJobPath const& new_path) const {
    return DiscardLookAheadJob(job.id(), job.initial_time(), job.human_sample(), new_mode_trace, new_path);
}

List<Pair<LookAheadJob,JobAwakeningResult>> DiscardLookAheadJobFactory::awaken(LookAheadJob const& job, TimestampType const& time, BodySegmentSample const& human_sample, RobotStateHistory const& robot_history) const {
    CONCLOG_SCOPE_CREATE
    CONCLOG_PRINTLN("Awakening dscrd job " << job.id() << ":" << job.path() << " at " << job.initial_time() << " with trace of size " << job.prediction_trace().size() <<
                                           " from " << job.prediction_trace().at(0).mode << " to " << job.prediction_trace().ending_mode())
    auto const& mode_to_start = robot_history.mode_at(time);
    if (job.initial_time() < time) {
        if (human_sample.is_empty()) return {{DiscardLookAheadJob(job.id(), time, job.human_sample(), job.prediction_trace(), job.path()), JobAwakeningResult::UNCOMPUTABLE}};
        if (not job.path().is_primary()) return {{}};
        return {{DiscardLookAheadJob(job.id(), time, human_sample, ModeTrace().push_back(mode_to_start, 1.0), LookAheadJobPath()), JobAwakeningResult::DIFFERENT}};
    }
    return {{job,JobAwakeningResult::UNAFFECTED}};
}

bool DiscardLookAheadJobFactory::has_registered(TimestampType const&, LookAheadJobIdentifier const&, LookAheadJobPath const&) const {
    return false;
}

ReuseLookAheadJobFactory::ReuseLookAheadJobFactory(MinimumDistanceBarrierSequenceUpdatePolicy const& update_policy, ReuseEquivalence const& equivalence) :
    _registry(std::make_shared<LookAheadJobRegistry>()), _update_policy(update_policy), _equivalence(equivalence) { }

LookAheadJob ReuseLookAheadJobFactory::create_new(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& mode_trace, LookAheadJobPath const& path) const {
    bool registered = _registry->try_register(initial_time,id,path);
    OPERA_ASSERT_MSG(registered,"Tried to create job already registered or that is unacceptable with respect to the job registry")
    return ReuseLookAheadJob(id, initial_time, initial_time, human_sample, mode_trace, path, MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(), _update_policy));
}

LookAheadJob ReuseLookAheadJobFactory::create_from_existing(LookAheadJob const& job, ModeTrace const& new_mode_trace, LookAheadJobPath const& new_path) const {
    return ReuseLookAheadJob(job.id(), job.initial_time(), job.snapshot_time(), job.human_sample(), new_mode_trace, new_path,
                             dynamic_cast<ReuseLookAheadJob const *>(job.ptr())->barrier_sequence());
}

List<Pair<LookAheadJob,JobAwakeningResult>> ReuseLookAheadJobFactory::awaken(LookAheadJob const& job, TimestampType const& time, BodySegmentSample const& human_sample, RobotStateHistory const& robot_history) const {
    CONCLOG_SCOPE_CREATE
    CONCLOG_PRINTLN("Awakening reuse job " << job.id() << ":" << job.path() << " at " << job.initial_time() << " with trace of size " << job.prediction_trace().size() <<
                    " from " << job.prediction_trace().at(0).mode << " to " << job.prediction_trace().ending_mode())
    auto const& mode_to_start = robot_history.mode_at(time);
    CONCLOG_PRINTLN_AT(1,"Awakening into " << time << " in " << mode_to_start)
    if (job.initial_time() < time) {

        auto prediction_trace = job.prediction_trace();
        auto path = job.path();
        auto barrier_sequence = dynamic_cast<ReuseLookAheadJob const *>(job.ptr())->barrier_sequence();
        auto snapshot_time = (_equivalence == ReuseEquivalence::STRONG ? time : job.snapshot_time());

        CONCLOG_PRINTLN_AT(3,"Barrier sequence:" << barrier_sequence)

        if (not barrier_sequence.is_empty()) {
            CONCLOG_PRINTLN_AT(2,"Barrier sequence covering up to " << barrier_sequence.last_section().last_barrier().range().maximum_trace_index() << "@" << barrier_sequence.last_section().last_barrier().range().maximum_sample_index())
        } else {
            CONCLOG_PRINTLN_AT(2,"Barrier sequence starts empty")
        }

        if (human_sample.is_empty()) {
            CONCLOG_PRINTLN_AT(1,"Human sample is empty, keeping traces the same")
            _registry->try_register(time,job.id(),path); // Will always be satisfied
            return {{ReuseLookAheadJob(job.id(), time, snapshot_time, job.human_sample(), prediction_trace, path, barrier_sequence), JobAwakeningResult::UNCOMPUTABLE}};
        }

        auto int_lower_trace_index = prediction_trace.forward_index(mode_to_start);
        if (int_lower_trace_index < 0) {
            CONCLOG_PRINTLN_AT(1,"Could not find the mode to start in the current prediction trace, will restart")
            prediction_trace = ModeTrace().push_back(mode_to_start);
            barrier_sequence.clear();
            path = LookAheadJobPath();
            snapshot_time = time;
        } else {
            auto lower_trace_index = static_cast<SizeType>(int_lower_trace_index);
            SizeType reset_upper_trace_index = prediction_trace.size()-1;
            if (_equivalence == ReuseEquivalence::STRONG and lower_trace_index > 0) {
                CONCLOG_PRINTLN_AT(2,"Barrier sequence: " << barrier_sequence)
                for (SizeType i=0; i<lower_trace_index; ++i) {
                    auto backward_index = prediction_trace.backward_index(prediction_trace.at(i).mode);
                    if (backward_index > static_cast<int>(i)) {
                        reset_upper_trace_index = std::min(reset_upper_trace_index, static_cast<SizeType>(backward_index-1));
                    }
                }
                CONCLOG_PRINTLN_AT(1,"Under strong equivalence, reducing the barrier sequence in [" << lower_trace_index << "," << reset_upper_trace_index << "]")
            }
            auto robot_history_snapshot = robot_history.snapshot_at(snapshot_time);
            auto start_sample_index = robot_history_snapshot.checked_sample_index(mode_to_start, time);
            barrier_sequence.reset(human_sample,{lower_trace_index,reset_upper_trace_index},start_sample_index);
            if (barrier_sequence.is_empty()) {
                CONCLOG_PRINTLN_AT(1,"Barrier trace is reset to empty, will restart")
                prediction_trace = ModeTrace().push_back(mode_to_start);
                path = LookAheadJobPath();
                snapshot_time = time;
            } else {
                CONCLOG_PRINTLN_AT(2,"Barrier sequence reset up to " << barrier_sequence.last_section().last_barrier().range().maximum_trace_index() << "@" << barrier_sequence.last_section().last_barrier().range().maximum_sample_index())
                auto upper_trace_index = lower_trace_index+barrier_sequence.last_upper_trace_index();
                auto const& mode_to_reuse = prediction_trace.at(upper_trace_index).mode;
                if (barrier_sequence.last_barrier().range().maximum_sample_index() == robot_history_snapshot.samples(mode_to_reuse).at(job.id().robot_segment()).size()-1) upper_trace_index++;

                if (upper_trace_index == prediction_trace.size()) {
                    CONCLOG_PRINTLN_AT(1,"Updating needs to find the next modes from the current trace")
                    prediction_trace.reduce_between(lower_trace_index, upper_trace_index - 1);
                    path.reduce_between(lower_trace_index, upper_trace_index);

                    List<Pair<LookAheadJob,JobAwakeningResult>> result;
                    auto jobs = create_next(ReuseLookAheadJob(job.id(), time, snapshot_time, human_sample, prediction_trace, path, barrier_sequence), robot_history);
                    if (jobs.empty()) {
                        result.emplace_back(ReuseLookAheadJob(job.id(), time, snapshot_time, human_sample, prediction_trace, path, barrier_sequence), JobAwakeningResult::COMPLETED);
                    } else for (auto const& next : jobs) {
                            if (_registry->try_register(time,job.id(),next.path()))
                                result.emplace_back(next,JobAwakeningResult::DIFFERENT);
                        }
                    return result;
                }
                CONCLOG_PRINTLN_AT(1,"Reducing the prediction trace between " << lower_trace_index << " and " << upper_trace_index)
                prediction_trace.reduce_between(lower_trace_index, upper_trace_index);
                path.reduce_between(lower_trace_index, upper_trace_index);
            }
        }

        if (_registry->try_register(time,job.id(),path))
            return {{ReuseLookAheadJob(job.id(), time, snapshot_time, human_sample, prediction_trace, path, barrier_sequence), JobAwakeningResult::DIFFERENT}};
        else
            return {{}};
    } else return {{job,JobAwakeningResult::UNAFFECTED}};
}

bool ReuseLookAheadJobFactory::has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const {
    return _registry->has_registered(timestamp, id, path);
}

}