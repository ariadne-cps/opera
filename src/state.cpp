/***************************************************************************
 *            state.cpp
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
#include "macros.hpp"
#include "state.hpp"
#include "conclog/include/logging.hpp"
#include <iostream>

using namespace ConcLog;

namespace Opera {

HumanStateInstance::HumanStateInstance(Human const& human, Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp) : _timestamp(timestamp) {
    for (SizeType i=0; i<human.num_segments(); ++i) {
        auto const& segment = human.segment(i);
        auto head = points.find(segment.head_id());
        auto tail = points.find(segment.tail_id());
        List<Point> head_pts = (head != points.end() ? head->second : List<Point>());
        List<Point> tail_pts = (tail != points.end() ? tail->second : List<Point>());
        BodySegmentSample sample = segment.create_sample();
        sample.update(head_pts,tail_pts);
        _samples.push_back(sample);
    }
}

List<BodySegmentSample> const& HumanStateInstance::samples() const {
    return _samples;
}

TimestampType const& HumanStateInstance::timestamp() const {
    return _timestamp;
}

HumanStateHistory::HumanStateHistory(Human const& human) : _human(human) { }

void HumanStateHistory::acquire(Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp) {
    _instances.push_back({_human,points,timestamp});
}

HumanStateInstance const& HumanStateHistory::latest_within(TimestampType const& timestamp) const {
    OPERA_PRECONDITION(not _instances.empty())
    for (auto it = _instances.crbegin(); it != _instances.crend(); ++it)
        if (it->timestamp() <= timestamp) return *it;
    OPERA_FAIL_MSG("No human instance could be found for timestamp " << timestamp)
}

TimestampType const& HumanStateHistory::latest_time() const {
    OPERA_PRECONDITION(not _instances.empty())
    return _instances.back().timestamp();
}

TimestampType const& HumanStateHistory::earliest_time() const {
    OPERA_PRECONDITION(not _instances.empty())
    return _instances.front().timestamp();
}

bool HumanStateHistory::has_instances_within(TimestampType const& timestamp) const {
    for (auto const& instance : _instances) if (instance.timestamp() <= timestamp) return true;
    return false;
}

SizeType HumanStateHistory::instance_distance(TimestampType const& lower, TimestampType const& upper) const {
    OPERA_PRECONDITION(not _instances.empty())
    OPERA_PRECONDITION(lower <= upper)
    auto upper_it = _instances.crbegin();
    for (; upper_it != _instances.crend(); ++upper_it)
        if (upper_it->timestamp() == upper) break;
    OPERA_ASSERT_MSG(upper_it != _instances.crend(), "Upper timestamp " << upper << " not found in the human instances.")
    auto lower_it = upper_it;
    for (; lower_it != _instances.crend(); ++lower_it)
        if (lower_it->timestamp() == lower) break;
    OPERA_ASSERT_MSG(lower_it != _instances.crend(), "Lower timestamp " << lower << " not found in the human instances.")
    return static_cast<SizeType>(lower_it - upper_it);
}

SizeType HumanStateHistory::instance_number(TimestampType const& timestamp) const {
    for (auto it = _instances.crbegin(); it != _instances.crend(); ++it)
        if (it->timestamp() == timestamp) return _instances.size()-1-static_cast<SizeType>(it-_instances.crbegin());
    OPERA_FAIL_MSG("No instance found with timestamp " << timestamp)
}

HumanStateInstance const& HumanStateHistory::at(SizeType const& idx) const {
    return _instances.at(idx);
}

SizeType HumanStateHistory::size() const {
    return _instances.size();
}

void HumanStateHistory::remove_older_than(TimestampType const& timestamp) {
    while(not _instances.empty() and earliest_time() < timestamp) {
        _instances.pop_front();
    }
}

RobotModePresence::RobotModePresence(Mode const& mode, Mode const& exit_destination, TimestampType const& from, TimestampType const& to) :
_mode(mode), _exit_destination(exit_destination), _from(from), _to(to) { }

Mode const& RobotModePresence::mode() const {
    return _mode;
}

Mode const& RobotModePresence::exit_destination() const {
    return _exit_destination;
}

TimestampType const& RobotModePresence::from() const {
    return _from;
}

TimestampType const& RobotModePresence::to() const {
    return _to;
}

std::ostream& operator<<(std::ostream& os, RobotModePresence const& p) {
    return os << "(within '" << p.mode() << "' in [" << p.from() << "," << p.to() << "), exiting to '" << p.exit_destination() << "')";
}

auto SamplesHistory::at(TimestampType const& timestamp) const -> BodySamplesType const& {
    OPERA_PRECONDITION(not _entries.empty())
    SizeType i=0;
    for (; i<_entries.size(); ++i) {
        if (_entries.at(i).first > timestamp) break;
    }
    OPERA_ASSERT_MSG(i>0,"No samples history found at " << timestamp)
    return _entries.at(i-1).second;
}

bool SamplesHistory::has_samples_at(TimestampType const& timestamp) const {
    for (auto const& e : _entries) {
        if (e.first <= timestamp) return true;
    }
    return false;
}

void SamplesHistory::append(TimestampType const& timestamp, BodySamplesType const& samples) {
    _entries.emplace_back(timestamp,samples);
}

SizeType SamplesHistory::size_at(TimestampType const& timestamp) const {
    return at(timestamp).at(0).size();
}

RobotStateHistory::RobotStateHistory(Robot const& robot) :
    _latest_time(0), _current_mode_states_buffer(List<List<BodySegmentSample>>()), _mode_traces(), _robot(robot) {
    for (SizeType i=0; i < _robot.num_segments(); ++i)
        _current_mode_states_buffer.push_back(List<BodySegmentSample>());
    _mode_traces.emplace_back(0,ModeTrace());
}

TimestampType const& RobotStateHistory::latest_time() const {
    return _latest_time;
}

TimestampType const& RobotStateHistory::earliest_time() const {
    OPERA_PRECONDITION(not _mode_presences.empty())
    return _mode_presences.front().from();
}

void RobotStateHistory::remove_older_than(TimestampType const& timestamp) {
    OPERA_PRECONDITION(timestamp > 0)
    while (not _mode_presences.empty() and _mode_presences.front().to() < timestamp) {
        _mode_presences.pop_front();
    }
    while (not _mode_traces.empty() and _mode_traces.front().first < timestamp) {
        _mode_traces.pop_front();
    }
    SizeType desired_size = 1;
    SizeType current_size = _mode_traces.front().second.size();
    for (auto& t : _mode_traces) {
        if (t.second.size() > current_size) {
            current_size = t.second.size();
            ++desired_size;
        }
        t.second.reduce_between(current_size-desired_size,current_size-1);
    }
}

SizeType RobotStateHistory::size() const {
    return _mode_presences.size();
}

Mode const& RobotStateHistory::latest_mode() const {
    return _latest_mode;
}

Mode const& RobotStateHistory::mode_at(TimestampType const& time) const {
    for (auto const& p : _mode_presences)
        if (p.from() <= time and time < p.to())
            return p.mode();
    return _latest_mode;
}

void RobotStateHistory::acquire(Mode const& mode, Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp) {
    /*
     * 1) If the mode is different from the current one (including the first mode inserted)
     *   a) Save the buffered content
     *   b) Create a new is_empty buffered content
     *   c) Add the mode presence
     *   d) Update the current mode
     * 3) Check if the mode has a history
     *   a) If not, adding each segment sample to the buffer
     *   b) If it has, identify the index from the timestamp and update the sample on the corresponding entry, adding it to the buffer
     */
    OPERA_ASSERT(points.size() == _robot.num_points())

    auto const& latest_time_snapshot = snapshot_at(timestamp);

    if (_latest_mode.is_empty() or _latest_mode != mode) {
        if (not _latest_mode.is_empty()) {
            std::lock_guard<std::mutex> lock(_states_mux);
            auto unrounded_index = latest_time_snapshot.unrounded_sample_index(_latest_mode, timestamp);
            if (not _current_mode_states_buffer.empty()) {
                auto last_state_idx = _current_mode_states_buffer.at(0).size() - 1;
                if (unrounded_index > static_cast<FloatType>(last_state_idx+1)) {
                    auto idx_distance = static_cast<SizeType>(floor(unrounded_index)) - last_state_idx;
                    for (SizeType i=0; i<_robot.num_segments(); ++i) {
                        auto last_sample = _current_mode_states_buffer.at(i).at(last_state_idx);
                        for (SizeType j=0; j < idx_distance; ++j)
                            _current_mode_states_buffer.at(i).push_back(last_sample);
                    }
                }
            }
            _mode_states[_latest_mode].append(timestamp,_current_mode_states_buffer);
            CONCLOG_PRINTLN_AT(1,"Added snapshot at " << timestamp << " for " << _latest_mode)
        }

        if (_mode_states.has_key(mode)) {
            _current_mode_states_buffer = _mode_states[mode].at(timestamp);
        } else {
            _current_mode_states_buffer = List<List<BodySegmentSample>>();
            for (SizeType i=0; i < _robot.num_segments(); ++i)
                _current_mode_states_buffer.push_back(List<BodySegmentSample>());
        }

        TimestampType entrance_timestamp = (_mode_presences.empty() ? timestamp : _mode_presences.back().to());
        {
            std::lock_guard<std::mutex> lock(_presences_mux);
            _mode_presences.emplace_back(RobotModePresence(_latest_mode, mode, entrance_timestamp, timestamp));
            if (not _latest_mode.is_empty()) {
                auto trace = _mode_traces.back().second;
                trace.push_back(_latest_mode,1.0);
                _mode_traces.emplace_back(timestamp,trace);
            }
        }

        _latest_mode = mode;
    }
    _latest_time = timestamp;

    SizeType update_idx = _current_mode_states_buffer.at(0).size();
    int idx_distance = 1;
    if (_mode_states.has_key(_latest_mode)) {
        update_idx = latest_time_snapshot.sample_index(_latest_mode, timestamp);
        idx_distance = static_cast<int>(floor(update_idx)) - static_cast<int>((_mode_states[_latest_mode].size_at(timestamp)-1));
    }

    for (SizeType i=0; i<_robot.num_segments(); ++i) {
        auto const& head_pts = points.at(_robot.segment(i).head_id());
        auto const& tail_pts = points.at(_robot.segment(i).tail_id());
        for (int j=0; j<idx_distance-1; ++j)
            _current_mode_states_buffer.at(i).push_back(_current_mode_states_buffer.at(i).at(_current_mode_states_buffer.at(i).size()-1));
        if (idx_distance > 0) _current_mode_states_buffer.at(i).push_back(_robot.segment(i).create_sample());
        _current_mode_states_buffer.at(i).at(update_idx).update(head_pts,tail_pts);
    }
}

