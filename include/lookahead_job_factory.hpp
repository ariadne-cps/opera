/***************************************************************************
 *            lookahead_job_factory.hpp
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

#ifndef OPERA_LOOKAHEAD_JOB_FACTORY_HPP
#define OPERA_LOOKAHEAD_JOB_FACTORY_HPP

#include "lookahead_job_registry.hpp"

namespace Opera {

using IdType = unsigned int;
using BodyIdType = std::string;

class BodyRegistry;

//! \brief Create LookAheadJob objects
class LookAheadJobFactoryInterface {
  public:
    //! \brief Create a new job given the fields
    virtual LookAheadJob create_new(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& mode_trace, LookAheadJobPath const& path) const = 0;
    //! \brief Create the new jobs that result from moving to the next mode(s) obtained according to the \a robot_history
    //! \details If the next mode is already within the trace (but not the first element of the trace) then no corresponding job is created
    virtual List<LookAheadJob> create_next(LookAheadJob const& job, RobotStateHistory const& robot_history) const = 0;
    //! \brief Create job(s) that result from waking up \a job according to a new \a time and \a human_sample, where the \a robot_history is required to produce the resulting job(s)
    //! \details Each job specifies the actual result of awakening, in order to understand where the job needs to be put (working or sleeping queues)
    virtual List<Pair<LookAheadJob,JobAwakeningResult>> awaken(LookAheadJob const& job, TimestampType const& time, BodySegmentSample const& human_sample, RobotStateHistory const& robot_history) const = 0;
    //! \brief Check if the \a path at \a id and \a timestamp has been registered
    virtual bool has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const = 0;

    //! \brief Default virtual destructor
    virtual ~LookAheadJobFactoryInterface() = default;
};

//! \brief Handle class for a look-ahead job factory
class LookAheadJobFactory : public Handle<LookAheadJobFactoryInterface> {
public:
    using Handle<LookAheadJobFactoryInterface>::Handle;

    LookAheadJob create_new_job(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& mode_trace, LookAheadJobPath const& path) const { return _ptr->create_new(id, initial_time, human_sample, mode_trace, path); }
    List<LookAheadJob> create_next_jobs(LookAheadJob const& job, RobotStateHistory const& robot_history) const { return _ptr->create_next(job, robot_history); }
    List<Pair<LookAheadJob,JobAwakeningResult>> awaken(LookAheadJob const& job, TimestampType const& time, BodySegmentSample const& human_sample, RobotStateHistory const& robot_history) const { return _ptr->awaken(job, time, human_sample, robot_history); }
    bool has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const { return _ptr->has_registered(timestamp,id,path); }
};

//! \brief Base functionality for a look ahead job factory
class LookAheadJobFactoryBase : public LookAheadJobFactoryInterface {
  public:
    List<LookAheadJob> create_next(LookAheadJob const& job, RobotStateHistory const& robot_history) const override;
  protected:
    virtual LookAheadJob create_from_existing(LookAheadJob const& job, ModeTrace const& new_mode_trace, LookAheadJobPath const& new_path) const = 0;
};

//! \brief Factory for the case where we restart from scratch after a new initial time/mode is received
class DiscardLookAheadJobFactory : public LookAheadJobFactoryBase {
  public:
    LookAheadJob create_new(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& mode_trace, LookAheadJobPath const& path) const override;
    List<Pair<LookAheadJob,JobAwakeningResult>> awaken(LookAheadJob const& job, TimestampType const& time, BodySegmentSample const& human_sample, RobotStateHistory const& robot_history) const override;
    bool has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const override;
  protected:
    LookAheadJob create_from_existing(LookAheadJob const& job, ModeTrace const& new_mode_trace, LookAheadJobPath const& new_path) const override;
};

//! \brief Factory for the case where we restart from an intermediate result, using capsule intersections for checking
class ReuseLookAheadJobFactory : public LookAheadJobFactoryBase {
  public:
    ReuseLookAheadJobFactory(MinimumDistanceBarrierSequenceUpdatePolicy const& update_policy, ReuseEquivalence const& equivalence);
    LookAheadJob create_new(LookAheadJobIdentifier const& id, TimestampType const& initial_time, BodySegmentSample const& human_sample, ModeTrace const& mode_trace, LookAheadJobPath const& path) const override;
    List<Pair<LookAheadJob,JobAwakeningResult>> awaken(LookAheadJob const& job, TimestampType const& time, BodySegmentSample const& human_sample, RobotStateHistory const& robot_history) const override;
    bool has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const override;
  protected:
    LookAheadJob create_from_existing(LookAheadJob const& job, ModeTrace const& new_mode_trace, LookAheadJobPath const& new_path) const override;
  private:
    SharedPointer<LookAheadJobRegistry> _registry;
    MinimumDistanceBarrierSequenceUpdatePolicy _update_policy;
    ReuseEquivalence _equivalence;
};

}

#endif //OPERA_LOOKAHEAD_JOB_FACTORY_HPP
