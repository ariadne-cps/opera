/***************************************************************************
 *            body_registry.hpp
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

#ifndef OPERA_BODY_REGISTRY_HPP
#define OPERA_BODY_REGISTRY_HPP

#include "state.hpp"
#include "message.hpp"

namespace Opera {

//! \brief Holds a human body along with its history
class HumanRegistryEntry {
  public:
    //! \brief Construct from human fields
    HumanRegistryEntry(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses);

    //! \brief Return the body
    Human const& body() const;
    //! \brief Return the history
    HumanStateHistory& history();

    //! \brief Whether there are instances with a given \a timestamp
    bool has_instances_within(TimestampType const& timestamp) const;
    //! \brief Return the instance within a given \a timestamp
    HumanStateInstance const& latest_instance_within(TimestampType const& timestamp) const;
    //! \brief Get the number of instances between two timestamps \a lower and \a upper
    //! \details Used to acknowledge if some instance has been skipped between two awaken-ups of a given job
    SizeType instance_distance(TimestampType const& lower, TimestampType const& upper) const;

    //! \brief Get the number of instance corresponding to \a timestamp
    //! \details Fails if no instance exists
    SizeType instance_number(TimestampType const& timestamp) const;

    //! \brief The number of states in the history
    SizeType size() const;

    //! \brief The most recent timestamp from the history
    TimestampType latest_timestamp() const;

    //! \brief Return the instance at \a idx
    HumanStateInstance const& at(SizeType const& idx) const;

    //! \brief Add a new instance from \a points and \a timestamp
    void add(Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp);
  private:
    Human const _body;
    HumanStateHistory _history;
};

//! \brief Holds a robot body along with its history
struct RobotRegistryEntry {
  public:
    //! \brief Construct from robot fields
    RobotRegistryEntry(BodyIdType const& id, SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses);

    //! \brief Return the body
    Robot const& body() const;
    //! \brief Return the history
    RobotStateHistory& history();
  private:
    Robot const _body;
    RobotStateHistory _history;
};

//! \brief A registry for bodies introduced by presentation
//! \details Used as a synchronised source for body data instead of passing bodies around
class BodyRegistry {
  public:
    BodyRegistry() = default;
    BodyRegistry(BodyRegistry const&) = delete;
    void operator=(BodyRegistry const&) = delete;

    //! \brief Whether the body given by \a id is already registered
    bool contains(BodyIdType const& id) const;
    //! \brief The number of robots
    SizeType num_robots() const;
    //! \brief The number of humans
    SizeType num_humans() const;

    //! \brief The number of segment pairs from the robots and humans
    SizeType num_segment_pairs() const;

    //! \brief Return the ids for all the robots
    List<BodyIdType> robot_ids() const;
    //! \brief Return the ids for all the humans
    List<BodyIdType> human_ids() const;

    //! \brief The robot having the given \a id
    Robot const& robot(BodyIdType const& id) const;
    //! \brief The human having the given \a id
    Human const& human(BodyIdType const& id) const;

    //! \brief Whether the registry has the human with given \a id
    bool has_human(BodyIdType const& id) const;
    //! \brief Whether the registry has the robot with given \a id
    bool has_robot(BodyIdType const& id) const;

    //! \brief The history of the robot having the given \a id
    RobotStateHistory& robot_history(BodyIdType const& id);
    RobotStateHistory const & robot_history(BodyIdType const& id) const;

    //! \brief The history of the human having the given \a id
    HumanStateHistory& human_history(BodyIdType const& id);
    HumanStateHistory const & human_history(BodyIdType const& id) const;

    //! \brief Whether the human with given \a id has instances within a given \a timestamp
    bool has_human_instances_within(BodyIdType const& id, TimestampType const& timestamp) const;

    //! \brief The most recent instance of the human having the given \a id within a \a timestamp
    //! \details There is no check for the existence of instances, since it would be performed before using this method
    HumanStateInstance const& latest_human_instance_within(BodyIdType const& id, TimestampType const& timestamp) const;

    //! \brief Return the size of the human history for the given \a id
    SizeType human_history_size(BodyIdType const& id) const;

    //! \brief Return the timestamp of the latest instance for the given human
    TimestampType latest_human_timestamp(BodyIdType const& id) const;

    //! \brief Get the number of human instances between two timestamps \a lower and \a upper
    SizeType instance_distance(BodyIdType const& id, TimestampType const& lower, TimestampType const& upper) const;

    //! \brief Get the number of human instance corresponding to \a timestamp for body \a id
    //! \details Fails if no instance exists
    SizeType instance_number(BodyIdType const& id, TimestampType const& timestamp) const;

    //! \brief Return the human instance at \a idx
    HumanStateInstance const& instance_at(BodyIdType const& id, SizeType const& idx) const;

    //! \brief Acquire state from a human state \a msg
    void acquire_state(HumanStateMessage const& msg);
    //! \brief Acquire state from a robot state \a msg
    void acquire_state(RobotStateMessage const& msg);

    //! \brief Insert a new body from a \a presentation message
    void insert(BodyPresentationMessage const& presentation);
    //! \brief Insert a new human from fields
    void insert_human(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses);
    //! \brief Insert a new robot from fields
    void insert_robot(BodyIdType const& id, SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& segment_pairs, List<FloatType> const& thicknesses);

    //! \brief Try to get the human \a human_id head/tail keypoint ids for a given \a segment_id
    //! \details The human may not be found if it has been removed, in that case the first field is false
    std::tuple<bool,KeypointIdType, KeypointIdType> get_human_keypoint_ids(BodyIdType const& human_id, IdType const& segment_id) const;

    //! \brief Remove the body given the \a id
    void remove(BodyIdType const& id);
    //! \brief Remove all bodies
    void clear();

  private:

    void _add_human_instance(BodyIdType const& id, Map<KeypointIdType,List<Point>> const& points, TimestampType const& timestamp);

  private:
    Map<BodyIdType,SharedPointer<RobotRegistryEntry>> _robots;
    Map<BodyIdType,SharedPointer<HumanRegistryEntry>> _humans;
    mutable std::mutex _content_mux;
};

}

#endif //OPERA_BODY_REGISTRY_HPP
