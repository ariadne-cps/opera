/***************************************************************************
 *            lookahead_job.cpp
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

#include "macros.hpp"
#include "lookahead_job.hpp"
#include "body_registry.hpp"
#include "conclog/include/logging.hpp"
#include <iostream>

using namespace ConcLog;

namespace Opera {

LookAheadJobPath& LookAheadJobPath::add(PriorityType const& priority, PredictionTracePosition const& trace_position) {
    OPERA_PRECONDITION(trace_position > 0)
    auto size = _path.size();
    OPERA_ASSERT_MSG(size == 0 or _path.at(size-1).second < trace_position,"The trace position must be greater than the previous element in the path.")
    _path.push_back(std::make_pair(priority,trace_position));
    return *this;
}

LookAheadJobPath& LookAheadJobPath::remove_le_than(PredictionTracePosition const& trace_position) {
    LookAheadJobPath new_path;
    for (auto const& e : _path)
        if (e.second > trace_position) new_path.add(e.first, e.second-trace_position);
    *this = new_path;
    return *this;
}

LookAheadJobPath& LookAheadJobPath::remove_g_than(PredictionTracePosition const& trace_position) {
    while ((not _path.empty()) and _path.back().second > trace_position)
        _path.pop_back();
    return *this;
}

LookAheadJobPath& LookAheadJobPath::reduce_between(PredictionTracePosition const& lower_trace_position, PredictionTracePosition const& upper_trace_position) {
    return remove_g_than(upper_trace_position).remove_le_than(lower_trace_position);
}

SizeType LookAheadJobPath::size() const {
    return _path.size();
}

auto LookAheadJobPath::priority(SizeType const& index) const -> PriorityType const& {
    OPERA_PRECONDITION(index < this->size())
    return _path.at(index).first;
}

bool LookAheadJobPath::is_primary() const {
    for (auto const& e : _path)
        if (e.first > 0) return false;
    return true;
}

std::ostream& operator<<(std::ostream& os, LookAheadJobPath const& path) {
    SizeType const size = path._path.size();
    if (size == 0) return os << "[]";
    os << "[";
    for (SizeType i=0; i < size-1; ++i)
        os << path._path.at(i).first << ":" << path._path.at(i).second << ",";
    return os << path._path.at(size-1).first << ":" << path._path.at(size-1).second << "]";
}

LookAheadJobIdentifier::LookAheadJobIdentifier(BodyIdType const& human, IdType const& human_segment, BodyIdType const& robot, IdType const& robot_segment)
    : _human(human), _human_segment(human_segment), _robot(robot), _robot_segment(robot_segment) { }

BodyIdType const& LookAheadJobIdentifier::human() const {
    return _human;
}

IdType const& LookAheadJobIdentifier::human_segment() const {
    return _human_segment;
}

BodyIdType const& LookAheadJobIdentifier::robot() const {
    return _robot;
}

IdType const& LookAheadJobIdentifier::robot_segment() const {
    return _robot_segment;
}

bool operator==(const LookAheadJobIdentifier& id1, const LookAheadJobIdentifier& id2) {
    return id1.human() == id2.human() and id1.human_segment() == id2.human_segment() and id1.robot() == id2.robot() and id1.robot_segment() == id2.robot_segment();
}

bool operator<(const LookAheadJobIdentifier& id1, const LookAheadJobIdentifier& id2) {
    if (id1.human() < id2.human()) return true;
    else if (id1.human() > id2.human()) return false;
    else if (id1.human_segment() < id2.human_segment()) return true;
    else if (id1.human_segment() > id2.human_segment()) return false;
    else if (id1.robot() < id2.robot()) return true;
    else if (id1.robot() > id2.robot()) return false;
    else if (id1.robot_segment() < id2.robot_segment()) return true;
    else return false;
}

std::ostream& operator<<(std::ostream& os, LookAheadJobIdentifier const& id) {
    return os << id.human() << "@" << id.human_segment() << "+" << id.robot() << "@" << id.robot_segment();
}

LookAheadJobBase::LookAheadJobBase(LookAheadJobIdentifier const& id, TimestampType const& initial_time, TimestampType const& snapshot_time, BodySegmentSample const& human_sample, ModeTrace const& prediction_trace, LookAheadJobPath const& path)
        : _id(id), _initial_time(initial_time),_snapshot_time(snapshot_time), _human_sample(human_sample), _prediction_trace(prediction_trace), _path(path) { }

LookAheadJobIdentifier const& LookAheadJobBase::id() const {
    return _id;
}

TimestampType const& LookAheadJobBase::initial_time() const {
    return _initial_time;
}

TimestampType const& LookAheadJobBase::snapshot_time() const {
    return _snapshot_time;
}

BodySegmentSample const& LookAheadJobBase::human_sample() const {
    return _human_sample;
}

ModeTrace const& LookAheadJobBase::prediction_trace() const {
    return _prediction_trace;
}

LookAheadJobPath const& LookAheadJobBase::path() const {
    return _path;
}

DiscardLookAheadJob::DiscardLookAheadJob(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& prediction_trace, LookAheadJobPath const& path)
    : LookAheadJobBase(id, initial_time, initial_time, human_sample, prediction_trace, path) { }

int DiscardLookAheadJob::earliest_collision_index(RobotStateHistory const& robot_history) const {
    auto const& mode_to_look = prediction_trace().ending_mode();
    auto robot_history_snapshot = robot_history.snapshot_at(_snapshot_time);
    auto const samples = robot_history_snapshot.samples(mode_to_look).at(id().robot_segment());

    OPERA_ASSERT_MSG(samples.size() > 0, "Should not have empty samples when checking for collision index")

    SizeType lower = 0;
    SizeType upper = samples.size()-1;
    if (mode_to_look == _prediction_trace.starting_mode()) {
        auto bound = robot_history_snapshot.checked_sample_index(mode_to_look, _initial_time);
        if (_prediction_trace.size() == 1) lower = bound;
        else {
            if (bound == 0) return -1;
            else upper = bound - 1;
        }
    }

    for (SizeType i=lower; i<=upper; ++i) {
        auto const& robot_segment = samples.at(i);
        if (not robot_segment.is_empty() and _human_sample.intersects(robot_segment)) { return static_cast<int>(i); }
    }

    return -1;
}

ReuseLookAheadJob::ReuseLookAheadJob(LookAheadJobIdentifier const& id, TimestampType const& initial_time, TimestampType const& snapshot_time, BodySegmentSample const& human_sample, ModeTrace const& prediction_trace, LookAheadJobPath const& path, MinimumDistanceBarrierSequence const& barrier_sequence)
        : LookAheadJobBase(id, initial_time, snapshot_time, human_sample, prediction_trace, path), _barrier_sequence(barrier_sequence) { }

MinimumDistanceBarrierSequence const& ReuseLookAheadJob::barrier_sequence() const {
    return _barrier_sequence;
}

int ReuseLookAheadJob::earliest_collision_index(RobotStateHistory const& robot_history) const {
    CONCLOG_SCOPE_CREATE
    auto const& mode_to_look = prediction_trace().ending_mode();
    auto const trace_index = prediction_trace().size()-1;
    auto robot_history_snapshot = robot_history.snapshot_at(_snapshot_time);
    auto const samples = robot_history_snapshot.samples(mode_to_look).at(id().robot_segment());

    OPERA_ASSERT_MSG(samples.size() > 0, "Should not have empty samples when checking for collision index")

    SizeType lower = (_barrier_sequence.is_empty() or _barrier_sequence.last_upper_trace_index() != trace_index) ? 0 : _barrier_sequence.last_barrier().range().maximum_sample_index() + 1;
    SizeType upper = samples.size()-1;
    if (mode_to_look == _prediction_trace.starting_mode()) {
        auto bound = robot_history_snapshot.checked_sample_index(mode_to_look, _initial_time);
        if (_prediction_trace.size() == 1) lower = std::max(lower,bound);
        else {
            if (bound == 0) return -1;
            else upper = bound-1;
        }
    }

    CONCLOG_PRINTLN("Checking earliest collision index for trace index " << trace_index << " in [" << lower << "," << upper << "]")

    for (SizeType i=lower; i<=upper; ++i) {
        auto const& robot_sample = samples.at(i);
        if (not robot_sample.is_empty() and not _barrier_sequence.check_and_update(_human_sample,robot_sample,{trace_index,i})) { return static_cast<int>(i); }
    }

    return -1;
}

}