RobotStateHistorySnapshot RobotStateHistory::snapshot_at(TimestampType const& timestamp) const {
    return RobotStateHistorySnapshot(*this,timestamp);
}

RobotStateHistorySnapshot::RobotStateHistorySnapshot(RobotStateHistory const& history, TimestampType const& timestamp) :
        _history(history), _snapshot_time(timestamp) { }

ModeTrace const& RobotStateHistorySnapshot::mode_trace() const {
    std::lock_guard<std::mutex> lock(_history._presences_mux);
    SizeType i=0;
    for (; i<_history._mode_traces.size(); ++i) {
        if (_history._mode_traces.at(i).first > _snapshot_time) break;
    }
    return _history._mode_traces.at(i-1).second;
}

Set<Mode> RobotStateHistorySnapshot::modes_with_samples() const {
    Set<Mode> result;
    {
        std::lock_guard<std::mutex> lock(_history._states_mux);
        for (auto const& m : _history._mode_states)
            if (m.second.has_samples_at(_snapshot_time)) result.insert(m.first);
    }
    return result;
}

bool RobotStateHistorySnapshot::can_look_ahead(TimestampType const& time) const {
    if (time > _history.latest_time()) return false;
    auto const& mode = _history.mode_at(time);
    if (not _history._mode_states.has_key(mode)) return false;
    if (not _history._mode_states.at(mode).has_samples_at(time)) return false;
    if (unrounded_sample_index(mode, time) >= range_of_num_samples_in(mode).upper()) return false;
    for (auto const& p : _history._mode_presences) {
        if (p.from() >= _snapshot_time) break;
        if ((not p.mode().is_empty()) and p.mode() == mode and time > p.to()) return true;
    }
    return false;
}

