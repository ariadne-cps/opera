/***************************************************************************
 *            message.hpp
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

#ifndef OPERA_MESSAGE_HPP
#define OPERA_MESSAGE_HPP

#include "handle.hpp"
#include "geometry.hpp"
#include "interval.hpp"
#include "mode.hpp"

namespace Opera {

using IdType = unsigned int;
using BodyIdType = std::string;

//! \brief A representation of an inbound message for the presentation of a body
class BodyPresentationMessage {
  public:
    //! \brief Construct for a human
    BodyPresentationMessage(BodyIdType const& id, List<Pair<IdType,IdType>> const& point_ids, List<FloatType> const& thicknesses);
    //! \brief Construct for a robot
    BodyPresentationMessage(BodyIdType const& id, SizeType const& message_frequency, List<Pair<IdType,IdType>> const& point_ids, List<FloatType> const& thicknesses);
    //! \brief The id of the related body
    BodyIdType const& id() const;
    //! \brief Whether this is a human
    bool const& is_human() const;
    //! \brief The message sending frequency in Hz
    //! \details We use 0 when not known (i.e., for a human)
    SizeType const& message_frequency() const;
    //! \brief The points for each segment
    List<Pair<IdType,IdType>> const& point_ids() const;
    //! \brief The thicknesses for each segment
    List<FloatType> const& thicknesses() const;
  private:
    BodyIdType const _id;
    bool const _is_human;
    SizeType const _message_frequency;
    List<Pair<IdType,IdType>> const _point_ids;
    List<FloatType> const _thicknesses;
};

//! \brief A representation of an inbound message for the state of a human
class HumanStateMessage {
  public:
    //! \brief Construct without a mode
    HumanStateMessage(List<Pair<BodyIdType,List<List<Point>>>> const& bodies, TimestampType const& timestamp);
    //! \brief The bodies
    List<Pair<BodyIdType,List<List<Point>>>> const& bodies() const;
    //! \brief The timestamp associated with the message
    TimestampType const& timestamp() const;
  private:
    List<Pair<BodyIdType,List<List<Point>>>> const _bodies;
    TimestampType const _timestamp;
};

//! \brief A representation of an inbound message for the state of a robot
class RobotStateMessage {
  public:
    //! \brief Construct from an id, a mode, a list of samples for each point, and a \a timestamp
    RobotStateMessage(BodyIdType const& id, Mode const& mode, List<List<Point>> const& points, TimestampType const& timestamp);
    //! \brief The id of the related body
    BodyIdType const& id() const;
    //! \brief The mode
    Mode const& mode() const;
    //! \brief The samples for each point
    List<List<Point>> const& points() const;
    //! \brief The timestamp associated with the message
    TimestampType const& timestamp() const;
  private:
    BodyIdType const _id;
    Mode const _mode;
    List<List<Point>> const _points;
    TimestampType const _timestamp;
};

//! \brief A representation of an outbound message for a detected collision
class CollisionNotificationMessage {
  public:
    //! \brief Construct from fields
    CollisionNotificationMessage(BodyIdType const& human_id, IdType const& human_segment_id, BodyIdType const& robot_id, IdType const& robot_segment_id,
                                 TimestampType const& current_time, Interval<TimestampType> const& collision_distance, Mode const& collision_mode, PositiveFloatType const& likelihood);

    //! \brief The identifier of the human
    BodyIdType const& human_id() const;
    //! \brief The identifier of the segment for the human
    IdType const& human_segment_id() const;
    //! \brief The identifier of the robot
    BodyIdType const& robot_id() const;
    //! \brief The identifier of the segment for the robot
    IdType const& robot_segment_id() const;
    //! \brief The current time according to the message
    TimestampType const& current_time() const;
    //! \brief The collision segment_distance (in ns)
    Interval<TimestampType> const& collision_distance() const;
    //! \brief The mode of the robot in the collision
    Mode const& collision_mode() const;
    //! \brief The likelihood that the path leading to this collision will be taken
    PositiveFloatType const& likelihood() const;

  private:
    BodyIdType const _human_id;
    IdType const _human_segment_id;
    BodyIdType const _robot_id;
    IdType const _robot_segment_id;
    TimestampType const _current_time;
    Interval<TimestampType> const _collision_distance;
    Mode const _collision_mode;
    PositiveFloatType const _likelihood;
};

}

#endif //OPERA_MESSAGE_HPP
