/***************************************************************************
 *            barrier.cpp
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

#include "macros.hpp"
#include "barrier.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

MinimumDistanceBarrier::MinimumDistanceBarrier(PositiveFloatType const& minimum_distance, TraceSampleRange const& range) :
        _minimum_distance(minimum_distance), _range(range) {}

PositiveFloatType const& MinimumDistanceBarrier::minimum_distance() const {
    return _minimum_distance;
}

bool MinimumDistanceBarrier::is_collision() const {
    return _minimum_distance == 0;
}

TraceSampleRange const& MinimumDistanceBarrier::range() const {
    return _range;
}

void MinimumDistanceBarrier::update_with(TraceSampleIndex const& index) {
    if (index.trace > _range.maximum_trace_index()) {
        for (SizeType i=0; i< index.trace - _range.maximum_trace_index(); ++i)
            _range.increase_trace_index();
    } else _range.update(index.sample);
}

void MinimumDistanceBarrier::scale_down_range_of(SizeType const& amount) {
    _range.scale_down_trace_of(amount);
}

void MinimumDistanceBarrier::trim_down_range_to(SizeType const& index_bound) {
    _range.trim_down_trace_to(index_bound);
}

std::ostream &operator<<(std::ostream &os, MinimumDistanceBarrier const& s) {
    return os << "(d:" << s.minimum_distance() << ", r:" << s.range() << ")";
}

MinimumDistanceBarrierSequenceSectionBase::MinimumDistanceBarrierSequenceSectionBase(BodySegmentSample const& human_sample) :
    _human_sample(human_sample) { }

BodySegmentSample const& MinimumDistanceBarrierSequenceSectionBase::human_sample() const {
    return _human_sample;
}

MinimumDistanceBarrier const& MinimumDistanceBarrierSequenceSectionBase::barrier(SizeType const& idx) const {
    return _barriers.at(idx);
}

MinimumDistanceBarrier const& MinimumDistanceBarrierSequenceSectionBase::last_barrier() const {
    return _barriers.at(_barriers.size()-1);
}

SizeType MinimumDistanceBarrierSequenceSectionBase::last_upper_trace_index() const {
    return is_empty() ? 0 : last_barrier().range().maximum_trace_index();
}

SizeType MinimumDistanceBarrierSequenceSectionBase::size() const {
    return _barriers.size();
}

void MinimumDistanceBarrierSequenceSectionBase::add_barrier(PositiveFloatType const& minimum_distance, TraceSampleRange const& range) {
    _barriers.emplace_back(MinimumDistanceBarrier(minimum_distance, range));
}

void MinimumDistanceBarrierSequenceSectionBase::remove_first_barrier() {
    OPERA_PRECONDITION(not is_empty())
    _barriers.pop_front();
}

void MinimumDistanceBarrierSequenceSectionBase::remove_last_barrier() {
    OPERA_PRECONDITION(not is_empty())
    _barriers.pop_back();
}

bool MinimumDistanceBarrierSequenceSectionBase::check_and_update(BodySegmentSample const& robot_sample, TraceSampleIndex const& index) {
    CONCLOG_SCOPE_CREATE
    if (reaches_collision()) {
        CONCLOG_PRINTLN("Distance already reached zero, do not do anything")
        return false;
    }
    auto current_distance = minimum_human_robot_distance(_human_sample, robot_sample);
    if (is_empty() or current_distance < current_minimum_distance()) {
        CONCLOG_PRINTLN("Distance reduced (or section is empty), add barrier")
        add_barrier(current_distance,index);
    } else if (current_distance >= current_minimum_distance()) {
        CONCLOG_PRINTLN("Not empty and distance not reduced, update the index for the range")
        _barriers.back().update_with(index);
    }
    return (current_distance > 0);
}

bool MinimumDistanceBarrierSequenceSectionBase::are_colliding(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const {
    return minimum_human_robot_distance(human_sample,robot_sample) == 0;
}

bool MinimumDistanceBarrierSequenceSectionBase::reaches_collision() const {
    if (is_empty()) return false;
    else return last_barrier().is_collision();
}

int MinimumDistanceBarrierSequenceSectionBase::_reuse_element(BodySegmentSample const& other_human_sample) const {
    if (is_empty()) return -1;
    SizeType lower = 0;
    SizeType upper = _barriers.size() - 1;
    if (maximum_human_human_distance(_human_sample, other_human_sample) >= _barriers.at(lower).minimum_distance()) return -1;
    if (maximum_human_human_distance(_human_sample, other_human_sample) < _barriers.at(upper).minimum_distance()) return static_cast<int>(upper);
    SizeType result = (upper + lower) / 2;
    while (upper > lower + 1) {
        if (maximum_human_human_distance(_human_sample, other_human_sample) >= _barriers.at(result).minimum_distance()) upper = result;
        else lower = result;
        result = (upper + lower) / 2;
    }
    return static_cast<int>(result);
}


void MinimumDistanceBarrierSequenceSectionBase::_trim_down_trace_index_ranges_to(SizeType const& trace_index_bound) {
    while(not is_empty()) {
        if (last_barrier().range().maximum_trace_index() > trace_index_bound) {
            if (last_barrier().range().initial().trace <= trace_index_bound) {
                _barriers.back().trim_down_range_to(trace_index_bound);
                break;
            } else _barriers.pop_back();
        } else break;
    }
}

void MinimumDistanceBarrierSequenceSectionBase::_scale_down_trace_index_ranges_of(SizeType const& amount) {
    if (amount > 0) for (auto& b : _barriers) b.scale_down_range_of(amount);
}

void MinimumDistanceBarrierSequenceSectionBase::reset(BodySegmentSample const& human_sample, Interval<SizeType> const& trace_index_range, SizeType const& sample_index) {
    _trim_down_trace_index_ranges_to(trace_index_range.upper());
    int reuse = _reuse_element(human_sample);
    if (reuse >= 0) {
        auto ul_reuse = static_cast<SizeType>(reuse);
        while (_barriers.size()-1 > ul_reuse) _barriers.pop_back();
        while (not _barriers.empty() and (_barriers.at(0).range().maximum_trace_index() < trace_index_range.lower() or (_barriers.at(
                0).range().maximum_trace_index() == trace_index_range.lower() and
                _barriers.at(0).range().maximum_sample_index() < sample_index))) _barriers.pop_front();
        _scale_down_trace_index_ranges_of(trace_index_range.lower());
    } else {
        _barriers.clear();
    }
}

bool MinimumDistanceBarrierSequenceSectionBase::is_empty() const {
    return _barriers.empty();
}

void MinimumDistanceBarrierSequenceSectionBase::clear() {
    _barriers.clear();
}

PositiveFloatType const& MinimumDistanceBarrierSequenceSectionBase::current_minimum_distance() const {
    if (is_empty()) return infinity;
    else return _barriers.back().minimum_distance();
}

std::ostream &operator<<(std::ostream &os, MinimumDistanceBarrierSequenceSectionBase const& t) {
    List<MinimumDistanceBarrier> barriers;
    for (auto const& b: t._barriers) barriers.push_back(b);
    return os << t._human_sample << ";" << barriers;
}

SphereMinimumDistanceBarrierSequenceSection::SphereMinimumDistanceBarrierSequenceSection(BodySegmentSample const& human_sample) :
        MinimumDistanceBarrierSequenceSectionBase(human_sample) {}

PositiveFloatType SphereMinimumDistanceBarrierSequenceSection::maximum_human_human_distance(BodySegmentSample const& human_sample_reference, BodySegmentSample const& human_sample_other) const {
    auto const& sphere_reference = human_sample_reference.bounding_sphere();
    auto const& sphere_other = human_sample_other.bounding_sphere();
    return std::max(0.0,distance(sphere_reference.centre(), sphere_other.centre()) + sphere_other.radius() - sphere_reference.radius());
}

PositiveFloatType SphereMinimumDistanceBarrierSequenceSection::minimum_human_robot_distance(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const {
    return sphere_capsule_distance(human_sample.bounding_sphere(), robot_sample);
}

CapsuleMinimumDistanceBarrierSequenceSection::CapsuleMinimumDistanceBarrierSequenceSection(BodySegmentSample const& human_sample) :
        MinimumDistanceBarrierSequenceSectionBase(human_sample) {}

PositiveFloatType CapsuleMinimumDistanceBarrierSequenceSection::maximum_human_human_distance(BodySegmentSample const& human_sample_reference, BodySegmentSample const& human_sample_other) const {
    return std::max(0.0,std::max(distance(human_sample_other.head_centre(), human_sample_reference.head_centre(), human_sample_reference.tail_centre()),
                    distance(human_sample_other.tail_centre(), human_sample_reference.head_centre(), human_sample_reference.tail_centre()))
           + human_sample_other.thickness() + human_sample_other.error() - human_sample_reference.thickness() - human_sample_reference.error());
}

PositiveFloatType CapsuleMinimumDistanceBarrierSequenceSection::minimum_human_robot_distance(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample) const {
    auto distance = segment_distance(human_sample, robot_sample);
    auto epsilon = human_sample.error() + human_sample.thickness() + robot_sample.error() + robot_sample.thickness();
    return (distance <= epsilon ? 0.0 : distance - epsilon);
}

MinimumDistanceBarrierSequenceSection SphereMinimumDistanceBarrierSequenceSectionFactory::create(BodySegmentSample const& human_sample) const {
    return SphereMinimumDistanceBarrierSequenceSection(human_sample);
}

MinimumDistanceBarrierSequenceSection SphereMinimumDistanceBarrierSequenceSectionFactory::copy(MinimumDistanceBarrierSequenceSection const& section) const {
    auto const& sphere_section = dynamic_cast<SphereMinimumDistanceBarrierSequenceSection const&>(*section.ptr());
    return SphereMinimumDistanceBarrierSequenceSection(sphere_section);
}

MinimumDistanceBarrierSequenceSection CapsuleMinimumDistanceBarrierSequenceSectionFactory::create(BodySegmentSample const& human_sample) const {
    return CapsuleMinimumDistanceBarrierSequenceSection(human_sample);
}

MinimumDistanceBarrierSequenceSection CapsuleMinimumDistanceBarrierSequenceSectionFactory::copy(MinimumDistanceBarrierSequenceSection const& section) const {
    auto const& capsule_section = dynamic_cast<CapsuleMinimumDistanceBarrierSequenceSection const&>(*section.ptr());
    return CapsuleMinimumDistanceBarrierSequenceSection(capsule_section);
}

bool KeepOneMinimumDistanceBarrierSequenceUpdatePolicy::check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const {
    bool result;
    auto& section = sequence.last_section();
    if (sequence.last_section().human_sample() != human_sample) {
        result = (not section.are_colliding(human_sample,robot_sample));
        section.check_and_update(robot_sample, index);
    } else {
        result = section.check_and_update(robot_sample, index);
    }
    return result;
}

bool AddWhenDifferentMinimumDistanceBarrierSequenceUpdatePolicy::check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const {
    if (sequence.last_section().human_sample() != human_sample) {
        if (not sequence.reaches_collision()) {
            sequence.add_from(human_sample);
            return sequence.last_section().check_and_update(robot_sample, index);
        } else {
            auto section_base = dynamic_cast<MinimumDistanceBarrierSequenceSectionBase const*>(sequence.last_section().ptr());
            return (section_base->minimum_human_robot_distance(human_sample,robot_sample) > 0);
        }
    } else return sequence.last_section().check_and_update(robot_sample, index);
}

bool AddWhenNecessaryMinimumDistanceBarrierSequenceUpdatePolicy::check_and_update(MinimumDistanceBarrierSequence& sequence, BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) const {
    bool result;
    if (sequence.last_section().human_sample() != human_sample) {
        if (not sequence.reaches_collision() and not sequence.last_section().check_and_update(robot_sample, index)) {
            sequence.last_section().remove_last_barrier();
            sequence.add_from(human_sample);
            result = sequence.last_section().check_and_update(robot_sample, index);
        } else {
            auto section_base = dynamic_cast<MinimumDistanceBarrierSequenceSectionBase const*>(sequence.last_section().ptr());
            result = (section_base->minimum_human_robot_distance(human_sample,robot_sample) > 0);
        }
    } else {
        result = sequence.last_section().check_and_update(robot_sample, index);
    }
    return result;
}

MinimumDistanceBarrierSequence::MinimumDistanceBarrierSequence(MinimumDistanceBarrierSequenceSectionFactory const& section_factory, MinimumDistanceBarrierSequenceUpdatePolicy const& update_policy)
    : _section_factory(section_factory), _update_policy(update_policy) { }

MinimumDistanceBarrierSequence::MinimumDistanceBarrierSequence(MinimumDistanceBarrierSequence const& other)
    : _section_factory(other._section_factory), _update_policy(other._update_policy) {
    for (auto const& s : other._sections) {
        _sections.push_back(_section_factory.copy(s));
    }
}
MinimumDistanceBarrierSequence& MinimumDistanceBarrierSequence::operator=(MinimumDistanceBarrierSequence const& other) {
    _section_factory = other._section_factory;
    _sections.clear();
    for (auto const& s : other._sections) {
        _sections.push_back(_section_factory.copy(s));
    }
    return *this;
}

MinimumDistanceBarrier const& MinimumDistanceBarrierSequence::last_barrier() const {
    return last_section().last_barrier();
}

MinimumDistanceBarrierSequenceSection& MinimumDistanceBarrierSequence::last_section() {
    OPERA_PRECONDITION(not is_empty())
    return _sections.back();
}

MinimumDistanceBarrierSequenceSection const& MinimumDistanceBarrierSequence::last_section() const {
    OPERA_PRECONDITION(not is_empty())
    return _sections.back();
}

SizeType MinimumDistanceBarrierSequence::last_upper_trace_index() const {
    if (is_empty()) return 0;
    return last_section().last_upper_trace_index();
}

bool MinimumDistanceBarrierSequence::is_empty() const {
    return _sections.empty();
}

SizeType MinimumDistanceBarrierSequence::num_sections() const {
    return _sections.size();
}

SizeType MinimumDistanceBarrierSequence::num_barriers() const {
    SizeType result = 0;
    for (auto const& s: _sections) result += s.size();
    return result;
}

bool MinimumDistanceBarrierSequence::reaches_collision() const {
    if (is_empty()) return false;
    else return last_section().reaches_collision();
}

void MinimumDistanceBarrierSequence::clear() {
    _sections.clear();
}

MinimumDistanceBarrierSequence& MinimumDistanceBarrierSequence::add(MinimumDistanceBarrierSequenceSection const& section) {
    _sections.emplace_back(section);
    return *this;
}

void MinimumDistanceBarrierSequence::add_from(BodySegmentSample const& human_sample) {
    _sections.emplace_back(_section_factory.create(human_sample));
}

bool MinimumDistanceBarrierSequence::check_and_update(BodySegmentSample const& human_sample, BodySegmentSample const& robot_sample, TraceSampleIndex const& index) {
    if (_sections.empty()) _sections.emplace_back(_section_factory.create(human_sample));
    auto result = _update_policy.check_and_update_section(*this, human_sample, robot_sample, index);
    if (last_section().is_empty()) _sections.pop_back();
    return result;
}

void MinimumDistanceBarrierSequence::reset(BodySegmentSample const& human_sample, Interval<SizeType> const& trace_index_range, SizeType const& sample_index) {
    List<MinimumDistanceBarrierSequenceSection> result;
    for (auto& s : _sections) {
        auto original_size = s.size();
        s.reset(human_sample,trace_index_range,sample_index);
        if (not s.is_empty()) result.push_back(s);
        if (s.size() < original_size) break;
    }
    _sections = result;
}

std::ostream& operator<<(std::ostream& os, MinimumDistanceBarrierSequence const& p) {
    os << "{";
    if (not p.is_empty()) {
        for (SizeType i=0; i<p._sections.size()-1; ++i)
            os << p._sections.at(i) << "&";
        os << p._sections.at(p._sections.size()-1);
    }
    return os << "}";
}

}