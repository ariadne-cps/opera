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

HumanStateInstance::HumanStateInstance(Human const& human, List<List<Point>> const& points, TimestampType const& timestamp) : _timestamp(timestamp) {
    OPERA_PRECONDITION(human.num_points() == points.size())
    for (SizeType i=0; i<human.num_segments(); ++i) {
        auto const& segment = human.segment(i);
        auto head_pts = points.at(segment.head_id());
        auto tail_pts = points.at(segment.tail_id());
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

void HumanStateHistory::acquire(List<List<Point>> const& points, TimestampType const& timestamp) {
    _instances.push_back({_human,points,timestamp});
}

HumanStateInstance const& HumanStateHistory::latest_within(TimestampType const& timestamp) const {
    OPERA_PRECONDITION(not _instances.empty())
    for (auto it = _instances.crbegin(); it != _instances.crend(); ++it)
        if (it->timestamp() <= timestamp) return *it;
    OPERA_FAIL_MSG("No human instance could be found for timestamp " << timestamp)
}

List<SizeType> HumanStateHistory::idxs_within(TimestampType const& lower_timestamp, TimestampType const& higher_timestamp) const{
    OPERA_PRECONDITION(not _instances.empty())
    List<SizeType> result = List<SizeType>();
    for (auto it = _instances.crbegin(); it != _instances.crend(); ++it){
        if (it->timestamp() >= lower_timestamp && it->timestamp() <= higher_timestamp){
            SizeType idx = _instances.size()-1-static_cast<SizeType>(it-_instances.crbegin());
            result.push_back(idx);
        }
    }

    return result;
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

Mode const& RobotStateHistory::latest_mode() const {
    return _latest_mode;
}

Mode const& RobotStateHistory::mode_at(TimestampType const& time) const {
    for (auto const& p : _mode_presences)
        if (p.from() <= time and time < p.to())
            return p.mode();
    return _latest_mode;
}

void RobotStateHistory::acquire(Mode const& mode, List<List<Point>> const& points, TimestampType const& timestamp) {
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


// #~#v

bool RobotStateHistory::has_mode_at(TimestampType const& time) const {
    for (auto const& p : _mode_presences)
        if (p.from() <= time and time < p.to())
            return true;
    return false;
}

SamplesHistory RobotStateHistory::samples_history(Mode const& mode) const {
    auto it = _mode_states.cbegin();

    for (auto entry : _mode_states){
        if (entry.first == mode){
            return entry.second;
        }
    }
    return it->second;
}

Robot const& RobotStateHistory::get_robot() const{
    return _robot;
}


RobotPredictTiming::RobotPredictTiming(RobotStateHistorySnapshot const& snapshot, Mode const&target):
    _snapshot(snapshot), _robot(_snapshot.get_robot()), _target(target), _present_mode(_snapshot.latest_mode()){
        _common_constructor();
    }


RobotPredictTiming::RobotPredictTiming(RobotStateHistory const& history, Mode const& target):
    _snapshot(history.snapshot_at(history.latest_time())), _robot(_snapshot.get_robot()), _target(target), _present_mode(_snapshot.latest_mode()){
        _common_constructor();
}

void RobotPredictTiming::_common_constructor(){
    _extract_mode_trace();
    _index_present_mode = _mode_trace.size()-1;
    _branch_paths.push_back(_find_paths(_mode_trace.clone()));
    if (_set_best_path() < 0){
        impossible_prediction_flag = true;
        return;
    }
    _predict_timing();
}

ModeTrace RobotPredictTiming::_find_paths(ModeTrace trace){
    int current_depth = (int) (trace.size() - 1 - _index_present_mode);
    while (trace.at(trace.size()-1).mode != _target && current_depth <= _path_max_depth)
    {
        current_depth = (int) (trace.size() - 1 - _index_present_mode);
        if (trace.next_modes().size() == 1){
            trace.push_back(trace.next_modes().begin()->first, trace.next_modes().begin()->second);
        }
        else{
            bool first = true;
            auto next_modes = trace.next_modes();

            for (auto iterator = next_modes.begin(); iterator != next_modes.end(); iterator++){
                if (!iterator->first.is_empty() && first){
                    Mode mode_to_add = iterator->first;
                    PositiveFloatType probability_to_add = iterator->second;
                    trace.push_back(mode_to_add, probability_to_add);
                    first = false;
                }else if(! iterator->first.is_empty()){
                    ModeTrace clone = trace.clone();
                    clone.push_back(iterator->first, iterator->second);
                    _branch_paths.push_back(_find_paths(clone));
                }
            }
        }

    }
    return trace;
}

int RobotPredictTiming::_set_best_path(){
    PositiveFloatType best_likelihood = 0;
    for (ModeTrace path : _branch_paths){
        if (path.ending_mode() == _target && path.likelihood() > best_likelihood){
            _best_path = path;
            best_likelihood = path.likelihood();
        }
    }
    if (best_likelihood == 0){
        impossible_prediction_flag = true;
        return -1;
    }
    return 0;
}

void RobotPredictTiming::_predict_timing(){
    SizeType conversion_factor = 1000000000;
    SizeType n_samples = 0;
    SizeType frequency = _robot.message_frequency();

    for (SizeType i = _index_present_mode; i < _best_path.size()-1; ++i){
        auto range_of_n_samples_in = _snapshot.range_of_num_samples_in(_best_path.at(i).mode, _best_path.at(i+1).mode);
        auto upper_bound = range_of_n_samples_in.upper();
        auto lower_bound = range_of_n_samples_in.lower();
        auto samples_mean = (upper_bound + lower_bound) / 2;
        n_samples +=  samples_mean;
    }
    nanoseconds_to_mode = n_samples * frequency * conversion_factor;
}

/*void RobotPredictTiming::_augment_trace(){

        construct trace while making predictions to find the first
        occurrence of the target mode in the future. If a branch is detected
        in the prediction, takes the first path and adds the position of the
        branch and the path taken o the _branch_tracking map.
        If the target mode is not found in the current path when
        the counter reaches the max_depth variable, the trace gets
        deleted from the index of the last branch and the cycle
        restarts from that index, since that index is going to be present
        in the _branch tracking map, the next path is taken and the map is updated

    int depth_count = 0;
    int branch_to_take = 0;
    SizeType trace_index;
    std::cout << "Trace before augmentation: " << _mode_trace << std::endl;
    while (_mode_trace.at(_mode_trace.size()-1).mode != _target){
        trace_index = _mode_trace.size()-1;
        depth_count = trace_index - _index_present_mode;
        std::cout << "Current depth: " << depth_count << std::endl;
        if (_mode_trace.next_modes().size() != 1){
                std::cout << "Branch found! trace index: " << trace_index << " branch traking: ";
                for (auto entry : _branch_tracking){
                    std::cout << "{" << entry.first << ", " << entry.second << "} ";
            }
            std::cout<<std::endl;
            if(_branch_tracking.has_key(trace_index)){
                std::cout << "fetching path to take" << std::endl;
                branch_to_take = _branch_tracking.find(_mode_trace.size()-1)->second + 1;
                _branch_tracking.erase(trace_index);
            }else{
                branch_to_take = 0;
            }
            if ((int)_mode_trace.next_modes().size() > branch_to_take + 1){
                _branch_tracking.insert({trace_index, branch_to_take});
            }

        }else{
            branch_to_take = 0;
        }
        int branch_tmp_count = 0;
        Mode entry_to_add;
        PositiveFloatType probability_to_add;
        for (auto entry : _mode_trace.next_modes()){
            if (branch_tmp_count == branch_to_take){
                std::cout << "Taking branch " << branch_tmp_count << std::endl;
                entry_to_add = entry.first;
                probability_to_add = entry.second;
                break;
            }
            branch_tmp_count++;
        }
        _mode_trace.push_back(entry_to_add, probability_to_add);
        if ((_mode_trace.at(_mode_trace.size()-1).mode != _target) && depth_count > _max_depth_search){
            //_branch_tracking.erase(_branch_tracking.rbegin()->first);
            SizeType branch_index = _branch_tracking.rbegin()->first;
            std::cout << "trace index: " << trace_index << " branch index: " << branch_index << std::endl;
            for (SizeType i = trace_index; i >= branch_index; i--){
                _mode_trace.remove_at(i);
                std::cout << "removing trace entry " << i << std::endl;
            }
            if (_branch_tracking.size() == 0 && depth_count == _max_depth_search){
                std::cout << "Max depth reached while predicting the next occurrence of the target mode" << std::endl;
                break;
            }
        }
    }

    std::cout << "Trace after augmentation: " << _mode_trace << std::endl;

}*/

void RobotPredictTiming::_extract_mode_trace(){
    _mode_trace = _snapshot.mode_trace();
}

HumanRobotDistance::HumanRobotDistance(HumanStateHistory const& human_history, RobotStateHistorySnapshot const& robot_snapshot, IdType const& human_segment_id, IdType const& robot_segment_id, TimestampType const& lower_timestamp, TimestampType const& higher_timestamp):
_human_history(human_history), _robot_snapshot(robot_snapshot), _human_segment_id(human_segment_id), _robot_segment_id(robot_segment_id), _lower_timestamp(lower_timestamp), _higher_timestamp(higher_timestamp), _minimum_distances(List<FloatType>()){
    _set_human_instances();
    _compute_distances();
    _compute_min_max();
}

Interval<FloatType> HumanRobotDistance::get_min_max_distances() const{
    return *_min_max_distances;
}

void HumanRobotDistance::_set_human_instances(){
    auto idx_list = _human_history.idxs_within(_lower_timestamp, _higher_timestamp);
    for (auto idx : idx_list){
        _human_instances.push_back(_human_history.at(idx));
    }
}

/*
    in human state instance si può chiamare timestamp() per avere il timestamp
    e samples() per ottenere una lista di BodySegmentSample.
    su BodySegmentSample si può chiamare segment_id() per ottenere l'id del segment (IdType),
    head_centre() (Point), tail_centre() (Point), thickness()(FloatType)
*/

void HumanRobotDistance::_compute_distances(){
    for (HumanStateInstance instance : _human_instances){
        TimestampType timestamp = instance.timestamp();

        if (! _robot_snapshot.has_mode_at(timestamp)){
            continue;
        }

        Mode mode = _robot_snapshot.mode_at(timestamp);
        SamplesHistory robot_samples_history = _robot_snapshot.samples_history(mode);

        if (!robot_samples_history.has_samples_at(timestamp)){
            continue;
        }

        Point *robot_head;
        Point *robot_tail;
        FloatType *robot_segment_thickness;

        Point *human_head;
        Point *human_tail;
        FloatType *human_segment_thickness;

        BodySamplesType robot_body_sample = robot_samples_history.at(timestamp);
        for (auto segment_temporal_samples : robot_body_sample){
            for (auto body_segment_sample : segment_temporal_samples){
                if (body_segment_sample.segment_id() == _robot_segment_id){
                    *robot_head = body_segment_sample.head_centre();
                    *robot_tail = body_segment_sample.tail_centre();
                    *robot_segment_thickness = body_segment_sample.thickness();
                }
            }
        }

       for ( BodySegmentSample body_segment_sample : instance.samples()){
            if (body_segment_sample.segment_id() == _human_segment_id){
                *human_head = body_segment_sample.head_centre();
                *human_tail = body_segment_sample.tail_centre();
                *human_segment_thickness = body_segment_sample.thickness();
            }
       }

        FloatType segments_distance = distance(*human_head, *human_tail, *robot_head, *robot_tail);
        segments_distance = segments_distance - *human_segment_thickness - *robot_segment_thickness;
        _minimum_distances.push_back(segments_distance);
    }
}

void HumanRobotDistance::_compute_min_max(){
    FloatType min = -1;
    FloatType max = -1;

    for (FloatType distance : _minimum_distances){
        if (min == -1){
            min = distance;
            max = distance;
        }

        if (distance < min){
            min = distance;
        }
        if (distance > max){
            max = distance;
        }
    }

    _min_max_distances->set_lower(min);
    _min_max_distances->set_upper(max);
}



//#~#^



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
        auto val = static_cast<SizeType>(floor(static_cast<double>(p.to()-p.from())/1e9*_history._robot.message_frequency()));
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
    return FloatType(timestamp - entry_time) / 1e9 * FloatType(_history._robot.message_frequency());
}

SizeType RobotStateHistorySnapshot::sample_index(Mode const& mode, TimestampType const& timestamp) const {
    return static_cast<SizeType>(floor(unrounded_sample_index(mode, timestamp)));
}

SizeType RobotStateHistorySnapshot::checked_sample_index(Mode const& mode, TimestampType const& timestamp) const {
    auto result = sample_index(mode,timestamp);
    OPERA_ASSERT_MSG(result < maximum_number_of_samples(mode), "The sample index must be lower than the number of states in the given mode, instead " << result << " >= " << maximum_number_of_samples(mode) << ".")
    return result;
}

// #~#v

Mode const& RobotStateHistorySnapshot::mode_at(TimestampType const& time) const{
    return _history.mode_at(time);
}

bool RobotStateHistorySnapshot::has_mode_at(TimestampType const& time) const{
    return _history.has_mode_at(time);
}

SamplesHistory RobotStateHistorySnapshot::samples_history(Mode const& mode) const {
    return _history.samples_history(mode);
}

Robot const& RobotStateHistorySnapshot::get_robot() const{
    return _history.get_robot();
}

Mode const& RobotStateHistorySnapshot::latest_mode() const{
    return _history.latest_mode();
}

std::ostream& operator<<(std::ostream& os, RobotPredictTiming const& p) {
    if (p.impossible_prediction_flag){
        return os << "Predicted reaching mode '" << p._target << "' not reachable";
    }else{
        return os << "Predicted reaching mode '" << p._target << "' in [ " << p.nanoseconds_to_mode << " ] nanoseconds";
    }
}

std::ostream& operator<<(std::ostream& os, HumanRobotDistance const& p) {
    Interval<FloatType> min_max = p.get_min_max_distances();
    return os << "Interval of minimum distances, lower: '" << min_max.lower() << " - upper: " << min_max.upper();
}

// #~#^

}