auto RobotStateHistorySnapshot::samples(Mode const& mode) const -> BodySamplesType const& {
    std::lock_guard<std::mutex> lock(_history._states_mux);
    return _history._mode_states.at(mode).at(_snapshot_time);
}

SizeType RobotStateHistorySnapshot::maximum_number_of_samples(Mode const& mode) const {
    std::lock_guard<std::mutex> lock(_history._states_mux);
    return _history._mode_states.at(mode).size_at(_snapshot_time);
}

List<RobotModePresence> RobotStateHistorySnapshot::presences_in(Mode const& mode) const {
    List<RobotModePresence> result;
    for (auto const& p : _history._mode_presences)
        if ((not p.mode().is_empty()) and p.mode() == mode and p.to() <= _snapshot_time)
            result.push_back(p);
    return result;
}

List<RobotModePresence> RobotStateHistorySnapshot::presences_between(Mode const& source, Mode const& destination) const {
    List<RobotModePresence> result;
    for (auto const& p : _history._mode_presences)
        if ((not p.mode().is_empty()) and p.mode() == source and p.exit_destination() == destination and p.to() <= _snapshot_time)
            result.push_back(p);
    return result;
}

List<RobotModePresence> RobotStateHistorySnapshot::presences_exiting_into(Mode const& mode) const {
    List<RobotModePresence> result;
    for (auto const& p : _history._mode_presences)
        if (p.exit_destination() == mode and p.to() <= _snapshot_time)
            result.push_back(p);
    return result;
}

