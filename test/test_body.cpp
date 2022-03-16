/***************************************************************************
 *            test_body.cpp
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

#include "body.hpp"

#include "test.hpp"

using namespace Opera;

class TestBody {
public:
    void test() {
        OPERA_TEST_CALL(test_body_creation())
        OPERA_TEST_CALL(test_bodysegmentsample_creation())
        OPERA_TEST_CALL(test_bodysegmentsample_update())
        OPERA_TEST_CALL(test_bodysegmentsample_compare())
        OPERA_TEST_CALL(test_bodysegmentsample_intersection())
        OPERA_TEST_CALL(test_bounding_box())
        OPERA_TEST_CALL(test_bounding_sphere())
    }

    void test_body_creation() {
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});

        OPERA_TEST_PRINT(h)
        OPERA_TEST_EQUALS(h.id(),"h0")
        OPERA_TEST_EQUALS(h.num_segments(),2)
        OPERA_TEST_EQUALS(h.num_points(),4)

        Robot r("r0", 10, {{0,1}}, {0.5});
        OPERA_TEST_PRINT(r)
        OPERA_TEST_EQUALS(r.id(),"r0")
        OPERA_TEST_EQUALS(r.num_segments(),1)
        OPERA_TEST_EQUALS(r.num_points(),2)
        OPERA_TEST_EQUALS(r.message_frequency(), 10)
    }

    void test_bodysegmentsample_creation() {

        Robot r("r0", 10, {{3, 2},{1, 0}}, {1.0, 0.5});
        auto segment = r.segment(1);

        OPERA_TEST_EQUALS(segment.id(),1)
        OPERA_TEST_EQUALS(segment.head_id(),1)
        OPERA_TEST_EQUALS(segment.tail_id(),0)
        OPERA_TEST_EQUALS(segment.thickness(),0.5)

        auto s1 = segment.create_sample();
        OPERA_TEST_ASSERT(s1.is_empty())
    }

    void test_bodysegmentsample_update() {

        FloatType thickness = 1.0;

        Robot r("r0", 10, {{0, 1}}, {thickness});
        auto segment = r.segment(0);

        auto s1 = segment.create_sample();
        s1.update({Point(-0.5, 1.0, 1.25)},{});
        OPERA_TEST_ASSERT(s1.is_empty())
        OPERA_TEST_EQUALS(s1.head_centre().x, -0.5)
        OPERA_TEST_EQUALS(s1.head_centre().y, 1.0)
        OPERA_TEST_EQUALS(s1.head_centre().z, 1.25)
        s1.update({},{Point(1.0, 2.5, 0.0)});
        OPERA_TEST_EQUALS(s1.tail_centre().x, 1.0)
        OPERA_TEST_EQUALS(s1.tail_centre().y, 2.5)
        OPERA_TEST_EQUALS(s1.tail_centre().z, 0.0)
        OPERA_TEST_ASSERT(not s1.is_empty())

        auto s2 = segment.create_sample();
        s2.update({},{Point(1.0, 2.5, 0.0)});
        OPERA_TEST_ASSERT(s2.is_empty())
        OPERA_TEST_EQUALS(s2.tail_centre().x, 1.0)
        OPERA_TEST_EQUALS(s2.tail_centre().y, 2.5)
        OPERA_TEST_EQUALS(s2.tail_centre().z, 0.0)
        s2.update({Point(-0.5, 1.0, 1.25)},{});
        OPERA_TEST_EQUALS(s2.head_centre().x, -0.5)
        OPERA_TEST_EQUALS(s2.head_centre().y, 1.0)
        OPERA_TEST_EQUALS(s2.head_centre().z, 1.25)
        OPERA_TEST_ASSERT(not s2.is_empty())

        auto s3 = segment.create_sample();
        s3.update({Point(-0.5, 1.0, 1.25)}, {Point(1.0, 2.5, 0.0)});
        OPERA_TEST_ASSERT(not s3.is_empty())

        auto s4 = segment.create_sample();
        s4.update({Point(0,0.5,1.0),Point(-0.5, 1.0, 1.25)},{Point(1.0,2.0,-1.0),Point(1.0, 2.5, 0.0)});
        auto err = s4.error();

        OPERA_TEST_EQUALS(s4.head_centre().x, -0.25)
        OPERA_TEST_EQUALS(s4.head_centre().y, 0.75)
        OPERA_TEST_EQUALS(s4.head_centre().z, 1.125)
        OPERA_TEST_EQUALS(s4.tail_centre().x, 1.0)
        OPERA_TEST_EQUALS(s4.tail_centre().y, 2.25)
        OPERA_TEST_EQUALS(s4.tail_centre().z, -0.5)

        auto bb = s4.bounding_box();
        OPERA_TEST_EQUALS(bb.xl(),s4.head_centre().x-err-thickness)
        OPERA_TEST_EQUALS(bb.xu(),s4.tail_centre().x+err+thickness)
        OPERA_TEST_EQUALS(bb.yl(),s4.head_centre().y-err-thickness)
        OPERA_TEST_EQUALS(bb.yu(),s4.tail_centre().y+err+thickness)
        OPERA_TEST_EQUALS(bb.zl(),s4.tail_centre().z-err-thickness)
        OPERA_TEST_EQUALS(bb.zu(),s4.head_centre().z+err+thickness)
    }

    void test_bodysegmentsample_compare() {
        Robot r("r0", 10, {{3, 2},{1, 0}}, {1.0, 0.5});
        auto segment = r.segment(1);

        auto s1 = segment.create_sample();
        s1.update({Point(-0.5, 1.0, 1.25)},{});

        auto s2 = segment.create_sample();
        s2.update({},{Point(1.0, 2.5, 0.0)});

        auto s3 = segment.create_sample();
        s3.update({},{Point(1.0, 2.5, 0.0)});

        auto s4 = segment.create_sample();
        auto s5 = segment.create_sample();

        OPERA_TEST_ASSERT(not (s1 == s2))
        OPERA_TEST_EQUALS(s2,s3)
        OPERA_TEST_EQUALS(s4,s5)
    }

    void test_bodysegmentsample_intersection() {
        FloatType thickness = 1.0;
        Robot r("r0", 10,{{0, 1}}, {thickness});
        auto segment = r.segment(0);

        auto s1 = segment.create_sample({Point(0, 0, 0)}, {Point(5, 5, 5)});
        auto s2 = segment.create_sample({Point(0, 3, 0)}, {Point(5, 5, 5)});
        auto s3 = segment.create_sample({Point(0, 3, 0)}, {Point(5, 6, 5)});
        auto s4 = segment.create_sample({Point(0, 3, 3)}, {Point(0, 8, 8)});
        auto s5 = segment.create_sample({Point(2.01, 3, 3)}, {Point(2.01, 5, 5)});
        auto s6 = segment.create_sample({Point(2, 3, 3)}, {Point(2, 5, 5)});
        auto s7 = segment.create_sample({Point(0, 8, 0)}, {Point(0, 10, 0)});

        OPERA_TEST_PRINT(s1.bounding_box())
        OPERA_TEST_PRINT(s2.bounding_box())
        OPERA_TEST_PRINT(s3.bounding_box())
        OPERA_TEST_PRINT(s4.bounding_box())
        OPERA_TEST_PRINT(s5.bounding_box())
        OPERA_TEST_PRINT(s6.bounding_box())
        OPERA_TEST_PRINT(s7.bounding_box())

        OPERA_TEST_PRINT(segment_distance(s1, s2))
        OPERA_TEST_PRINT(segment_distance(s1, s3))
        OPERA_TEST_PRINT(segment_distance(s1, s4))
        OPERA_TEST_PRINT(segment_distance(s4, s5))
        OPERA_TEST_PRINT(segment_distance(s4, s6))

        OPERA_TEST_ASSERT(s1.intersects(s2))
        OPERA_TEST_ASSERT(s1.intersects(s3))
        OPERA_TEST_ASSERT(not s1.intersects(s4))
        OPERA_TEST_ASSERT(not s4.intersects(s5))
        OPERA_TEST_ASSERT(s4.intersects(s6))
        OPERA_TEST_ASSERT(not s1.intersects(s7))
    }

    void test_bounding_box() {
        Human h("h0", {{3, 2},{1, 0}}, {1.0, 0.5});
        auto human_sample = h.segment(1).create_sample();
        human_sample.update({Point(1,5,0)},{Point(3,5,0)});

        auto human_sbb = human_sample.bounding_box();
        OPERA_TEST_PRINT(human_sbb)
        OPERA_TEST_EQUAL(human_sbb.centre(),Point(2,5,0))
        OPERA_TEST_EQUAL(human_sbb.xl(),0.5)
        OPERA_TEST_EQUAL(human_sbb.xu(),3.5)
        OPERA_TEST_EQUAL(human_sbb.yl(),4.5)
        OPERA_TEST_EQUAL(human_sbb.yu(),5.5)
        OPERA_TEST_EQUAL(human_sbb.zl(),-0.5)
        OPERA_TEST_EQUAL(human_sbb.zu(),0.5)
    }

    void test_bounding_sphere() {
        Robot r("r0", 10, {{3, 2},{1, 0}}, {1.0, 0.5});
        auto robot_sample = r.segment(0).create_sample();
        robot_sample.update({Point(0,0,0)},{Point(2,0,0)});
        auto human_sample = r.segment(1).create_sample();
        human_sample.update({Point(1,5,0)},{Point(3,5,0)});

        auto human_sbs = human_sample.bounding_sphere();
        OPERA_TEST_PRINT(human_sbs)
        OPERA_TEST_EQUAL(human_sbs.centre(),Point(2,5,0))
        OPERA_TEST_EQUALS(human_sbs.radius(),1.5)
        OPERA_TEST_EQUALS(sphere_capsule_distance(human_sbs, robot_sample), 2.5)
    }
};


int main() {
    TestBody().test();

    return OPERA_TEST_FAILURES;
}
