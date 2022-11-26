/***************************************************************************
 *            barrier.hpp
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

#ifndef OPERA_BARRIER_HPP
#define OPERA_BARRIER_HPP

#include "state.hpp"
#include "handle.hpp"
#include "trace_sample_range.hpp"

namespace Opera {

class MinimumDistanceBarrierSequenceSectionBase;

//! \brief A barrier on the minimum segment_distance
class MinimumDistanceBarrier {
    friend MinimumDistanceBarrierSequenceSectionBase;
  protected:
    //! \brief Construct from fields
    MinimumDistanceBarrier(PositiveFloatType const& minimum_distance, TraceSampleRange const& range);
    //! \brief Update given a new \a index
    //! \details The new index must be adjacent to the maximum one
    void update_with(TraceSampleIndex const& index);
    //! \brief Scale down the trace index range of the given amount
    void scale_down_range_of(SizeType const& amount);
    //! \brief Cut the last elements down to a trace index higher than \a index_bound
    void trim_down_range_to(SizeType const& index_bound);
  public:
    //! \brief The minimum segment_distance from the sample
    PositiveFloatType const& minimum_distance() const;
    //! \brief The range of trace-sample index values where the minimum distance holds
    TraceSampleRange const& range() const;
    //! \brief Whether there is a collision, i.e., the minimum distance is zero
    bool is_collision() const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, MinimumDistanceBarrier const& s);

  private:
    PositiveFloatType const _minimum_distance;
    TraceSampleRange _range;
};

//! \brief The full sequence of minimum segment_distance barriers for a human-robot segment pair
class MinimumDistanceBarrierSequenceSectionInterface {
  public:
    //! \brief The human sample associated with the sequence
    virtual BodySegmentSample const& human_sample() const = 0;

    //! \brief The barrier at the given \a idx
    virtual MinimumDistanceBarrier const& barrier(SizeType const& idx) const = 0;
    //! \brief The last barrier
    virtual MinimumDistanceBarrier const& last_barrier() const = 0;
    //! \brief The number of barriers
    virtual SizeType size() const = 0;
    //! \brief Add a manual barrier to the end of the sequence
    virtual void add_barrier(PositiveFloatType const& minimum_distance, TraceSampleRange const& range) = 0;
    //! \brief Remove the first barrier of the trace
    //! \details Used when recognising that the barrier is outdated, i.e., before the current timestamp
    virtual void remove_first_barrier() = 0;
    //! \brief Remove the last barrier of the trace
    //! \details Used when updating resulted in a zero-distance barrier and we rather want to create a new section
    virtual void remove_last_barrier() = 0;
    //! \brief Check the human sample with a \a robot_sample at \a index to update the last barrier or create a new one
    //! \returns Return false if a collision is found. After a collision is found,
    //! the index for the corresponding zero distance is not updated on following checks.
    virtual bool check_and_update(BodySegmentSample const& robot_sample, TraceSampleIndex const& index) = 0;

    //! \brief Check if a \a human_sample and a \a robot_sample are colliding
    //! \returns This is used to check collision without updating, for those policies that require it
    virtual bool are_colliding(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const = 0;

    //! \brief Whether there is a collision, i.e., the minimum distance on the last barrier is zero
    virtual bool reaches_collision() const = 0;

    //! \brief The minimum segment_distance by the latest barrier
    virtual PositiveFloatType const& current_minimum_distance() const = 0;
    //! \brief The element in the barrier trace from which we can reuse checking on another \a human_sample
    //! \details If the whole trace still holds, then the result is next index
    //! A result of -1 instead means starting from scratch with respect to the samples used to generate this trace
    //! This is public for testing purposes only
    virtual int _reuse_element(BodySegmentSample const& human_sample) const = 0;
    //! \brief Whether the trace is currently empty
    virtual bool is_empty() const = 0;

    //! \brief The upper value of the trace index from the last barrier, 0 if the trace is empty
    virtual SizeType last_upper_trace_index() const = 0;

    //! \brief Reset the trace according to resuming obtained from using \a human_sample with a starting \a trace_index_range and \a sample_index
    virtual void reset(BodySegmentSample const& human_sample, Interval<SizeType> const& trace_index_range, SizeType const& sample_index) = 0;

    //! \brief Remove all the barriers, essentially invalidating the content without resetting
    virtual void clear() = 0;

    //! \brief Default virtual destructor
    virtual ~MinimumDistanceBarrierSequenceSectionInterface() = default;
};

//! \brief The base implementation for a barrier sequence
class MinimumDistanceBarrierSequenceSectionBase : public MinimumDistanceBarrierSequenceSectionInterface {
  protected:
    //! \brief Construct from a human sample
    MinimumDistanceBarrierSequenceSectionBase(BodySegmentSample const& human_sample);
  public:
    BodySegmentSample const& human_sample() const override;

    MinimumDistanceBarrier const& barrier(SizeType const& idx) const override;
    MinimumDistanceBarrier const& last_barrier() const override;
    SizeType last_upper_trace_index() const override;
    SizeType size() const override;
    void add_barrier(PositiveFloatType const& minimum_distance, TraceSampleRange const& range) override;
    void remove_first_barrier() override;
    void remove_last_barrier() override;
    bool check_and_update(BodySegmentSample const& robot_sample, TraceSampleIndex const& index) override;
    bool are_colliding(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const override;

    bool reaches_collision() const override;

    PositiveFloatType const& current_minimum_distance() const override;
    int _reuse_element(BodySegmentSample const& human_sample) const override;
    bool is_empty() const override;

    void reset(BodySegmentSample const& human_sample, Interval<SizeType> const& trace_index_range, SizeType const& sample_index) override;
    void clear() override;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, MinimumDistanceBarrierSequenceSectionBase const& t);

    //! \brief Compute the maximum distance that \a human_sample_other has with respect to \a human_sample_reference
    virtual PositiveFloatType maximum_human_human_distance(BodySegmentSample const& human_sample_reference, BodySegmentSample const& human_sample_other) const = 0;
    //! \brief Compute the minimum segment_distance between a human and a robot segment samples according to the specific trace policy
    virtual PositiveFloatType minimum_human_robot_distance(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const = 0;

  private:

    //! \brief Remove or reduce last barriers until the trace index range as bound by \a trace_index_bound
    void _trim_down_trace_index_ranges_to(SizeType const& trace_index_bound);
    //! \brief Scale down all index ranges of the given \a amount
    void _scale_down_trace_index_ranges_of(SizeType const& amount);

  private:
    BodySegmentSample const _human_sample;
    Deque<MinimumDistanceBarrier> _barriers;
};

//! \brief A barrier sequence where the bounding sphere of human samples are used to compute segment_distance
class SphereMinimumDistanceBarrierSequenceSection : public MinimumDistanceBarrierSequenceSectionBase {
  public:
    SphereMinimumDistanceBarrierSequenceSection(BodySegmentSample const& human_sample);
    SphereMinimumDistanceBarrierSequenceSection(SphereMinimumDistanceBarrierSequenceSection const& other) = default;
    SphereMinimumDistanceBarrierSequenceSection& operator=(SphereMinimumDistanceBarrierSequenceSection const& other) = default;
  protected:
    PositiveFloatType maximum_human_human_distance(BodySegmentSample const& human_sample_reference, BodySegmentSample const& human_sample_other) const override;
    PositiveFloatType minimum_human_robot_distance(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const override;
};

//! \brief A barrier sequence where the capsule representations of human samples are used to compute segment_distance
class CapsuleMinimumDistanceBarrierSequenceSection : public MinimumDistanceBarrierSequenceSectionBase {
  public:
    CapsuleMinimumDistanceBarrierSequenceSection(BodySegmentSample const& human_sample);
    CapsuleMinimumDistanceBarrierSequenceSection(CapsuleMinimumDistanceBarrierSequenceSection const& other) = default;
    CapsuleMinimumDistanceBarrierSequenceSection& operator=(CapsuleMinimumDistanceBarrierSequenceSection const& other) = default;
  protected:
    PositiveFloatType maximum_human_human_distance(BodySegmentSample const& human_sample_reference, BodySegmentSample const& human_sample_other) const override;
    PositiveFloatType minimum_human_robot_distance(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const override;
};

class MinimumDistanceBarrierSequenceSection : public Handle<MinimumDistanceBarrierSequenceSectionInterface> {
  public:
    using Handle<MinimumDistanceBarrierSequenceSectionInterface>::Handle;

    BodySegmentSample const& human_sample() const { return _ptr->human_sample(); }
    MinimumDistanceBarrier const& barrier(SizeType const& idx) const { return _ptr->barrier(idx); }
    MinimumDistanceBarrier const& last_barrier() const { return _ptr->last_barrier(); }
    SizeType size() const { return _ptr->size(); }
    void add_barrier(PositiveFloatType const& minimum_distance, TraceSampleRange const& range) { return _ptr->add_barrier(minimum_distance,range); }
    void remove_first_barrier() { _ptr->remove_first_barrier(); }
    void remove_last_barrier() { _ptr->remove_last_barrier(); }
    bool check_and_update(BodySegmentSample const& robot_sample, TraceSampleIndex const& index) { return _ptr->check_and_update(robot_sample, index); }
    bool are_colliding(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const { return _ptr->are_colliding(human_sample,robot_sample); }
    bool reaches_collision() const { return _ptr->reaches_collision(); }
    SizeType last_upper_trace_index() const { return _ptr->last_upper_trace_index(); }
    PositiveFloatType const& current_minimum_distance() const { return _ptr->current_minimum_distance(); }
    bool is_empty() const { return _ptr->is_empty(); }

    void reset(BodySegmentSample const& human_sample, Interval<SizeType> const& trace_index_range, SizeType const& sample_index) { return _ptr->reset(human_sample,trace_index_range,sample_index); }
    void clear() { return _ptr->clear(); }

    friend std::ostream& operator<<(std::ostream& os, MinimumDistanceBarrierSequenceSection const& t) { return os << dynamic_cast<MinimumDistanceBarrierSequenceSectionBase const&>(*t._ptr); }
};

//! \brief The interface for creating sequence sections
class MinimumDistanceBarrierSequenceSectionFactoryInterface {
  public:
    //! \brief Create a section associated with a given \a human_sample
    virtual MinimumDistanceBarrierSequenceSection create(BodySegmentSample const& human_sample) const = 0;
    //! \brief Create a deep copy of the given \a section
    virtual MinimumDistanceBarrierSequenceSection copy(MinimumDistanceBarrierSequenceSection const& section) const = 0;
    //! \brief Default virtual destructor
    virtual ~MinimumDistanceBarrierSequenceSectionFactoryInterface() = default;
};

class SphereMinimumDistanceBarrierSequenceSectionFactory : public MinimumDistanceBarrierSequenceSectionFactoryInterface {
  public:
    MinimumDistanceBarrierSequenceSection create(BodySegmentSample const& human_sample) const override;
    MinimumDistanceBarrierSequenceSection copy(MinimumDistanceBarrierSequenceSection const& section) const override;
};

class CapsuleMinimumDistanceBarrierSequenceSectionFactory : public MinimumDistanceBarrierSequenceSectionFactoryInterface {
  public:
    MinimumDistanceBarrierSequenceSection create(BodySegmentSample const& human_sample) const override;
    MinimumDistanceBarrierSequenceSection copy(MinimumDistanceBarrierSequenceSection const& section) const override;
};

class MinimumDistanceBarrierSequenceSectionFactory : public Handle<MinimumDistanceBarrierSequenceSectionFactoryInterface> {
  public:
    using Handle<MinimumDistanceBarrierSequenceSectionFactoryInterface>::Handle;
    MinimumDistanceBarrierSequenceSection create(BodySegmentSample const& human_sample) const { return _ptr->create(human_sample); }
    MinimumDistanceBarrierSequenceSection copy(MinimumDistanceBarrierSequenceSection const& section) const { return _ptr->copy(section); }
};

class MinimumDistanceBarrierSequence;

//! \brief Interface for handling the try_update on the last section of the barrier sequence
class MinimumDistanceBarrierSequenceUpdatePolicyInterface {
  public:
    virtual bool check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const = 0;
    virtual ~MinimumDistanceBarrierSequenceUpdatePolicyInterface() = default;
};

//! \brief Keep one section only, resetting when distance gets to zero
class KeepOneMinimumDistanceBarrierSequenceUpdatePolicy : public MinimumDistanceBarrierSequenceUpdatePolicyInterface {
  public:
    bool check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const override;
};

//! \brief Add a new section as soon as distance gets to zero in the last section
class AddWhenNecessaryMinimumDistanceBarrierSequenceUpdatePolicy : public MinimumDistanceBarrierSequenceUpdatePolicyInterface {
public:
    bool check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const override;
};

//! \brief Add a new section as soon as the human sample changes
class AddWhenDifferentMinimumDistanceBarrierSequenceUpdatePolicy : public MinimumDistanceBarrierSequenceUpdatePolicyInterface {
public:
    bool check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const override;
};

class MinimumDistanceBarrierSequenceUpdatePolicy : public Handle<MinimumDistanceBarrierSequenceUpdatePolicyInterface> {
  public:
    using Handle<MinimumDistanceBarrierSequenceUpdatePolicyInterface>::Handle;
    bool check_and_update_section(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const {
        return _ptr->check_and_update(sequence, human_sample, robot_sample, index);
    }
};

//! \brief A full barrier sequence, made of successive sections pieced together
class MinimumDistanceBarrierSequence {
  public:
    //! \brief Construct from the human and robot segment ids, along with a factory for the sequence sections
    MinimumDistanceBarrierSequence(MinimumDistanceBarrierSequenceSectionFactory const& section_factory, MinimumDistanceBarrierSequenceUpdatePolicy const& update_policy);
    //! \brief Copy constructor
    MinimumDistanceBarrierSequence(MinimumDistanceBarrierSequence const& other);

    //! \brief Assignment operator
    MinimumDistanceBarrierSequence& operator=(MinimumDistanceBarrierSequence const& other);

    //! \brief The last barrier across all sections
    MinimumDistanceBarrier const& last_barrier() const;

    //! \brief The last section
    MinimumDistanceBarrierSequenceSection& last_section();
    //! \brief The last section
    MinimumDistanceBarrierSequenceSection const& last_section() const;

    //! \brief The upper value of the trace index from the last barrier, 0 if the trace is empty
    SizeType last_upper_trace_index() const;

    //! \brief Add a pre-constructed section
    MinimumDistanceBarrierSequence& add(MinimumDistanceBarrierSequenceSection const& section);
    //! \brief Add a section using the \a human_sample
    void add_from(BodySegmentSample const& human_sample);

    //! \brief Check the \a human_sample with a \a robot_sample at \a index to update the last barrier or create a new one
    //! \returns Return false if no update has been performed, which is due to a collision
    bool check_and_update(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index);

    //! \brief Reset the trace according to resuming obtained from using \a human_sample with given \a trace_index_range and \a sample_index
    void reset(BodySegmentSample const& human_sample, Interval<SizeType> const& trace_index_range, SizeType const& sample_index);

    //! \brief Remove all the sections
    void clear();

    //! \brief Check if the sequence is empty
    bool is_empty() const;

    //! \brief The size of the sequence in terms of its sections
    SizeType num_sections() const;

    //! \brief The size of the sequence in terms of its barriers
    SizeType num_barriers() const;

    //! \brief Whether the sequence reaches collision, i.e., the last barrier on the last section has distance equal to zero
    bool reaches_collision() const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, MinimumDistanceBarrierSequence const& p);

  private:
    MinimumDistanceBarrierSequenceSectionFactory _section_factory;
    MinimumDistanceBarrierSequenceUpdatePolicy _update_policy;
    List<MinimumDistanceBarrierSequenceSection> _sections;
};

}

#endif //OPERA_BARRIER_HPP
