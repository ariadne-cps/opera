/***************************************************************************
 *            geometry.cpp
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

#include <cmath>
#include "macros.hpp"
#include "geometry.hpp"

namespace Opera {

Point centre(Point const& p1, Point const& p2) {
    return (p1+p2)/2;
}

Box hull(Point const& p1, Point const& p2) {
    return {std::min(p1.x, p2.x), std::max(p1.x, p2.x), std::min(p1.y, p2.y), std::max(p1.y, p2.y), std::min(p1.z, p2.z), std::max(p1.z, p2.z)};
}

Point average(List<Point> const& pts) {
    OPERA_ASSERT(not pts.empty())
    FloatType ax = 0, ay = 0, az = 0;
    for (auto const& p : pts) {
        ax += p.x;
        ay += p.y;
        az += p.z;
    }
    return {ax/pts.size(), ay/pts.size(), az/pts.size()};
}

Point geometric_median(List<Point> const& pts) {

    SizeType const NUM_ITERATIONS = 200;
    FloatType const CONVERGENCE_THRESHOLD = 0.05;

    if (pts.size() == 1) return pts.at(0);

    Point r = average(pts);

    bool convergence = false;
    List<FloatType> dist;

    SizeType i = 0;
    while (not convergence and i < NUM_ITERATIONS) {

        FloatType denum = 0, d = 0;
        Point num(0,0,0);

        for (auto const& p : pts) {
            auto div = distance(p,r);
            num += p/div;

            denum += 1.0 / div;
            d += pow(div,2);
        }
        dist.push_back(d);

        r = num/denum;

        if (i > 3)
            convergence = (std::abs(dist.at(i) - dist.at(i - 2))/dist.at(i) < CONVERGENCE_THRESHOLD);

        ++i;
    }

    OPERA_ASSERT_MSG(i < NUM_ITERATIONS, "Maximum iterations of the geometric median has been reached")

    return r;
}

FloatType distance(Point const& s1h, Point const& s1t, Point const& s2h, Point const& s2t) {

    const FloatType SMALL_VALUE = 1e-6;

    auto u = s1t - s1h;
    auto v = s2t - s2h;
    auto w = s1h - s2h;

    FloatType a = dot(u, u);
    FloatType b = dot(u, v);
    FloatType c = dot(v, v);
    FloatType d = dot(u, w);
    FloatType e = dot(v, w);
    FloatType D = a * c - b * b;
    FloatType sc, sN, sD = D;
    FloatType tc, tN, tD = D;
    if (D < SMALL_VALUE) {
        sN = 0;
        sD = 1;
        tN = e;
        tD = c;
    } else {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0) {
            sN = 0;
            tN = e;
            tD = c;
        } else if (sN > sD) {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }
    if (tN < 0) {
        tN = 0;
        if (-d < 0) {
            sN = 0;
        } else if (-d > a) {
            sN = sD;
        } else {
            sN = -d;
            sD = a;
        }
    } else if (tN > tD) {
        tN = tD;
        if ((-d + b) < 0) {
            sN = 0;
        } else if ((-d + b) > a) {
            sN = sD;
        } else {
            sN = (-d + b);
            sD = a;
        }
    }

    if (std::abs(sN) < SMALL_VALUE) sc = 0;
    else sc = sN / sD;

    if (std::abs(tN) < SMALL_VALUE) tc = 0;
    else tc = tN / tD;

    auto dP = w + (sc * u) - (tc * v);

    return sqrt(dot(dP, dP));
}

FloatType distance(Point const& p1, Point const& s2h, Point const& s2t) {

    const FloatType SMALL_VALUE = 1e-6;

    auto v = s2t - s2h;
    auto w = p1 - s2h;

    FloatType c = dot(v, v);
    FloatType e = dot(v, w);
    FloatType tc = 0.0, tN = e, tD = c;

    if (tN < 0)
        tN = 0;
    else if (tN > tD)
        tN = tD;

    if (std::abs(tN) >= SMALL_VALUE)
        tc = tN / tD;

    auto dP = w - (tc * v);

    return sqrt(dot(dP, dP));
}

FloatType distance(Point const& p1, Point const& p2) {
    return sqrt(dot(p1-p2,p1-p2));
}

Box::Box(FloatType const& xl, FloatType const& xu, FloatType const& yl, FloatType const& yu, FloatType const& zl, FloatType const& zu)
    : _xl(xl), _xu(xu), _yl(yl), _yu(yu), _zl(zl), _zu(zu) { }

Box Box::make_empty() {
    return {infinity,-infinity,infinity,-infinity,infinity,-infinity};
}

bool Box::is_empty() const {
    return (_xl > _xu or _yl > _yu or _zl > _zu);
}

FloatType const& Box::xl() const { return _xl; }

FloatType const& Box::xu() const { return _xu; }

FloatType const& Box::yl() const { return _yl; }

FloatType const& Box::yu() const { return _yu; }

FloatType const& Box::zl() const { return _zl; }

FloatType const& Box::zu() const { return _zu; }

Point Box::centre() const {
    OPERA_PRECONDITION(not is_empty())
    return {(_xl+_xu)/2,(_yl+_yu)/2,(_zl+_zu)/2};
}

FloatType Box::circle_radius() const {
    return sqrt(pow(_xu-_xl,2)+pow(_yu-_yl,2)+pow(_zu-_zl,2))/2;
}

bool Box::disjoint(Box const& other) const {
    return _xu < other._xl or _xl > other._xu or _yu < other._yl or _yl > other._yu or _zu < other._zl or _zl > other._zu;
}

Box widen(Box const& bb, FloatType const& v) {
    return {bb.xl()-v,bb.xu()+v,bb.yl()-v,bb.yu()+v,bb.zl()-v,bb.zu()+v};
}

Sphere::Sphere(Point const& centre, FloatType const& radius) : _centre(centre), _radius(radius) { }

Point const& Sphere::centre() const {
    return _centre;
}

FloatType const& Sphere::radius() const {
    return _radius;
}

std::ostream& operator<<(std::ostream& os, Sphere const& s) {
    return os << "(centre: " << s.centre() << ", radius: " << s.radius() << ")";
}

std::ostream& operator<<(std::ostream& os, Box const& b) {
    return os << "{[" << b.xl() << ":" << b.xu() <<"],[" << b.yl() << ":" << b.yu() << "],[" << b.zl() << ":" << b.zu() << "]}";
}

}