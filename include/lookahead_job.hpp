/***************************************************************************
 *            lookahead_job.hpp
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

#ifndef OPERA_LOOKAHEAD_JOB_HPP
#define OPERA_LOOKAHEAD_JOB_HPP

#include "handle.hpp"
#include "barrier.hpp"
#include "mode.hpp"

namespace Opera {

using IdType = unsigned int;
using BodyIdType = std::string;

class BodyRegistry;

//! \brief A path defining a leaf in the tree of jobs sharing an identifier
//! \details A job is normally a single node tree, but multiple leaves
//! are created when multiple next modes are present, iteratively.
//! Each node in the tree defines a priority, increasing from 0: only the job
//! with a path node featuring priority 0 is maintained when pruning jobs with
//! the same identifier. Each node also defines a trace position, which is a
//! positive value that defines the position in the trace in which the mode
//! deviated (i.e., the size of the mode trace before adding the next mode).
class LookAheadJobPath {
  public:
    using PriorityType = SizeType;
    using PredictionTracePosition = SizeType;
  public:
    //! \brief Create empty
    LookAheadJobPath() = default;

    //! \brief Add an element to the tail
    LookAheadJobPath& add(PriorityType const& priority, PredictionTracePosition const& trace_position);
    //! \brief Remove all elements with position lesser or equal than the given \a trace_position, scaling down positions accordingly
    LookAheadJobPath& remove_le_than(PredictionTracePosition const& trace_position);
    //! \brief Remove all elements with position greater than the given \a trace_position
    LookAheadJobPath& remove_g_than(PredictionTracePosition const& trace_position);
    //! \brief Remove all elements > \a upper_trace_position and <= \a lower_trace_position
    LookAheadJobPath& reduce_between(PredictionTracePosition const& lower_trace_position, PredictionTracePosition const& upper_trace_position);

    //! \brief Get the priority of the element at \a index
    PriorityType const& priority(SizeType const& index) const;

    //! \brief Whether this represents a primary job (whose priorities are all zeros)
    bool is_primary() const;

    //! \brief The size of the path
    SizeType size() const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, LookAheadJobPath const& path);
  private:
    List<std::pair<PriorityType,PredictionTracePosition>> _path;
};

//! \brief The composite identifier related to the segments for intersection look-head
class LookAheadJobIdentifier {
  public:
    //! \brief Construct from fields
    LookAheadJobIdentifier(BodyIdType const& human, IdType const& human_segment, BodyIdType const& robot, IdType const& robot_segment);
    //! \brief The human identifier
    BodyIdType const& human() const;
    //! \brief The human segment identifier
    IdType const& human_segment() const;
    //! \brief The robot identifier
    BodyIdType const& robot() const;
    //! \brief The robot segment identifier
    IdType const& robot_segment() const;

    //! \brief Compare two ids for equality
    friend bool operator==(const LookAheadJobIdentifier& id1, const LookAheadJobIdentifier& id2);

    //! \brief Compare two ids for ordering
    friend bool operator<(const LookAheadJobIdentifier& id1, const LookAheadJobIdentifier& id2);

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, LookAheadJobIdentifier const& id);

  private:
    BodyIdType const _human;
    IdType const _human_segment;
    BodyIdType const _robot;
    IdType const _robot_segment;
};

//! \brief Enumeration for the result of awakening a job:
//! \details DIFFERENT: the job is different from the original and must be processed again, hence it is put into working
//!          UNAFFECTED: the job is not different, hence it must be put back into sleeping
//!          COMPLETED: the job is already completed due to a loop
//!          UNCOMPUTABLE: the human sample is empty, making it uncomputable, hence it must be put back into sleeping
enum class JobAwakeningResult { DIFFERENT, UNAFFECTED, COMPLETED, UNCOMPUTABLE };

//! \brief Interface for look-ahead job
//! \details A job is intended to be immutable and processed, from which one or more jobs may be created using a factory.
//! A job can be in one of three different states: waiting, running and sleeping
//! - waiting: the job is waiting for a thread to pick it up, which moves it to the running state
//! - running: (if the initial time/mode changed, create a new job with the updated time/mode) process all continuous states
//!   of the robot on the tail of the prediction trace, with the following two possible outcomes:
//!   1) a collision is found, which produces a notification and puts the job in the sleeping state
//!   2) the job is completed successfully, then for each predicted next mode we can have these sub-outcomes:
//!      a) the mode is not already present in the prediction trace: put the job in the waiting state with updated trace, to be checked fully;
//!      b) the mode is already present and the first in the trace: put the job in the waiting state with updated trace, to be checked till the initial time;
//!      c) the mode is already present but not the first in the trace: put the job in the sleeping state with updated trace;
//! - sleeping: does nothing until a new human sample is received or the robot mode changes; when this happens, the thread delegated to samples acquirement
//!   creates updated jobs in the waiting state
class LookAheadJobInterface {
  public:
    //! \brief The identifier for the job
    virtual LookAheadJobIdentifier const& id() const = 0;
    //! \brief The time from which look-ahead started, used to identify loops
    virtual TimestampType const& initial_time() const = 0;
    //! \brief The time used as reference in the history
    //! \details No history after this instant is considered
    virtual TimestampType const& snapshot_time() const = 0;
    //! \brief The human segment sample to be used for the job
    virtual BodySegmentSample const& human_sample() const = 0;
    //! \brief The current predicted discrete trace of the robot from the beginning of look-ahead
    //! \details A mode is put here as soon as it must be analysed, hence the trace starts with the initial mode
    virtual ModeTrace const& prediction_trace() const = 0;
    //! \brief The path in the tree for this job identifier, as obtained when splitting due to multiple target modes
    //! \details This is usually pruned when updating the initial time and the prediction trace, possibly resulting in the job needing discarding
    virtual LookAheadJobPath const& path() const = 0;

    //! \brief Find the earliest index for which a collision is present within the tail mode in the job collision trace, according to the \a robot_history
    //! \details Returns -1 if no collision is found
    virtual int earliest_collision_index(RobotStateHistory const& robot_history) const = 0;

    //! \brief Default virtual destructor
    virtual ~LookAheadJobInterface() = default;
};

//! \brief Base class for implementations
class LookAheadJobBase : public LookAheadJobInterface {
  protected:
    //! \brief Construct a job from fields
    LookAheadJobBase(LookAheadJobIdentifier const& id, TimestampType const& initial_time, TimestampType const& snapshot_time, BodySegmentSample const& human_sample, ModeTrace const& prediction_trace, LookAheadJobPath const& path);
  public:
    //! \brief Interface methods
    LookAheadJobIdentifier const& id() const;
    TimestampType const& initial_time() const;
    TimestampType const& snapshot_time() const;
    BodySegmentSample const& human_sample() const;
    ModeTrace const& prediction_trace() const;
    LookAheadJobPath const& path() const;

  protected:
    LookAheadJobIdentifier const _id;
    TimestampType const _initial_time;
    TimestampType const _snapshot_time;
    BodySegmentSample const _human_sample;
    ModeTrace const _prediction_trace;
    LookAheadJobPath const _path;
};

//! \brief A simple implementation where we restart from scratch each time, discarding prediction data from the previous iteration
//! \details In this case, the prediction trace always starts empty
class DiscardLookAheadJob : public LookAheadJobBase {
  public:
    DiscardLookAheadJob(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& prediction_trace, LookAheadJobPath const& path);
    int earliest_collision_index(RobotStateHistory const& robot_history) const override;
};

//! \brief Reuses prediction data by storing the segment sample representation in a barrier trace
class ReuseLookAheadJob : public LookAheadJobBase {
  public:
    ReuseLookAheadJob(LookAheadJobIdentifier const& id, TimestampType const& initial_time, TimestampType const& snapshot_time, BodySegmentSample const& human_sample, ModeTrace const& prediction_trace,
                      LookAheadJobPath const& path, MinimumDistanceBarrierSequence const& barrier_sequence);
    MinimumDistanceBarrierSequence const& barrier_sequence() const;
    int earliest_collision_index(RobotStateHistory const& robot_history) const override;
  private:
    MinimumDistanceBarrierSequence mutable _barrier_sequence;
};

//! \brief Enumeration for the available equivalence guarantee when reusing prediction data
//! \details STRONG: prediction results are strictly equivalent to those obtained when not using reusal
//! (under the condition of sufficient computational capabilities not to miss any prediction in either case);
//! Here any mode whose samples are updated is excluded from reusal.
//! \details WEAK: reusal is preserved as long as the mode trace remains consistent with the prediction trace.
//! In the case of an infinite loop in the mode trace, weak reusal will never update the samples used.
enum class ReuseEquivalence { STRONG, WEAK };

//! \brief Handle class for a look-ahead job
class LookAheadJob : public Handle<LookAheadJobInterface> {
  public:
    using Handle<LookAheadJobInterface>::Handle;
    using Handle<LookAheadJobInterface>::ptr;

    LookAheadJobIdentifier const& id() const { return _ptr->id(); }
    TimestampType const& initial_time() const { return _ptr->initial_time(); }
    TimestampType const& snapshot_time() const { return _ptr->snapshot_time(); }
    BodySegmentSample const& human_sample() const { return _ptr->human_sample(); }
    ModeTrace const& prediction_trace() const { return _ptr->prediction_trace(); }
    LookAheadJobPath const& path() const { return _ptr->path(); }

    int earliest_collision_index(RobotStateHistory const& robot_history) const { return _ptr->earliest_collision_index(robot_history); };

    friend std::ostream& operator<<(std::ostream& os, LookAheadJob const& j) {
        return os << "{id=" << j.id() << ", time=" << j.initial_time() << ", human_sample: " << j.human_sample() << ", trace: " << j.prediction_trace() << ", path: " << j.path() << "}"; }
};

}

#endif //OPERA_LOOKAHEAD_JOB_HPP
