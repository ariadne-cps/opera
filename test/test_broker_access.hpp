/***************************************************************************
 *            test_broker_access.hpp
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

#include <thread>
#include "utility.hpp"
#include "broker.hpp"

#include "test.hpp"

using namespace Opera;

class TestBrokerAccess {
  private:
    BrokerAccess const& _access;
  public:
    TestBrokerAccess(BrokerAccess const& access) : _access(access) { }

    void test() {
        OPERA_TEST_CALL(test_create_destroy())
        OPERA_TEST_CALL(test_single_transfer())
        OPERA_TEST_CALL(test_multiple_transfer())
    }

    void test_create_destroy() {
        BodyStateMessage p("robot0", Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), {{}, {Point(0, -1, 0.1), Point(0.3, 3.1, -1.2)}, {}}, 93249042230);

        OPERA_PRINT_TEST_COMMENT("Creating subscriber and removing it")
        auto* subscriber = _access.make_body_state_subscriber([](auto){});
        OPERA_TEST_EXECUTE(delete subscriber)

        OPERA_PRINT_TEST_COMMENT("Creating publisher and removing it immediately")
        auto* publisher1 = _access.make_body_state_publisher();
        OPERA_TEST_EXECUTE(delete publisher1)

        OPERA_PRINT_TEST_COMMENT("Creating publisher and removing it after publishing")
        auto* publisher2 = _access.make_body_state_publisher();
        publisher2->put(p);
        OPERA_TEST_EXECUTE(delete publisher2)
    }

    void test_single_transfer() {
        BodyPresentationMessage hp("human1", {{0, 1},{3, 2}}, {1.0,0.5});
        List<BodyPresentationMessage> bp_received;

        auto bp_subscriber = _access.make_body_presentation_subscriber([&](auto p){ bp_received.push_back(p); });
        auto bp_publisher = _access.make_body_presentation_publisher();
        bp_publisher->put(hp);
        SizeType i=0;
        while (bp_received.size() != 1) {
            ++i;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        OPERA_PRINT_TEST_COMMENT("Took " << i << " ms to acknowledge the reception")

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        delete bp_subscriber;
        delete bp_publisher;
    }

    void test_multiple_transfer() {
        BodyPresentationMessage hp("human1", {{0, 1},{3, 2}}, {1.0,0.5});
        BodyPresentationMessage rp("robot1", 30, {{0, 1},{3, 2},{4, 2}}, {1.0,0.5, 0.5});
        BodyStateMessage hs("human0",{{Point(0.4,2.1,0.2)},{Point(0,-1,0.1),Point(0.3,3.1,-1.2)},{Point(0.4,0.1,1.2)},{Point(0,0,1)}},3423235253290);
        BodyStateMessage rs("robot0", Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), {{}, {Point(0, -1, 0.1), Point(0.3, 3.1, -1.2)}, {}}, 93249042230);
        CollisionNotificationMessage cn("h0", 0, "r0", 3, 32890592300, Interval<TimestampType>(72, 123), Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), 0.5);

        List<BodyPresentationMessage> bp_received;
        List<BodyStateMessage> bs_received;
        List<CollisionNotificationMessage> cn_received;

        auto bp_subscriber = _access.make_body_presentation_subscriber([&](auto p){ bp_received.push_back(p); });
        auto bs_subscriber = _access.make_body_state_subscriber([&](auto p){ bs_received.push_back(p); });
        auto cn_subscriber = _access.make_collision_notification_subscriber([&](auto p){ cn_received.push_back(p); });
        auto bp_publisher = _access.make_body_presentation_publisher();
        auto bs_publisher = _access.make_body_state_publisher();
        auto cn_publisher = _access.make_collision_notification_publisher();
        bp_publisher->put(hp);
        bp_publisher->put(rp);
        bs_publisher->put(hs);
        bs_publisher->put(rs);
        cn_publisher->put(cn);
        SizeType i=0;
        while (bp_received.size() != 2 or bs_received.size() != 2 or cn_received.size() != 1) {
            ++i;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        OPERA_PRINT_TEST_COMMENT("Took " << i << " ms to acknowledge the reception")

        delete bp_subscriber;
        delete bs_subscriber;
        delete cn_subscriber;
        delete bp_publisher;
        delete bs_publisher;
        delete cn_publisher;
    }
};
