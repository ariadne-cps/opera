/***************************************************************************
 *            body.hpp
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

#ifndef OPERA_BODY_HPP
#define OPERA_BODY_HPP

#include "declarations.hpp"
#include "mode.hpp"
#include "geometry.hpp"

namespace Opera {

using SegmentIndexType = unsigned int;
using KeypointIdType = std::string;
using BodyIdType = std::string;

class BodySegment;

//! \brief A generic body having segments
class Body {
  protected:
    //! \brief Construct from fields
    Body(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& points_ids, List<FloatType> const& thicknesses);
    //! \brief Copy constructor
    Body(Body const& other);
  public:
    //! \brief The body identifier
    BodyIdType const& id() const;

    //! \brief The identifiers for each keypoint, with defined order
    List<KeypointIdType> const& keypoint_ids() const;

    //! \brief Return the segment indexed by \a idx
    BodySegment const& segment(SizeType const& idx) const;

    //! \brief The number of segments in the body
    SizeType num_segments() const;

    //! \brief The number of points in the body
    //! \details Useful for consistency checks when receiving body samples
    SizeType num_points() const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, Body const& b);

  private:
    BodyIdType const _id;
    List<KeypointIdType> _keypoint_ids;
  protected:
    List<BodySegment> _segments;
};

//! \brief A human is a body able to get a singular state
class Human : public Body {
  public:
    //! \brief Construct from fields
    Human(BodyIdType const& id, List<Pair<KeypointIdType,KeypointIdType>> const& points_ids, List<FloatType> const& thicknesses);
};

//! \brief A robot is a body able to have its history
class Robot : public Body {
  public:
    //! \brief Construct from fields
    Robot(BodyIdType const& id, SizeType const& message_frequency, List<Pair<KeypointIdType,KeypointIdType>> const& points_ids, List<FloatType> const& thicknesses);
    //! \brief The frequency of messages sent by the robot, in Hz
    SizeType const& message_frequency() const;
  private:
    SizeType const _message_frequency;
};

class BodySegmentSample;

class BodySegment {
    friend class Body;
  public:
    //! \brief Construct from body and its index in it, head_centre/tail_centre identifiers and thickness
    BodySegment(Body const* body, SegmentIndexType const& index, KeypointIdType const& head_id, KeypointIdType const& tail_id, FloatType const& thickness);
  public:
    //! \brief Index for the segment within the specific body
    SegmentIndexType const& index() const;

    //! \brief Identifier for the head
    KeypointIdType const& head_id() const;

    //! \brief Identifier for the tail
    KeypointIdType const& tail_id() const;

    //! \brief Return the thickness of the body segment around the geometrical segment
    FloatType const& thickness() const;

    //! \brief Create an is_empty sample
    BodySegmentSample create_sample() const;

    //! \brief Create a sample for the segment from a list of \a begin points and \a end points
    BodySegmentSample create_sample(List<Point> const& begin, List<Point> const& end) const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, BodySegment const& s);

  private:
    SegmentIndexType const _index;
    KeypointIdType const _head_id;
    KeypointIdType const _tail_id;
    FloatType const _thickness;
    Body const* _body;
};

class BodySegmentSampleInterface {
  public:
    //! \brief Return the identifier of the segment
    virtual SegmentIndexType const& segment_index() const = 0;

    //! \brief Return the centre point for the head of the segment
    virtual Point const& head_centre() const = 0;
    //! \brief Return the centre point for the tail of the segment
    virtual Point const& tail_centre() const = 0;

    //! \brief Return the maximum spherical error in the segment head/tail positions,
    //! as obtained from the centres with respect to the bounds
    virtual FloatType const& error() const = 0;

    //! \brief The thickness of the segment
    virtual FloatType const& thickness() const = 0;

    //! \brief Return the bounding box overapproximation
    virtual Box const& bounding_box() const = 0;

    //! \brief Return the bounding sphere overapproximation
    virtual Sphere const& bounding_sphere() const = 0;

    //! \brief Whether the segment is empty, i.e., whether either head and tail are is_empty
    virtual bool is_empty() const = 0;

    //! \brief Update the head and tail bounds from the given lists of points
    virtual void update(List<Point> const& heads, List<Point> const& tails) = 0;

    //! \brief Whether it intersects an \a other segment
    //! \details Returns true also in the case of tangency
    virtual bool intersects(BodySegmentSampleInterface const& other) const = 0;

    //! \brief Calculate the minimum segment_distance between two segment samples
    friend FloatType segment_distance(BodySegmentSampleInterface const& s1, BodySegmentSampleInterface const& s2);

    //! \brief Compare for inequality
    friend bool operator!=(BodySegmentSample const& first, BodySegmentSample const& second);
    //! \brief Compare for equality
    friend bool operator==(BodySegmentSample const& first, BodySegmentSample const& second);

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, BodySegmentSampleInterface const& s);
};

class BodySegmentSample: public BodySegmentSampleInterface {
    friend class BodySegment;
  public:
    //! \brief Create empty
    BodySegmentSample(BodySegment const* segment);

    SegmentIndexType const& segment_index() const override;

    Point const& head_centre() const override;
    Point const& tail_centre() const override;

    FloatType const& error() const override;
    FloatType const& thickness() const override;
    Box const& bounding_box() const override;
    Sphere const& bounding_sphere() const override;

    bool is_empty() const override;

    void update(List<Point> const& heads, List<Point> const& tails) override;

    bool intersects(BodySegmentSampleInterface const& other) const override;

  private:
    //! \brief Update head and tail bounds, without recalculation of metrics
    void _update(Point const& head, Point const& tail);
    //! \brief Update only the head bounds, without recalculation of metrics
    void _update_head(Point const& head);
    //! \brief Update only the tail bounds, without recalculation of metrics
    void _update_tail(Point const& tail);
    //! \brief Re-calculate the centres, error and bounding box/sphere from the bounds
    void _recalculate_radius_bounding_sets();

  private:
    BodySegment const* _segment;
    bool _is_empty;

    Box _head_bounds;
    Box _tail_bounds;

    Point _head_centre;
    Point _tail_centre;
    FloatType _radius;
    mutable SharedPointer<Box> _bb;
    mutable SharedPointer<Sphere> _bs;
};

//! \brief The segment_distance between a sphere and a segment sample
PositiveFloatType sphere_capsule_distance(Sphere const& sample, BodySegmentSample const& other);

}

#endif //OPERA_BODY_HPP
