/***************************************************************************
 *            message.cpp
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

#include "handle.hpp"
#include "message.hpp"

namespace Opera {

BodyPresentationMessage::BodyPresentationMessage(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses) :
    _id(id), _is_human(true), _message_frequency(0), _segment_pairs(segment_pairs), _thicknesses(thicknesses) { }

BodyPresentationMessage::BodyPresentationMessage(BodyIdType const& id,  SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses) :
    _id(id), _is_human(false), _message_frequency(message_frequency), _segment_pairs(segment_pairs), _thicknesses(thicknesses) { }

BodyIdType const& BodyPresentationMessage::id() const {
    return _id;
}

bool const& BodyPresentationMessage::is_human() const {
    return _is_human;
}

SizeType const& BodyPresentationMessage::message_frequency() const {
    return _message_frequency;
}

List<Pair<KeypointIdType,KeypointIdType>> const& BodyPresentationMessage::segment_pairs() const {
    return _segment_pairs;
}

List<FloatType> const& BodyPresentationMessage::thicknesses() const {
    return _thicknesses;
}

HumanStateMessage::HumanStateMessage(List<Pair<BodyIdType,Map<KeypointIdType,List<Point>>>> const& bodies, TimestampType const& timestamp) :
        _bodies(bodies), _timestamp(timestamp) { }

List<Pair<BodyIdType,Map<KeypointIdType,List<Point>>>> const& HumanStateMessage::bodies() const {
    return _bodies;
}

TimestampType const& HumanStateMessage::timestamp() const {
    return _timestamp;
}

RobotStateMessage::RobotStateMessage(BodyIdType const& id, Mode const& mode, List<List<Point>> const& points, TimestampType const& timestamp) :
    _id(id), _mode(mode), _points(points), _timestamp(timestamp) { }

BodyIdType const& RobotStateMessage::id() const {
    return _id;
}

Mode const& RobotStateMessage::mode() const {
    return _mode;
}

List<List<Point>> const& RobotStateMessage::points() const {
    return _points;
}

TimestampType const& RobotStateMessage::timestamp() const {
    return _timestamp;
}

CollisionNotificationMessage::CollisionNotificationMessage(BodyIdType const& human_id, Pair<KeypointIdType,KeypointIdType> const& human_segment_id, BodyIdType const& robot_id, Pair<KeypointIdType,KeypointIdType> const& robot_segment_id,
                                                           TimestampType const& current_time, Interval<TimestampType> const& collision_distance, Mode const& collision_mode, PositiveFloatType const& likelihood) :
        _human_id(human_id), _human_segment_id(human_segment_id), _robot_id(robot_id), _robot_segment_id(robot_segment_id),
        _current_time(current_time), _collision_distance(collision_distance), _collision_mode(collision_mode), _likelihood(likelihood) { }

BodyIdType const& CollisionNotificationMessage::human_id() const {
    return _human_id;
}

Pair<KeypointIdType,KeypointIdType> const& CollisionNotificationMessage::human_segment_id() const {
    return _human_segment_id;
}

BodyIdType const& CollisionNotificationMessage::robot_id() const {
    return _robot_id;
}

Pair<KeypointIdType,KeypointIdType> const& CollisionNotificationMessage::robot_segment_id() const {
    return _robot_segment_id;
}

Mode const& CollisionNotificationMessage::collision_mode() const {
    return _collision_mode;
}

TimestampType const& CollisionNotificationMessage::current_time() const {
    return _current_time;
}

Interval<TimestampType> const& CollisionNotificationMessage::collision_distance() const {
    return _collision_distance;
}

PositiveFloatType const& CollisionNotificationMessage::likelihood() const {
    return _likelihood;
}

}