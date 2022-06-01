/***************************************************************************
 *            test_memory.cpp
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
#include "test_broker_access.hpp"
#include "memory.hpp"

using namespace Opera;

class TestBrokerClear {
public:
    void test() {
        OPERA_TEST_CALL(test_clear())
    }

    void test_clear() {
        MemoryBroker::instance().clear();
        MemoryBrokerAccess access;
        HumanStateMessage hs({{"h0",{{{"head",{Point(0,0,0)}},{"neck",{Point(0,2,0)}}}}}},300);
        RobotStateMessage rs("robot0", Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), {{}, {Point(0, -1, 0.1), Point(0.3, 3.1, -1.2)}, {}}, 93249);
        BodyPresentationMessage bp("human1", {{"nose", "neck"},{"left_shoulder", "right_shoulder"}}, {1.0,0.5});
        CollisionNotificationMessage cn("h0", {"nose","neck"}, "r0", {"0","1"}, 32890, Interval<TimestampType>(72, 123), Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), 0.5);

        auto bp_publisher = access.make_body_presentation_publisher();
        auto hs_publisher = access.make_human_state_publisher();
        auto rs_publisher = access.make_robot_state_publisher();
        auto cn_publisher = access.make_collision_notification_publisher();

        bp_publisher->put(bp);
        hs_publisher->put(hs);
        rs_publisher->put(rs);
        cn_publisher->put(cn);

        OPERA_TEST_EQUALS(MemoryBroker::instance().size<BodyPresentationMessage>(),1)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<HumanStateMessage>(),1)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<RobotStateMessage>(),1)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<CollisionNotificationMessage>(),1)

        MemoryBroker::instance().clear();

        OPERA_TEST_EQUALS(MemoryBroker::instance().size<BodyPresentationMessage>(),0)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<HumanStateMessage>(),0)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<RobotStateMessage>(),0)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<CollisionNotificationMessage>(),0)

        delete bp_publisher;
        delete hs_publisher;
        delete rs_publisher;
        delete cn_publisher;
    }
};

int main() {
    BrokerAccess access = MemoryBrokerAccess();
    TestBrokerAccess(access).test();
    TestBrokerClear().test();
    return OPERA_TEST_FAILURES;
}
