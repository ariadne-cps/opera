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
        BodyStateMessage bs("robot0", Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), {{}, {Point(0, -1, 0.1), Point(0.3, 3.1, -1.2)}, {}}, 93249042230);
        BodyPresentationMessage bp("human1", {{0, 1},{3, 2}}, {1.0,0.5});
        CollisionNotificationMessage cn("h0", 0, "r0", 3, 32890592300, Interval<TimestampType>(72, 123), Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), 0.5);

        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        auto cn_publisher = access.make_collision_notification_publisher();

        bp_publisher->put(bp);
        bs_publisher->put(bs);
        cn_publisher->put(cn);

        OPERA_TEST_EQUALS(MemoryBroker::instance().size<BodyPresentationMessage>(),1)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<BodyStateMessage>(),1)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<CollisionNotificationMessage>(),1)

        MemoryBroker::instance().clear();

        OPERA_TEST_EQUALS(MemoryBroker::instance().size<BodyPresentationMessage>(),0)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<BodyStateMessage>(),0)
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<CollisionNotificationMessage>(),0)

        delete bp_publisher;
        delete bs_publisher;
        delete cn_publisher;
    }
};

int main() {
    BrokerAccess access = MemoryBrokerAccess();
    TestBrokerAccess(access).test();
    TestBrokerClear().test();
    return OPERA_TEST_FAILURES;
}
