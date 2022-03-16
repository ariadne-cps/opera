/***************************************************************************
 *            test_geometry.cpp
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

#include "test.hpp"

#include "geometry.hpp"

using namespace Opera;

class TestGeometry {
public:
    void test() {
        OPERA_TEST_CALL(test_construct_point())
        OPERA_TEST_CALL(test_segment_segment_distance())
        OPERA_TEST_CALL(test_point_segment_distance())
        OPERA_TEST_CALL(test_point_point_distance())
        OPERA_TEST_CALL(test_centre())
        OPERA_TEST_CALL(test_hull())
        OPERA_TEST_CALL(test_average())
        OPERA_TEST_CALL(test_geometric_median())
        OPERA_TEST_CALL(test_circle_radius())
        OPERA_TEST_CALL(test_sphere_create())
    }

    void test_construct_point() {
        Point p(NAN,NAN,NAN);
        OPERA_TEST_ASSERT(p.is_undefined())

        Point p2(1.0,-2.1,0);
        OPERA_TEST_ASSERT(not p2.is_undefined())
    }

    void test_segment_segment_distance() {
        OPERA_TEST_EQUALS(distance(Point(1,2,3),Point(1,2,3),Point(1,2,3),Point(1,2,3)),0)
        OPERA_TEST_EQUALS(distance(Point(1,2,3),Point(3,4,5),Point(1,2,3),Point(3,4,5)),0)
        OPERA_TEST_EQUALS(distance(Point(1,2,3),Point(0,0,0),Point(0,0,0),Point(2,2,2)),0)
        OPERA_TEST_EQUALS(distance(Point(0,0,0),Point(0,3,0),Point(0,4,0),Point(0,2,0)),0)
        OPERA_TEST_EQUALS(distance(Point(1,0,0),Point(3,0,0),Point(1,1,0),Point(3,1,0)),1)
        OPERA_TEST_EQUALS(distance(Point(1,0,0),Point(3,0,0),Point(0,0,0),Point(0,2,0)),1)

        OPERA_TEST_EXECUTE(distance(Point(-0.5073,-0.3273,-0.6143),Point(-0.8391,0.8633,-0.1950),Point(-0.2479,-0.6319,0.2624),Point(0.3919,-0.1700,0.8694)))
        OPERA_TEST_EXECUTE(distance(Point(0.7283,0.4762,0.1598),Point(0.6620,-0.07481,0.08877),Point(-0.6654,-0.6032,-0.9962),Point(-0.6910,-0.8980,-0.5835)))
        OPERA_TEST_EXECUTE(distance(Point(0.9076,-0.5889,-0.3511),Point(0.9236,0.6130,-0.9872),Point(-0.8805,-0.2538,0.6383),Point(-0.2311,-0.5325,0.9485)))
        OPERA_TEST_EXECUTE(distance(Point(-0.9097,-0.4835,0.3973),Point(-0.2489,-0.1628,-0.5455),Point(0.3303,0.9305,-0.1387),Point(0.7753,0.3848,0.9415)))
    }

    void test_point_segment_distance() {
        auto d1 = distance(Point(1.308,-2.690,1.567),Point(1.308,-2.690,1.567),Point(-1.174,4.631,-0.1193),Point(-4.892,-2.183,-3.825));
        OPERA_TEST_ASSERT(distance(Point(1.308,-2.690,1.567),Point(-1.174,4.631,-0.1193),Point(-4.892,-2.183,-3.825))-d1 < 1e-8)
        auto d2 = distance(Point(-0.1053,-0.1488,-2.390),Point(-0.1053,-0.1488,-2.390),Point(2.964,-1.106,0.4021),Point(2.887,-3.345,2.290));
        OPERA_TEST_ASSERT(distance(Point(-0.1053,-0.1488,-2.390),Point(2.964,-1.106,0.4021),Point(2.887,-3.345,2.290))-d2 < 1e-8)
        auto d3 = distance(Point(-1.560,3.773,-4.831),Point(-1.560,3.773,-4.831),Point(1.941,-1.352,3.894),Point(-4.736,0.9957,0.6373));
        OPERA_TEST_ASSERT(distance(Point(-1.560,3.773,-4.831),Point(1.941,-1.352,3.894),Point(-4.736,0.9957,0.6373))-d3 < 1e-8)
    }

    void test_point_point_distance() {
        OPERA_TEST_EQUALS(distance(Point(1,2,3),Point(1,2,3)),0)
        OPERA_TEST_EQUALS(distance(Point(1,2,3),Point(-1,2,3)),2)
        OPERA_TEST_EQUALS(distance(Point(1,2,3),Point(4,-2,3)),5)
    }

    void test_centre() {
        Point p1(1.0,3.0,-2.0);
        Point p2(4.0,1.2,0);

        auto c = centre(p1, p2);

        OPERA_TEST_EQUALS(c.x,2.5)
        OPERA_TEST_EQUALS(c.y,2.1)
        OPERA_TEST_EQUALS(c.z,-1.0)
    }

    void test_hull() {
        Point p1(4.0,3.0,-2.0);
        Point p2(4.0,1.2,0);

        auto h = hull(p1,p2);

        OPERA_TEST_EQUALS(h.xl(),4.0)
        OPERA_TEST_EQUALS(h.xu(),4.0)
        OPERA_TEST_EQUALS(h.yl(),1.2)
        OPERA_TEST_EQUALS(h.yu(),3.0)
        OPERA_TEST_EQUALS(h.zl(),-2.0)
        OPERA_TEST_EQUALS(h.zu(),0)
    }

    void test_average() {
        Point p1(4.0,3.0,-2.0);
        Point p2(4.0,1.0,0);
        Point p3(19.0,-4.0,5.0);

        OPERA_TEST_FAIL(average({}))

        OPERA_TEST_EQUAL(average({p1}),p1)

        auto avg = average({p1,p2,p3});
        OPERA_TEST_EQUALS(avg,Point(9.0,0.0,1.0))
        OPERA_TEST_PRINT(avg)
    }

    void test_geometric_median() {
        Point p1(4.0,3.0,-2.0);
        Point p2(4.0,1.0,0);
        Point p3(19.0,-4.0,5.0);

        OPERA_TEST_FAIL(geometric_median({}))

        OPERA_TEST_EQUAL(geometric_median({p1}),p1)

        auto median = geometric_median({p1,p2,p3});
        OPERA_TEST_PRINT(median)
    }

    void test_circle_radius() {
        Box bb(1,2,-1,2,4,6);
        OPERA_TEST_ASSERT(bb.circle_radius() - 1.8708 < 1e-3)
    }

    void test_sphere_create() {
        Sphere s(Point(0,0,0),1.0);
        OPERA_TEST_EQUALS(s.centre(),Point(0,0,0))
        OPERA_TEST_EQUALS(s.radius(), 1.0)
    }
};


int main() {
    TestGeometry().test();

    return OPERA_TEST_FAILURES;
}
