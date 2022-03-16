/***************************************************************************
 *            geometry.hpp
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

#ifndef OPERA_GEOMETRY_HPP
#define OPERA_GEOMETRY_HPP

#include <ostream>
#include <math.h>

#include "declarations.hpp"

namespace Opera {

struct Point {
    Point(FloatType x_, FloatType y_, FloatType z_) : x(x_), y(y_), z(z_) { }
    FloatType x;
    FloatType y;
    FloatType z;

    Point& operator+=(Point const& other);

    bool is_undefined() const { return std::isnan(x) and std::isnan(y) and std::isnan(z); }

    friend std::ostream& operator<<(std::ostream& os, const Point& v) {
        return os << "(" << v.x << "," << v.y << "," << v.z << ")"; }
};

inline Point& Point::operator+=(Point const& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

inline FloatType dot(Point const& v1, Point const& v2) {
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

inline Point operator-(Point const& v1, Point const& v2) {
    return Point({v1.x-v2.x, v1.y-v2.y, v1.z-v2.z });
}

inline Point operator+(Point const& v1, Point const& v2) {
    return Point({v1.x+v2.x, v1.y+v2.y, v1.z+v2.z });
}

inline Point operator*(FloatType const& s, Point const& v) {
    return Point({s*v.x, s*v.y, s*v.z});
}

inline Point operator/(Point const& v, FloatType const& c) {
    return Point({v.x/c, v.y/c, v.z/c});
}

inline Point operator+(Point const& v, FloatType const& s) {
    return Point({v.x+s, v.y+s, v.z+s });
}

inline Point operator*(Point const& v, FloatType const& s) {
    return s*v;
}

inline Point operator*(Point const& v1, Point const& v2) {
    return Point({v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x});
}

inline bool operator==(Point const& v1, Point const& v2) {
    return v1.x == v2.x and v1.y == v2.y and v1.z == v2.z;
}

//! \brief A box in the coordinate 3D space
class Box {
  public:
    Box(FloatType const& xl, FloatType const& xu, FloatType const& yl, FloatType const& yu, FloatType const& zl, FloatType const& zu);

    //! \brief Make an empty box
    static Box make_empty();

    //! \brief Whether the box is empty
    bool is_empty() const;

    FloatType const& xl() const;
    FloatType const& xu() const;
    FloatType const& yl() const;
    FloatType const& yu() const;
    FloatType const& zl() const;
    FloatType const& zu() const;

    //! \brief The centre point
    Point centre() const;
    //! \brief The radius of the circle inscribing the box
    FloatType circle_radius() const;

    //! \brief Check whether the boxes have any common point
    bool disjoint(Box const& other) const;

    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, Box const& b);

  private:
    FloatType _xl;
    FloatType _xu;
    FloatType _yl;
    FloatType _yu;
    FloatType _zl;
    FloatType _zu;
};

//! \brief A sphere represented by a centre and a radius
class Sphere {
  public:
    Sphere(Point const& centre, FloatType const& radius);

    //! \brief Return the centre
    Point const& centre() const;
    //! \brief Return the radius
    FloatType const& radius() const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, Sphere const& s);

  private:
    Point _centre;
    FloatType _radius;
};

//! \brief The centre of the segment joining two points
Point centre(Point const& p1, Point const& p2);

//! \brief The minimum bounding box enclosing the two points \a p1 and \a p2
Box hull(Point const& p1, Point const& p2);

//! \brief Compute the average point
Point average(List<Point> const& pts);

//! \brief Compute the geometric median point
Point geometric_median(List<Point> const& pts);

//! \brief The minimum segment_distance between a segment s1 (with head/tail points s1h and s1t) and segment s2
//! (with head/tail points s2h and s2t)
FloatType distance(Point const& s1h, Point const& s1t, Point const& s2h, Point const& s2t);

//! \brief The minimum segment_distance between a point p1 and segment s2
//! (with head/tail points s2h and s2t)
FloatType distance(Point const& p1, Point const& s2h, Point const& s2t);

//! \brief The segment_distance between two points
FloatType distance(Point const& p1, Point const& p2);

//! \brief Return the widening of \a bb of \a v in all directions
Box widen(Box const& bb, FloatType const& v);

}

#endif //OPERA_GEOMETRY_HPP
