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
#include "broker_access.hpp"
#include "serialisation.hpp"

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
        HumanStateMessage hs({{"human0",{{{"head",{Point(0,0,0)}},{"neck",{Point(0,2,0)}}}}}},300);

        OPERA_PRINT_TEST_COMMENT("Creating subscriber and removing it")
        auto* subscriber = _access.make_human_state_subscriber([](auto){});
        OPERA_TEST_EXECUTE(delete subscriber)

        OPERA_PRINT_TEST_COMMENT("Creating publisher and removing it immediately")
        auto* publisher1 = _access.make_human_state_publisher();
        OPERA_TEST_EXECUTE(delete publisher1)

        OPERA_PRINT_TEST_COMMENT("Creating publisher and removing it after publishing")
        auto* publisher2 = _access.make_human_state_publisher();
        publisher2->put(hs);
        OPERA_TEST_EXECUTE(delete publisher2)
    }

    void test_single_transfer() {
        BodyPresentationMessage hp("human1", {{"nose", "neck"},{"left_shoulder", "right_shoulder"}}, {1.0,0.5});
        List<BodyPresentationMessage> bp_received;

        auto bp_subscriber = _access.make_body_presentation_subscriber([&](auto p){ bp_received.push_back(p);
            OPERA_PRINT_TEST_COMMENT("Message received: " << Serialiser<BodyPresentationMessage>(p).to_string()) });
        auto bp_publisher = _access.make_body_presentation_publisher();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
        BodyPresentationMessage hp("human1", {{"nose", "neck"},{"left_shoulder", "right_shoulder"}}, {1.0,0.5});
        BodyPresentationMessage rp("robot1", 30, {{"0", "1"},{"3", "2"},{"4", "2"}}, {1.0,0.5, 0.5});
        HumanStateMessage hs({{"human0",{{{"head",{Point(0,0,0)}},{"neck",{Point(0,2,0)}},{"left_shoulder",{Point(1,2,0)}},{"right_shoulder",{Point(3,2,0)}}}}}},3423235);
        RobotStateMessage rs("robot0", Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), {{}, {Point(0, -1, 0.1), Point(0.3, 3.1, -1.2)}, {}}, 93249);
        CollisionNotificationMessage cn("h0", {"nose","neck"}, "r0", {"4","2"}, 32890592300, Interval<TimestampType>(72, 123), Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), 0.5);

        List<BodyPresentationMessage> bp_received;
        List<HumanStateMessage> hs_received;
        List<RobotStateMessage> rs_received;
        List<CollisionNotificationMessage> cn_received;

        auto bp_subscriber = _access.make_body_presentation_subscriber([&](auto p){ bp_received.push_back(p); OPERA_PRINT_TEST_COMMENT("Message received: " << Serialiser<BodyPresentationMessage>(p).to_string()) });
        auto hs_subscriber = _access.make_human_state_subscriber([&](auto p){ hs_received.push_back(p); OPERA_PRINT_TEST_COMMENT("Message received: " << Serialiser<HumanStateMessage>(p).to_string()) });
        auto rs_subscriber = _access.make_robot_state_subscriber([&](auto p){ rs_received.push_back(p); OPERA_PRINT_TEST_COMMENT("Message received: " << Serialiser<RobotStateMessage>(p).to_string()) });
        auto cn_subscriber = _access.make_collision_notification_subscriber([&](auto p){ cn_received.push_back(p); OPERA_PRINT_TEST_COMMENT("Message received: " << Serialiser<CollisionNotificationMessage>(p).to_string()) });
        auto bp_publisher = _access.make_body_presentation_publisher();
        auto hs_publisher = _access.make_human_state_publisher();
        auto rs_publisher = _access.make_robot_state_publisher();
        auto cn_publisher = _access.make_collision_notification_publisher();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bp_publisher->put(hp);
        bp_publisher->put(rp);
        hs_publisher->put(hs);
        rs_publisher->put(rs);
        cn_publisher->put(cn);
        SizeType i=0;
        while (bp_received.size() != 2 or hs_received.size() != 1 or rs_received.size() != 1 or cn_received.size() != 1) {
            ++i;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        OPERA_PRINT_TEST_COMMENT("Took " << i << " ms to acknowledge the reception")

        delete bp_subscriber;
        delete hs_subscriber;
        delete rs_subscriber;
        delete cn_subscriber;
        delete bp_publisher;
        delete hs_publisher;
        delete rs_publisher;
        delete cn_publisher;
    }
};
