/***************************************************************************
 *            body_registry.cpp
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

#include "body_registry.hpp"
#include "macros.hpp"

namespace Opera {

HumanRegistryEntry::HumanRegistryEntry(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses)
    : _body({id,segment_pairs,thicknesses}), _history(_body) { }

Human const& HumanRegistryEntry::body() const {
    return _body;
}

bool HumanRegistryEntry::has_instances_within(TimestampType const& timestamp) const {
    return _history.has_instances_within(timestamp);
}

HumanStateInstance const& HumanRegistryEntry::latest_instance_within(TimestampType const& timestamp) const {
    return _history.latest_within(timestamp);
}

SizeType HumanRegistryEntry::instance_distance(TimestampType const& lower, TimestampType const& upper) const {
    return _history.instance_distance(lower,upper);
}

SizeType HumanRegistryEntry::instance_number(TimestampType const& timestamp) const {
    return _history.instance_number(timestamp);
}

HumanStateInstance const& HumanRegistryEntry::at(SizeType const& idx) const {
        return _history.at(idx);
}

void HumanRegistryEntry::add(Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp) {
    _history.acquire(points,timestamp);
}

SizeType HumanRegistryEntry::size() const {
    return _history.size();
}

TimestampType HumanRegistryEntry::latest_timestamp() const {
    OPERA_PRECONDITION(_history.size() > 0)
    return _history.at(_history.size()-1).timestamp();
}

RobotRegistryEntry::RobotRegistryEntry(BodyIdType const& id, SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses)
    : _body({id,message_frequency,segment_pairs,thicknesses}), _history(_body) { }

Robot const& RobotRegistryEntry::body() const {
    return _body;
}

RobotStateHistory& RobotRegistryEntry::history() {
    return _history;
}

bool BodyRegistry::contains(BodyIdType const& id) const {
    return (_humans.has_key(id) or _robots.has_key(id));
}

SizeType BodyRegistry::num_robots() const {
    return _robots.size();
}

SizeType BodyRegistry::num_humans() const {
    return _humans.size();
}

SizeType BodyRegistry::num_segment_pairs() const {
    SizeType num_human_segments = 0;
    SizeType num_robot_segments = 0;

    for (auto const& r : _robots)
        num_robot_segments += r.second->body().num_segments();
    for (auto const& h : _humans)
        num_human_segments += h.second->body().num_segments();

    return num_human_segments * num_robot_segments;
}


List<BodyIdType> BodyRegistry::robot_ids() const {
    List<BodyIdType> result;
    for (auto const& e : _robots)
        result.emplace_back(e.first);
    return result;
}

List<BodyIdType> BodyRegistry::human_ids() const {
    List<BodyIdType> result;
    for (auto const& e : _humans)
        result.emplace_back(e.first);
    return result;
}

Robot const& BodyRegistry::robot(BodyIdType const& id) const {
    OPERA_PRECONDITION(contains(id))
    return _robots.at(id)->body();
}

Human const& BodyRegistry::human(BodyIdType const& id) const {
    OPERA_PRECONDITION(contains(id))
    return _humans.at(id)->body();
}

RobotStateHistory& BodyRegistry::robot_history(BodyIdType const& id) {
    OPERA_PRECONDITION(contains(id))
    return _robots.at(id)->history();
}

RobotStateHistory const& BodyRegistry::robot_history(BodyIdType const& id) const {
    OPERA_PRECONDITION(contains(id))
    return _robots.at(id)->history();
}

SizeType BodyRegistry::human_history_size(BodyIdType const& id) const {
    OPERA_PRECONDITION(contains(id))
    return _humans.at(id)->size();
}

bool BodyRegistry::has_human_instances_within(BodyIdType const& id, TimestampType const& timestamp) const {
    OPERA_PRECONDITION(contains(id))
    return _humans.at(id)->has_instances_within(timestamp);
}

HumanStateInstance const& BodyRegistry::latest_human_instance_within(BodyIdType const& id, TimestampType const& timestamp) const {
    return _humans.at(id)->latest_instance_within(timestamp);
}

TimestampType BodyRegistry::latest_human_timestamp(BodyIdType const& id) const {
    OPERA_PRECONDITION(contains(id))
    return _humans.at(id)->latest_timestamp();
}

SizeType BodyRegistry::instance_distance(BodyIdType const& id, TimestampType const& lower, TimestampType const& upper) const {
    return _humans.at(id)->instance_distance(lower,upper);
}

SizeType BodyRegistry::instance_number(BodyIdType const& id, TimestampType const& timestamp) const {
    return _humans.at(id)->instance_number(timestamp);
}

HumanStateInstance const& BodyRegistry::instance_at(BodyIdType const& id, SizeType const& idx) const {
    return _humans.at(id)-> at(idx);
}

void BodyRegistry::_add_human_instance(BodyIdType const& id, Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp) {
    OPERA_PRECONDITION(contains(id))
    if (_humans.at(id)->size() == 0 or timestamp > latest_human_timestamp(id))
        _humans.at(id)->add(points,timestamp);
}

void BodyRegistry::acquire_state(HumanStateMessage const& msg) {
    for (auto const& bd : msg.bodies())
        _add_human_instance(bd.first, bd.second, msg.timestamp());
}

void BodyRegistry::acquire_state(RobotStateMessage const& msg) {
    Map<KeypointIdType,List<Point>> points;
    for (SizeType i=0; i<msg.points().size(); ++i)
        points.insert(std::make_pair(to_string(i),msg.points().at(i)));
    robot_history(msg.id()).acquire(msg.mode(), points, msg.timestamp());
}

void BodyRegistry::insert(BodyPresentationMessage const& presentation) {
    if (not contains(presentation.id())) {
        if (presentation.is_human()) insert_human(presentation.id(),presentation.segment_pairs(),presentation.thicknesses());
        else insert_robot(presentation.id(),presentation.message_frequency(),presentation.segment_pairs(),presentation.thicknesses());
    }
}

void BodyRegistry::insert_human(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses) {
    _humans.insert(std::make_pair(id,new HumanRegistryEntry(id,segment_pairs,thicknesses)));
}

void BodyRegistry::insert_robot(BodyIdType const& id, SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses) {
    _robots.insert(std::make_pair(id,new RobotRegistryEntry(id,message_frequency,segment_pairs,thicknesses)));
}

std::tuple<bool,KeypointIdType, KeypointIdType> BodyRegistry::get_human_keypoint_ids(BodyIdType const& human_id, IdType const& segment_id) const {
    std::lock_guard<std::mutex> lock(_content_mux);
    if (_humans.has_key(human_id)) {
        auto const& segment = _humans.at(human_id)->body().segment(segment_id);
        return std::make_tuple(true,segment.head_id(),segment.tail_id());
    } else return std::make_tuple(false,std::string(),std::string());
}

bool BodyRegistry::has_human(BodyIdType const& id) const {
    std::lock_guard<std::mutex> lock(_content_mux);
    return _humans.has_key(id);
}

bool BodyRegistry::has_robot(BodyIdType const& id) const {
    std::lock_guard<std::mutex> lock(_content_mux);
    return _robots.has_key(id);
}

void BodyRegistry::remove(BodyIdType const& id) {
    std::lock_guard<std::mutex> lock(_content_mux);
    if (_humans.has_key(id)) _humans.erase(id);
    else if (_robots.has_key(id)) _robots.erase(id);
    else {
        OPERA_THROW_RTE("Body with id '" << id << "' is not present in the registry.")
    }
}

void BodyRegistry::clear() {
    _humans.clear();
    _robots.clear();
}

}
