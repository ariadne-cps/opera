/***************************************************************************
 *            test_runtime.cpp
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
#include "memory.hpp"
#include "mqtt.hpp"
#include "runtime.hpp"

using namespace Opera;

class TestRuntime {
  public:

    void test() {
        OPERA_TEST_CALL(test_discard_manual_singleplan())
        OPERA_TEST_CALL(test_capsulereuse_manual_singleplan())
        OPERA_TEST_CALL(test_discard_manual_multipleplans_differentsamples())
        OPERA_TEST_CALL(test_automatic_simple(DiscardLookAheadJobFactory()))
        OPERA_TEST_CALL(test_automatic_multipleplans_differentsamples(DiscardLookAheadJobFactory()))
        OPERA_TEST_CALL(test_automatic_simple(ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG)))
        OPERA_TEST_CALL(test_automatic_multipleplans_differentsamples(ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG)))
    }

    void test_discard_manual_singleplan() {
        BrokerAccess access = MemoryBrokerAccess();
        LookAheadJobFactory job_factory = DiscardLookAheadJobFactory();
        Runtime runtime(access,job_factory,0);

        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),0)
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        String rid = "r0";
        String hid = "h0";

        SynchronisedQueue<CollisionNotificationMessage> notifications;

        BodyPresentationMessage rp(rid, 1000, {{0, 1},{1, 2}}, {0.1,0.1});
        BodyPresentationMessage hp(hid,{{0,1}},{0.1});
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        auto cn_subscriber = access.make_collision_notification_subscriber([&](CollisionNotificationMessage const& msg){ notifications.enqueue(msg); });
        bp_publisher->put(rp);
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),1)
        OPERA_TEST_EQUALS(runtime.num_segment_pairs(),2)

        Mode contract({{"s", "contract"}});
        Mode endup({{"s", "endup"}});
        Mode kneedown({{"s", "kneedown"}});
        Mode fullright({{"s", "fullright"}});
        Mode expand({{"s", "xpand"}});

        TimestampType time = 0;
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 1}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 2}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 3}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 4}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{0, 0, 10}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{1, 0, 4}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{2, 0, 3}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{3, 0, 2}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{4, 0, 1}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{5, 0, 0}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{6, 0, 4}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{7, 0, 3}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{8, 0, 2}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{9, 0, 1}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        OPERA_PRINT_TEST_CASE_TITLE("Human colliding on one segment, not at the current mode")

        bs_publisher->put({hid,{{{0,1,5}},{{4,0,6}}},time-1});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),1)
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        bs_publisher->put({hid,{{{0,1,5}},{{4,0,6}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),0)
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),1)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),1)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(notifications.size(),1)
        notifications.reserve();
        auto msg = notifications.dequeue();
        OPERA_TEST_EQUALS(msg.collision_distance().lower(), msg.collision_distance().upper())
        OPERA_TEST_EQUALS(msg.collision_distance().lower(), 11)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        OPERA_PRINT_TEST_CASE_TITLE("Human not colliding on any segment")

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({hid,{{{5,1,0}},{{10,1,0}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),1)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),1)

        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        Mode newmode({{"s", "newmode"}});
        bs_publisher->put({rid,newmode, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({hid,{{{5,1,0}},{{10,1,0}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        delete bp_publisher;
        delete bs_publisher;
        delete cn_subscriber;
        MemoryBroker::instance().clear();
    }

    void test_capsulereuse_manual_singleplan() {
        BrokerAccess access = MemoryBrokerAccess();
        LookAheadJobFactory job_factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
        Runtime runtime(access,job_factory,0);

        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),0)
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        String rid = "r0";
        String hid = "h0";

        SynchronisedQueue<CollisionNotificationMessage> notifications;

        BodyPresentationMessage rp(rid, 1000, {{0, 1},{1, 2}}, {0.1,0.1});
        BodyPresentationMessage hp(hid,{{0,1}},{0.1});
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        auto cn_subscriber = access.make_collision_notification_subscriber([&](CollisionNotificationMessage const& msg){ notifications.enqueue(msg); });
        bp_publisher->put(rp);
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),1)

        Mode contract({{"s", "contract"}});
        Mode endup({{"s", "endup"}});
        Mode kneedown({{"s", "kneedown"}});
        Mode fullright({{"s", "fullright"}});
        Mode expand({{"s", "xpand"}});

        TimestampType time = 0;
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 1}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 2}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 3}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 4}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{0, 0, 10}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{1, 0, 4}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{2, 0, 3}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{3, 0, 2}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{4, 0, 1}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{5, 0, 0}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{6, 0, 4}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{7, 0, 3}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{8, 0, 2}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{9, 0, 1}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        OPERA_PRINT_TEST_CASE_TITLE("Human colliding on one segment, not at the current mode")

        bs_publisher->put({hid,{{{0,1,5}},{{4,0,6}}},time-1});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),1)
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        bs_publisher->put({hid,{{{0,1,5}},{{4,0,6}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_pending_human_robot_pairs(),0)
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),1)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),1)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(notifications.size(),1)
        notifications.reserve();
        auto msg = notifications.dequeue();
        OPERA_TEST_EQUALS(msg.collision_distance().lower(), msg.collision_distance().upper())
        OPERA_TEST_EQUALS(msg.collision_distance().lower(), 11)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        OPERA_PRINT_TEST_CASE_TITLE("Human updated with the same position")

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({hid,{{{0,1,5}},{{4,0,6}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),1)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),1)

        runtime.__test__process_one_working_job();

        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        Mode newmode({{"s", "newmode"}});
        bs_publisher->put({rid,newmode, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({hid,{{{5,1,0}},{{10,1,0}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        delete bp_publisher;
        delete bs_publisher;
        delete cn_subscriber;
        MemoryBroker::instance().clear();
    }

    void test_discard_manual_multipleplans_differentsamples() {
        BrokerAccess access = MemoryBrokerAccess();
        LookAheadJobFactory job_factory = DiscardLookAheadJobFactory();
        Runtime runtime(access,job_factory,0);

        String rid = "r0";
        String hid = "h0";

        SynchronisedQueue<CollisionNotificationMessage> notifications;

        BodyPresentationMessage rp(rid, 1000, {{0, 1},{1, 2}}, {0.1,0.1});
        BodyPresentationMessage hp(hid,{{0,1}},{0.1});
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        auto cn_subscriber = access.make_collision_notification_subscriber([&](CollisionNotificationMessage const& msg){ notifications.enqueue(msg); });
        bp_publisher->put(rp);
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        Mode contract({{"s", "contract"}});
        Mode endup({{"s", "endup"}});
        Mode kneedown({{"s", "kneedown"}});
        Mode fullright({{"s", "fullright"}});
        Mode xpand({{"s", "xpand"}});

        TimestampType time = 0;
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 1}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 2}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 3}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 4}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{0, 0, 10}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{1, 0, 4}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{2, 0, 3}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{3, 0, 2}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{4, 0, 1}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{5, 0, 0}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{6, 0, 4}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{7, 0, 3}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{8, 0, 2}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{9, 0, 1}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});

        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});

        OPERA_PRINT_TEST_CASE_TITLE("Human colliding on one segment, not at the current mode")

        bs_publisher->put({hid,{{{9,0,0}},{{9,0,1}}},time});
        OPERA_TEST_PRINT(time)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),4)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),4)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(notifications.size(),2)
        notifications.reserve();
        auto notification1 = notifications.dequeue();
        notifications.reserve();
        auto notification2 = notifications.dequeue();
        OPERA_TEST_EQUALS(notification1.collision_distance().lower(), 6)
        OPERA_TEST_EQUALS(notification1.collision_distance().upper(), 7)
        OPERA_TEST_EQUALS(notification2.collision_distance().lower(), 21)
        OPERA_TEST_EQUALS(notification2.collision_distance().upper(), 22)

        OPERA_PRINT_TEST_CASE_TITLE("Human not colliding on any segment")

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({hid,{{{5,1,0}},{{10,1,0}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),4)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),0)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),2)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),2)

        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();
        runtime.__test__process_one_working_job();

        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),4)

        delete bp_publisher;
        delete bs_publisher;
        delete cn_subscriber;
        MemoryBroker::instance().clear();
    }

    void test_automatic_simple(LookAheadJobFactory const& job_factory) {
        BrokerAccess access = MemoryBrokerAccess();
        Runtime runtime(access,job_factory,1);
        SynchronisedQueue<CollisionNotificationMessage> notifications;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        String rid = "r0";
        String hid = "h0";
        Mode mode({"phase", "waiting"});
        BodyPresentationMessage rp(rid,10,{{0,1},{1,2}},{1.0,0.5});
        BodyPresentationMessage hp(hid,{{0,1},{1,2}},{1.0,0.5});
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        auto cn_subscriber = access.make_collision_notification_subscriber([&](CollisionNotificationMessage const& msg){ notifications.enqueue(msg); });
        bp_publisher->put(rp);
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        BodyStateMessage rs(rid,mode,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3000);
        bs_publisher->put(rs);
        BodyStateMessage hs(hid,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3200);
        bs_publisher->put(hs);

        BodyStateMessage rs2(rid,Mode({"phase","running"}),{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3100);
        bs_publisher->put(rs2);
        BodyStateMessage rs3(rid,Mode({"phase","waiting"}),{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3200);
        bs_publisher->put(rs3);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        OPERA_TEST_EQUALS(notifications.size(),4)

        while (notifications.size() > 0) {
            notifications.reserve();
            auto msg = notifications.dequeue();
            LookAheadJobIdentifier id(msg.human_id(),msg.human_segment_id(),msg.robot_id(),msg.robot_segment_id());
            std::stringstream ss;
            ss << "Collision for " << id << " at " << msg.collision_distance() << " in mode " << msg.collision_mode();
            OPERA_TEST_PRINT(ss.str())
        }

        delete bp_publisher;
        delete bs_publisher;
        delete cn_subscriber;
        MemoryBroker::instance().clear();
    }

    void test_automatic_multipleplans_differentsamples(LookAheadJobFactory const& job_factory) {
        BrokerAccess access = MemoryBrokerAccess();
        Runtime runtime(access,job_factory,1);

        String rid = "r0";
        String hid = "h0";

        SynchronisedQueue<CollisionNotificationMessage> notifications;

        BodyPresentationMessage rp(rid, 1000, {{0, 1},{1, 2}}, {0.1,0.1});
        BodyPresentationMessage hp(hid,{{0,1}},{0.1});
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        auto cn_subscriber = access.make_collision_notification_subscriber([&](CollisionNotificationMessage const& msg){ notifications.enqueue(msg); });
        bp_publisher->put(rp);
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        Mode contract({{"s", "contract"}});
        Mode endup({{"s", "endup"}});
        Mode kneedown({{"s", "kneedown"}});
        Mode fullright({{"s", "fullright"}});
        Mode xpand({{"s", "xpand"}});

        TimestampType time = 0;
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 1}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 2}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 3}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 4}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{0, 0, 10}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{1, 0, 4}}, {{1, 0, 9}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{2, 0, 3}}, {{2, 0, 8}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{3, 0, 2}}, {{3, 0, 7}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{4, 0, 1}}, {{4, 0, 6}}}, ++time});
        bs_publisher->put({rid,kneedown, {{{0, 0, 0}}, {{5, 0, 0}}, {{5, 0, 5}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{6, 0, 4}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{7, 0, 3}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{8, 0, 2}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{9, 0, 1}}}, ++time});
        bs_publisher->put({rid,fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time});

        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,xpand, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});

        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time});
        bs_publisher->put({rid,contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time});

        OPERA_PRINT_TEST_CASE_TITLE("Human colliding on one segment, not at the current mode")

        bs_publisher->put({hid,{{{9,0,0}},{{9,0,1}}},time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        OPERA_TEST_EQUALS(notifications.size(),2)
        notifications.reserve();
        auto notification1 = notifications.dequeue();
        notifications.reserve();
        auto notification2 = notifications.dequeue();
        OPERA_TEST_EQUAL(notification1.collision_distance(),Interval<TimestampType>(6,7))
        OPERA_TEST_EQUAL(notification2.collision_distance(),Interval<TimestampType>(21,22))

        OPERA_PRINT_TEST_CASE_TITLE("Human not colliding on any segment")

        bs_publisher->put({hid,{{{5,1,0}},{{10,1,0}}},++time});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        OPERA_TEST_EQUALS(runtime.num_waiting_jobs(),0)
        OPERA_TEST_EQUALS(runtime.num_sleeping_jobs(),4)

        delete bp_publisher;
        delete bs_publisher;
        delete cn_subscriber;
        MemoryBroker::instance().clear();
    }
};

int main() {
    TestRuntime().test();
    return OPERA_TEST_FAILURES;
}
