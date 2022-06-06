/***************************************************************************
 *            state.hpp
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

#ifndef OPERA_STATE_HPP
#define OPERA_STATE_HPP

#include <deque>
#include <mutex>
#include "body.hpp"
#include "utility.hpp"
#include "interval.hpp"
#include "mode.hpp"

namespace Opera {

using IdType = unsigned int;
using BodyIdType = std::string;

//! \brief Holds the state of a human
class HumanStateInstance {
  public:
    //! \brief Construct from a human, points and timestamp
    HumanStateInstance(Human const& human, List<List<Point>> const& points, TimestampType const& timestamp);

    //! \brief The timestamp of the instance
    TimestampType const& timestamp() const;
    //! \brief The samples for each segment
    List<BodySegmentSample> const& samples() const;
  private:
    TimestampType const _timestamp;
    List<BodySegmentSample> _samples;
};

//! \brief Holds the states reached by a human up to now
class HumanStateHistory {
  public:
    //! \brief Construct from a human
    HumanStateHistory(Human const& human);
    HumanStateHistory(HumanStateHistory const& other) = delete;

    //! \brief Add an instance
    void acquire(List<List<Point>> const& points, TimestampType const& timestamp);

    //! \brief Check if there are instances with timestamp within \a timestamp
    bool has_instances_within(TimestampType const& timestamp) const;

    //! \brief Get the latest instance with timestamp lesser or equal than \a timestamp
    //! \details This is necessary to choose an instance for which we have a defined
    //! robot mode to check against, instead of an unbounded one
    HumanStateInstance const& latest_within(TimestampType const& timestamp) const;

    //! \brief Returns a list containing the indexes of the instances present within the two timestamps (inclusive)
    List<SizeType> idxs_within(TimestampType const& lower_timestamp, TimestampType const& higher_timestamp) const; //#~#

    //! \brief Get the number of instances between two timestamps \a lower and \a upper
    SizeType instance_distance(TimestampType const& lower, TimestampType const& upper) const;

    //! \brief Get the number of instance corresponding to \a timestamp
    //! \details Fails if no instance exists
    SizeType instance_number(TimestampType const& timestamp) const;

    //! \brief Return the instance at \a idx
    HumanStateInstance const& at(SizeType const& idx) const;

    //! \brief The number of instances
    SizeType size() const;
  private:
    Human const _human;
    List<HumanStateInstance> _instances;
};

//! \brief The presence of a robot in a given mode
class RobotModePresence {
  public:
    //! \brief Construct from a \a source, \a exit_destination, \a from and \a to for entrance/exit in the source mode
    RobotModePresence(Mode const& mode, Mode const& exit_destination, TimestampType const& from, TimestampType const& to);
    //! \brief The mode of presence
    Mode const& mode() const;
    //! \brief The destination mode after exiting
    Mode const& exit_destination() const;
    //! \brief The timestamp of entrance
    TimestampType const& from() const;
    //! \brief The timestamp of exit (which is excluded from the presence itself)
    TimestampType const& to() const;

    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, RobotModePresence const& p);
  private:
    Mode const _mode;
    Mode const _exit_destination;
    TimestampType const _from;
    TimestampType const _to;
};


//! \brief Holds the continuous history for a given mode
class SamplesHistory {
    typedef List<BodySegmentSample> SegmentTemporalSamplesType;
    typedef List<SegmentTemporalSamplesType> BodySamplesType;
    typedef Pair<TimestampType,BodySamplesType> TimedBodySamplesType;
  public:
    //! \brief Get the samples for the given \a timestamp
    //! \details An entry with timestamp greater or equal than \a timestamp must exist
    BodySamplesType const& at(TimestampType const& timestamp) const;

    //! \brief Append a new entry for \a timestamp, with \a samples
    void append(TimestampType const& timestamp, BodySamplesType const& samples);

    //! \brief Whether there are samples valid at \a timestamp
    bool has_samples_at(TimestampType const& timestamp) const;

    //! \brief The number of samples at the given \a timestamp
    SizeType size_at(TimestampType const& timestamp) const;
  private:
    List<TimedBodySamplesType> _entries;
};

class RobotStateHistorySnapshot;

//! \brief Holds the states reached by a robot up to now
class RobotStateHistory {
    friend class RobotStateHistorySnapshot;
    typedef List<BodySegmentSample> SegmentTemporalSamplesType;
    typedef List<SegmentTemporalSamplesType> BodySamplesType;
    typedef Map<Mode,SamplesHistory> ModeSamplesHistoryType;
  public:
    RobotStateHistory(Robot const& robot);
    RobotStateHistory(RobotStateHistory const& other) = delete;
  public:

    //! \brief The most recent time of a state acquired
    TimestampType const& latest_time() const;

    //! \brief The most recent mode according to the latest time
    Mode const& latest_mode() const;

    //! \brief Acquire the \a state to be ultimately held into the hystory
    //! \details Hystory will not be effectively updated till the mode changes
    void acquire(Mode const& mode, List<List<Point>> const& points, TimestampType const& timestamp);

    //! \brief The mode of the robot at the given \a timestamp
    //! \details If the time is greater than the received last sample, then the current mode is returned
    Mode const& mode_at(TimestampType const& timestamp) const;

    //! \brief If there is a mode at the given \a timestamp
    bool has_mode_at(TimestampType const& timestamp) const; //#~#
    SamplesHistory samples_history(Mode const& mode) const; //#~#
    //! \brief Return a snapshot at the given \a timestamp
    RobotStateHistorySnapshot snapshot_at(TimestampType const& timestamp) const;

  public:
    std::deque<RobotModePresence> _mode_presences;
  private:
    ModeSamplesHistoryType _mode_states;
    Mode _latest_mode;
    TimestampType _latest_time;
    BodySamplesType _current_mode_states_buffer;

    std::mutex mutable _states_mux;
    std::mutex mutable _presences_mux;

    List<Pair<TimestampType,ModeTrace>> _mode_traces;

  protected:
    Robot const _robot;

    // #~#v
    Robot const& get_robot() const;
    // #~#^
};

//! \brief A wrapper class for a snapshot of the history at a given time
class RobotStateHistorySnapshot {
    friend class RobotStateHistory;
    typedef List<BodySegmentSample> SegmentTemporalSamplesType;
    typedef List<SegmentTemporalSamplesType> BodySamplesType;
  protected:
    //! \brief Construct from a \a history and a \a snapshot_time
    RobotStateHistorySnapshot(RobotStateHistory const& history, TimestampType const& snapshot_time);
  public:

    //! \brief The mode trace
    //! \details A mode ends up here only after a sample from the next mode has been acquired, so that
    //! at least one next mode always exists from the trace
    ModeTrace const& mode_trace() const;

    //! \brief The modes having samples
    Set<Mode> modes_with_samples() const;

    //! \brief The samples in a given \a mode
    BodySamplesType const& samples(Mode const& mode) const;

    //! \brief The maximum number of samples in a given \a mode
    //! \details The result is independent of the segment chosen
    SizeType maximum_number_of_samples(Mode const& mode) const;

    //! \brief The presences in a given \a mode
    List<RobotModePresence> presences_in(Mode const& mode) const;
    //! \brief The presences exiting into a given \a mode
    List<RobotModePresence> presences_exiting_into(Mode const& mode) const;
    //! \brief The presences between a \a source and \a destination modes
    List<RobotModePresence> presences_between(Mode const& source, Mode const& destination) const;
    //! \brief The range of number of samples acquired in a given mode in general
    Interval<SizeType> range_of_num_samples_in(Mode const& mode) const;
    //! \brief The range of number of samples that are acquired when in \a mode going specifically into \a target
    Interval<SizeType> range_of_num_samples_in(Mode const& mode, Mode const& target) const;

    //! \brief Whether it is possible to look ahead starting at the given \a timestamp
    //! \details It is possible to look ahead only if, in the current mode, there are previous presences: this avoids
    //! the issue of the robot being slightly ahead of the human and erroneously thinking that a mode just completed
    //! is part of the history.
    bool can_look_ahead(TimestampType const& timestamp) const;

    //! \brief Find the index of the sample in the given \a mode according to a \a timestamp, with no rounding
    //! \details The index can be greater or equal than the current number of samples, meaning that we are introducing new samples
    FloatType unrounded_sample_index(Mode const& mode, TimestampType const& timestamp) const;
    //! \brief Find the index of the sample in the given \a mode according to a \a timestamp
    SizeType sample_index(Mode const& mode, TimestampType const& timestamp) const;
    //! \brief Find the index of the sample in the given \a mode according to a \a timestamp
    //! \details The index can not be greater or equal than the current number of samples
    SizeType checked_sample_index(Mode const& mode, TimestampType const& timestamp) const;

    // #~#v
    //! \brief The mode of the robot at the given \a timestamp
    //! \details If the time is greater than the received last sample, then the current mode is returned
    Mode const& mode_at(TimestampType const& timestamp) const;

    //! \brief If there is a mode at the given \a timestamp
    bool has_mode_at(TimestampType const& timestamp) const;

    SamplesHistory samples_history(Mode const& mode) const; //#~#

    Robot const& get_robot() const;
    Mode const& latest_mode() const;
    // #~#^

  private:
    //! \brief The range of number of samples within a list of \a presences
    Interval<SizeType> _range_of_num_samples_within(List<RobotModePresence> const& presences) const;

  private:
    RobotStateHistory const& _history;
    TimestampType _snapshot_time;


};

// #~#v

class RobotPredictTiming {
    friend class RobotStateHistorySnapshot;
    public:
    //! \brief construct from \a robotstatehistorysnapshot
        RobotPredictTiming(RobotStateHistorySnapshot const& snapshot, Mode const& target);
    //! \brief constuct from \a robotstatehisotry
        RobotPredictTiming(RobotStateHistory const& history, Mode const& target);
    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, RobotPredictTiming const& p);

    bool impossible_prediction_flag = false;
    SizeType nanoseconds_to_mode = 0;


    private:
            // must be called in every constructor

        void _common_constructor();
        void _extract_mode_trace();
        ModeTrace _find_paths(ModeTrace trace);
        int _set_best_path();
        // legacy function to get a trace predicting the target mode, uses less memory than the _compute_branch_path method
        //void _augment_trace();

        void _predict_timing();

        SizeType _index_present_mode;
        RobotStateHistorySnapshot _snapshot;

        int _path_max_depth = 10;
        ModeTrace _mode_trace;
        Robot const _robot;
        Mode const& _target;
        Mode _present_mode;
        ModeTrace _best_path;

        List<ModeTrace> _branch_paths;
};

class HumanRobotDistance{
    typedef List<BodySegmentSample> SegmentTemporalSamplesType;
    typedef List<SegmentTemporalSamplesType> BodySamplesType;
    public:
        HumanRobotDistance(HumanStateHistory const& human_history, RobotStateHistorySnapshot const& robot_snapshot, IdType const& human_segment_id, IdType const& robot_segment_id, TimestampType const& lower_timestamp, TimestampType const& higher_timestamp);
        Interval<FloatType> get_min_max_distances() const;
        //! \brief Print to the standard output
        friend std::ostream& operator<<(std::ostream& os, HumanRobotDistance const& p);


    private:

        void _compute_distances();
        void _set_human_instances();
        void _compute_min_max();

        void _print_robot_instances();

        HumanStateHistory const& _human_history;
        RobotStateHistorySnapshot const& _robot_snapshot;
        IdType const& _human_segment_id;
        IdType const& _robot_segment_id;
        TimestampType const& _lower_timestamp;
        TimestampType const& _higher_timestamp;

        Interval<FloatType> *_min_max_distances;

        List <HumanStateInstance> _human_instances;
        List<FloatType> _minimum_distances;
};

// #~#^

}

#endif //OPERA_STATE_HPP
