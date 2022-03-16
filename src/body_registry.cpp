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

HumanRegistryEntry::HumanRegistryEntry(BodyIdType const& id, List<Pair<IdType,IdType>> const& points_ids, List<FloatType> const& thicknesses)
    : _body({id,points_ids,thicknesses}), _history(_body) { }

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

void HumanRegistryEntry::add(List<List<Point>> const& points, TimestampType const& timestamp) {
    _history.acquire(points,timestamp);
}

RobotRegistryEntry::RobotRegistryEntry(BodyIdType const& id, SizeType const& message_frequency, List<Pair<IdType,IdType>> const& points_ids, List<FloatType> const& thicknesses)
    : _body({id,message_frequency,points_ids,thicknesses}), _history(_body) { }

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

bool BodyRegistry::has_human_instances_within(BodyIdType const& id, TimestampType const& timestamp) const {
    OPERA_PRECONDITION(contains(id))
    return _humans.at(id)->has_instances_within(timestamp);
}

HumanStateInstance const& BodyRegistry::latest_human_instance_within(BodyIdType const& id, TimestampType const& timestamp) const {
    return _humans.at(id)->latest_instance_within(timestamp);
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

void BodyRegistry::_add_human_instance(BodyIdType const& id, List<List<Point>> const& points, TimestampType const& timestamp) {
    OPERA_PRECONDITION(contains(id))
    _humans.at(id)->add(points,timestamp);
}

void BodyRegistry::acquire_state(BodyStateMessage const& msg) {
    if (msg.mode().is_empty()) _add_human_instance(msg.id(), msg.points(), msg.timestamp());
    else robot_history(msg.id()).acquire(msg.mode(), msg.points(), msg.timestamp());
}

void BodyRegistry::insert(BodyPresentationMessage const& presentation) {
    if (not contains(presentation.id())) {
        if (presentation.is_human())
            _humans.insert(std::make_pair(presentation.id(),new HumanRegistryEntry(presentation.id(),presentation.point_ids(),presentation.thicknesses())));
        else
            _robots.insert(std::make_pair(presentation.id(),new RobotRegistryEntry(presentation.id(),presentation.message_frequency(),presentation.point_ids(),presentation.thicknesses())));
    }
}

void BodyRegistry::remove(BodyIdType const& id) {
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