Interval<SizeType> RobotStateHistorySnapshot::_range_of_num_samples_within(List<RobotModePresence> const& presences) const {
    if (presences.empty())
        return {0u,0u};

    SizeType min_value = std::numeric_limits<SizeType>::max();
    SizeType max_value = 0;
    for (auto const& p : presences) {
        auto val = static_cast<SizeType>(floor(static_cast<double>(p.to()-p.from())/1000*_history._robot.message_frequency()));
        min_value = std::min(min_value,val);
        max_value = std::max(max_value,val);
    }
    return {min_value,max_value};
}

Interval<SizeType> RobotStateHistorySnapshot::range_of_num_samples_in(Mode const& mode) const {
    return _range_of_num_samples_within(presences_in(mode));
}

Interval<SizeType> RobotStateHistorySnapshot::range_of_num_samples_in(Mode const& mode, Mode const& target) const {
    return _range_of_num_samples_within(presences_between(mode, target));
}

FloatType RobotStateHistorySnapshot::unrounded_sample_index(Mode const& mode, TimestampType const& timestamp) const {
    TimestampType entry_time = timestamp + 1;
    auto it = _history._mode_presences.crbegin();
    if (timestamp >= it->to()) {
        entry_time = it->to();
    } else {
        for (; it != _history._mode_presences.crend() - 1; ++it) {
            if (it->mode() == mode and it->from() <= timestamp and it->to() > timestamp) {
                entry_time = it->from();
                break;
            }
        }
    }
    OPERA_ASSERT_MSG(entry_time <= timestamp, "No presence for " << mode << " was found to identify the sample index")
    return FloatType(timestamp - entry_time) / 1000 * FloatType(_history._robot.message_frequency());
}

SizeType RobotStateHistorySnapshot::sample_index(Mode const& mode, TimestampType const& timestamp) const {
    return static_cast<SizeType>(floor(unrounded_sample_index(mode, timestamp)));
}

SizeType RobotStateHistorySnapshot::checked_sample_index(Mode const& mode, TimestampType const& timestamp) const {
    auto result = sample_index(mode,timestamp);
    OPERA_ASSERT_MSG(result < maximum_number_of_samples(mode), "The sample index must be lower than the number of states in the given mode, instead " << result << " >= " << maximum_number_of_samples(mode) << ".")
    return result;
}

}