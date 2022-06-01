/***************************************************************************
 *            body.cpp
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

#include "mode.hpp"
#include "macros.hpp"
#include "body.hpp"
#include "conclog/include/logging.hpp"

namespace Opera {

Body::Body(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& points_ids, List<FloatType> const& thicknesses) :
    _id(id) {
    OPERA_ASSERT_MSG(points_ids.size() == thicknesses.size(), "The number of point pairs must equal the number of thicknesses")

    Set<KeypointIdType> keypoints;

    for (List<SegmentIndexType>::size_type i=0; i<thicknesses.size(); ++i) {
        KeypointIdType const& first = points_ids.at(i).first;
        KeypointIdType const& second = points_ids.at(i).second;
        if (not keypoints.contains(first)) { keypoints.insert(first); _keypoint_ids.push_back(first); }
        if (not keypoints.contains(second)) { keypoints.insert(second); _keypoint_ids.push_back(second); }
        _segments.push_back({this,static_cast<SegmentIndexType>(i),first,second,thicknesses.at(i)});
    }
}

Body::Body(Body const& other) : _id(other._id), _keypoint_ids(other._keypoint_ids) {
    for (auto s : other._segments)
        _segments.push_back({this, s.index(), s.head_id(), s.tail_id(), s.thickness()});
}

BodyIdType const& Body::id() const {
    return _id;
}

List<KeypointIdType> const& Body::keypoint_ids() const {
    return _keypoint_ids;
}

BodySegment const& Body::segment(SizeType const& idx) const {
    return _segments.at(idx);
}

SizeType Body::num_segments() const {
    return _segments.size();
}

SizeType Body::num_points() const {
    return _keypoint_ids.size();
}

std::ostream& operator<<(std::ostream& os, Body const& b) {
    os << "(id=" << b.id() << ", segments=[";
    for (SizeType i=0; i<b.num_segments()-1; ++i) {
        os << b.segment(i) << ",";
    }
    os << b.segment(b.num_segments()-1) << "])";
    return os;
}

Human::Human(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& points_ids, List<FloatType> const& thicknesses) :
    Body(id,points_ids,thicknesses) { }

Robot::Robot(BodyIdType const& id, SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& points_ids, List<FloatType> const& thicknesses) :
    Body(id,points_ids,thicknesses), _message_frequency(message_frequency) {
    OPERA_ASSERT_MSG(message_frequency > 0, "The message frequency must be strictly positive")
}

SizeType const& Robot::message_frequency() const {
    return _message_frequency;
}

BodySegment::BodySegment(Body const* body, SegmentIndexType const& id, KeypointIdType const& head_id, KeypointIdType const& tail_id, FloatType const& thickness) :
        _index(id), _head_id(head_id), _tail_id(tail_id), _thickness(thickness), _body(body) { }

SegmentIndexType const& BodySegment::index() const {
    return _index;
}

KeypointIdType const& BodySegment::head_id() const {
    return _head_id;
}

KeypointIdType const& BodySegment::tail_id() const {
    return _tail_id;
}

FloatType const& BodySegment::thickness() const {
    return _thickness;
}

BodySegmentSample BodySegment::create_sample() const {
    return BodySegmentSample(this);
}

BodySegmentSample BodySegment::create_sample(List<Point> const& begin, List<Point> const& end) const {
    BodySegmentSample result(this);
    result.update(begin,end);
    return result;
}

std::ostream& operator<<(std::ostream& os, BodySegment const& s) {
    return os << "(body_id=" << s._body->id() << ", index=" << s._index << ", head_id=" << s._head_id << ", tail_id=" << s._tail_id << ", thickness=" << s._thickness << ")";
}

BodySegmentSample::BodySegmentSample(BodySegment const* segment) :
        _segment(segment), _is_empty(true),
        _head_bounds(Box::make_empty()),
        _tail_bounds(_head_bounds),
        _head_centre(Point(NAN,NAN,NAN)),
        _tail_centre(_head_centre),
        _radius(0.0),
        _bb(nullptr),
        _bs(nullptr)
        { }

SegmentIndexType const& BodySegmentSample::segment_index() const {
    return _segment->index();
}

Point const& BodySegmentSample::head_centre() const {
    return _head_centre;
}

Point const& BodySegmentSample::tail_centre() const {
    return _tail_centre;
}

FloatType const& BodySegmentSample::error() const {
    return _radius;
}

FloatType const& BodySegmentSample::thickness() const {
    return _segment->thickness();
}

void BodySegmentSample::update(List<Point> const& heads, List<Point> const& tails) {
    auto const hs = heads.size();
    auto const ts = tails.size();
    auto common_size = std::min(hs,ts);
    for (SizeType j=0; j<common_size; ++j)
        _update(heads.at(j), tails.at(j));
    for (SizeType j=common_size; j<hs; ++j)
        _update_head(heads.at(j));
    for (SizeType j=common_size; j<ts; ++j)
        _update_tail(tails.at(j));
    if (_is_empty and (not _head_bounds.is_empty() and not _tail_bounds.is_empty()))
        _is_empty = false;
    if (not heads.empty())
        _head_centre = _head_bounds.centre();
    if (not tails.empty())
        _tail_centre = _tail_bounds.centre();
    if (not _is_empty) _recalculate_radius_bounding_sets();
}

void BodySegmentSample::_update(Point const& head, Point const& tail) {
    _update_head(head);
    _update_tail(tail);
}

void BodySegmentSample::_update_head(Point const& head) {
    _head_bounds = Box(std::min(_head_bounds.xl(),head.x),std::max(_head_bounds.xu(),head.x),
                       std::min(_head_bounds.yl(),head.y),std::max(_head_bounds.yu(),head.y),
                       std::min(_head_bounds.zl(),head.z),std::max(_head_bounds.zu(),head.z));
}

void BodySegmentSample::_update_tail(Point const& tail) {
    _tail_bounds = Box(std::min(_tail_bounds.xl(),tail.x),std::max(_tail_bounds.xu(),tail.x),
                       std::min(_tail_bounds.yl(),tail.y),std::max(_tail_bounds.yu(),tail.y),
                       std::min(_tail_bounds.zl(),tail.z),std::max(_tail_bounds.zu(),tail.z));
}

void BodySegmentSample::_recalculate_radius_bounding_sets() {
    _radius = std::max(_head_bounds.circle_radius(),_tail_bounds.circle_radius());
    if (_bb != nullptr) _bb = std::make_shared<Box>(widen(hull(_head_centre,_tail_centre),_radius+_segment->thickness()));
    if (_bs != nullptr) _bs = std::make_shared<Sphere>(centre(_head_centre,_tail_centre),distance(_head_centre,_tail_centre)/2+_radius+thickness());
}

Box const& BodySegmentSample::bounding_box() const {
    if (_bb == nullptr) _bb = std::make_shared<Box>(widen(hull(_head_centre,_tail_centre),_radius+_segment->thickness()));
    return *_bb;
}

Sphere const& BodySegmentSample::bounding_sphere() const {
    if (_bs == nullptr) _bs = std::make_shared<Sphere>(centre(_head_centre,_tail_centre),distance(_head_centre,_tail_centre)/2+_radius+thickness());
    return *_bs;
}

bool BodySegmentSample::is_empty() const {
    return _is_empty;
}

bool BodySegmentSample::intersects(BodySegmentSampleInterface const& other) const {
    if (bounding_box().disjoint(other.bounding_box())) return false;
    else return segment_distance(*this, other) <= this->thickness() + this->error() + other.thickness() + other.error();
}

std::ostream& operator<<(std::ostream& os, BodySegmentSampleInterface const& s) {
    return os << "(h=" << s.head_centre() << ",t=" << s.tail_centre() << ")";
}

bool operator!=(BodySegmentSample const& first, BodySegmentSample const& second) {
    if (first.segment_index() != second.segment_index()) return true;
    if (first.error() != second.error()) return true;
    if (first.thickness() != second.thickness()) return true;
    if (first.head_centre().is_undefined() or second.head_centre().is_undefined()) {
        if (first.head_centre().is_undefined() xor second.head_centre().is_undefined()) return true;
    } else if (first.head_centre() != second.head_centre()) return true;
    if (first.tail_centre().is_undefined() or second.tail_centre().is_undefined()) {
        if (first.tail_centre().is_undefined() xor second.tail_centre().is_undefined()) return true;
    } else if (first.tail_centre() != second.tail_centre()) return true;
    return false;
}

bool operator==(BodySegmentSample const& first, BodySegmentSample const& second) {
    return not (first != second);
}

FloatType segment_distance(BodySegmentSampleInterface const& s1, BodySegmentSampleInterface const& s2) {
    return distance(s1.head_centre(), s1.tail_centre(), s2.head_centre(), s2.tail_centre());
}

PositiveFloatType sphere_capsule_distance(Sphere const& sample, BodySegmentSample const& other) {
    return std::max(0.0,distance(sample.centre(),other.head_centre(),other.tail_centre())-other.error()-other.thickness()-sample.radius());
}

}