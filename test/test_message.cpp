/***************************************************************************
 *            test_message.cpp
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

#include "message.hpp"

using namespace Opera;

class TestMessage {
public:
    void test() {
        OPERA_TEST_CALL(test_human_presentation_message_create())
        OPERA_TEST_CALL(test_robot_presentation_message_create())
        OPERA_TEST_CALL(test_human_state_message_create())
        OPERA_TEST_CALL(test_robot_state_message_create())
        OPERA_TEST_CALL(test_notification_message_create())
    }

    void test_human_presentation_message_create() {
        BodyPresentationMessage p("h0",{{0,1},{1,2}},{1.0,0.5});
        OPERA_TEST_EQUALS(p.id(),"h0")
        OPERA_TEST_ASSERT(p.is_human())
        OPERA_TEST_EQUALS(p.point_ids().size(),2)
        OPERA_TEST_EQUALS(p.thicknesses().size(),2)
    }

    void test_robot_presentation_message_create() {
        BodyPresentationMessage p("r0",10,{{0,1},{1,2}},{1.0,0.5});
        OPERA_TEST_EQUALS(p.id(),"r0")
        OPERA_TEST_ASSERT(not p.is_human())
        OPERA_TEST_EQUALS(p.message_frequency(),10)
        OPERA_TEST_EQUALS(p.point_ids().size(),2)
        OPERA_TEST_EQUALS(p.thicknesses().size(),2)
    }

    void test_human_state_message_create() {
        BodyStateMessage p("h0",{{Point(0,0,0)},{Point(0,2,0)}},300);
        OPERA_TEST_EQUALS(p.id(),"h0")
        OPERA_TEST_ASSERT(p.mode().is_empty())
        OPERA_TEST_EQUALS(p.points().size(),2)
        OPERA_TEST_EQUALS(p.timestamp(),300)
    }

    void test_robot_state_message_create() {
        Mode loc({"r0", "first"});
        BodyStateMessage p("r0",loc,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},200);
        OPERA_TEST_EQUALS(p.id(),"r0")
        OPERA_TEST_EQUALS(p.mode(),loc)
        OPERA_TEST_EQUALS(p.points().size(),3)
        OPERA_TEST_EQUALS(p.timestamp(),200)
    }

    void test_notification_message_create() {
        Mode loc({"r0", "first"});
        CollisionNotificationMessage p("h0",1,"r0",4,200,Interval<TimestampType>(1000,2000),loc,1.0);

        OPERA_TEST_EQUALS(p.human_id(),"h0")
        OPERA_TEST_EQUALS(p.human_segment_id(),1)
        OPERA_TEST_EQUALS(p.robot_id(),"r0")
        OPERA_TEST_EQUALS(p.robot_segment_id(),4)
        OPERA_TEST_EQUALS(p.current_time(),200)
        OPERA_TEST_EQUALS(p.collision_distance().lower(), 1000)
        OPERA_TEST_EQUALS(p.collision_distance().upper(), 2000)
        OPERA_TEST_EQUALS(p.collision_mode(), loc)
        OPERA_TEST_EQUALS(p.likelihood(),1.0)
    }
};


int main() {
    TestMessage().test();

    return OPERA_TEST_FAILURES;
}
