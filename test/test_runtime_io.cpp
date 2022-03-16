/***************************************************************************
 *            test_runtime_io.cpp
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
#include "runtime_io.hpp"

using namespace Opera;

class TestRuntimeIO {
  public:

    void test() {
        OPERA_TEST_CALL(test_sender())
        OPERA_TEST_CALL(test_receiver_human())
        OPERA_TEST_CALL(test_receiver_robot())
        OPERA_TEST_CALL(test_receiver_both())
    }

    void test_sender() {
        BrokerAccess access = MemoryBrokerAccess();
        RuntimeSender sender(access);
        Mode mode({"phase","running"});
        CollisionNotificationMessage msg("h0", 0, "r0", 3, 32890592300, Interval<TimestampType>(72, 123), mode, 0.5);
        sender.put(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(MemoryBroker::instance().size<CollisionNotificationMessage>(),1)
        MemoryBroker::instance().clear();
    }

    void test_receiver_human() {
        BrokerAccess access = MemoryBrokerAccess();
        LookAheadJobFactory job_factory = DiscardLookAheadJobFactory();
        BodyRegistry registry;
        SynchronisedQueue<LookAheadJob> waiting_jobs, sleeping_jobs;
        RuntimeReceiver receiver(access, job_factory, registry, waiting_jobs, sleeping_jobs);
        String id = "h0";
        BodyPresentationMessage hp(id,{{0,1},{1,2}},{1.0,0.5});
        BodyStateMessage hs(id,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},300000000);
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        bs_publisher->put(hs);
        OPERA_TEST_ASSERT(not registry.contains(id))
        OPERA_TEST_FAIL(registry.latest_human_instance_within(id,0))
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_ASSERT(registry.contains(id))
        bs_publisher->put(hs);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto const& last_state = registry.latest_human_instance_within(id,300000000);
        OPERA_TEST_EQUALS(last_state.timestamp(),300000000)
        delete bp_publisher;
        delete bs_publisher;
        MemoryBroker::instance().clear();
    }

    void test_receiver_robot() {
        BrokerAccess access = MemoryBrokerAccess();
        LookAheadJobFactory job_factory = DiscardLookAheadJobFactory();
        BodyRegistry registry;
        SynchronisedQueue<LookAheadJob> waiting_jobs, sleeping_jobs;
        RuntimeReceiver receiver(access, job_factory, registry, waiting_jobs, sleeping_jobs);
        String id = "r0";
        Mode mode({"phase", "waiting"});
        BodyPresentationMessage rp(id,10,{{0,1},{1,2}},{1.0,0.5});
        BodyStateMessage rs(id,mode,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},300000000);
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        bs_publisher->put(rs);
        OPERA_TEST_ASSERT(not registry.contains(id))
        OPERA_TEST_FAIL(registry.robot_history(id))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bp_publisher->put(rp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_ASSERT(registry.contains(id))
        bs_publisher->put(rs);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(receiver.num_pending_human_robot_pairs(),0)
        delete bp_publisher;
        delete bs_publisher;
        MemoryBroker::instance().clear();
    }

    void test_receiver_both() {
        BrokerAccess access = MemoryBrokerAccess();
        LookAheadJobFactory job_factory = DiscardLookAheadJobFactory();
        BodyRegistry registry;
        SynchronisedQueue<LookAheadJob> waiting_jobs, sleeping_jobs;
        RuntimeReceiver receiver(access, job_factory, registry, waiting_jobs, sleeping_jobs);
        String rid = "r0";
        String hid = "h0";
        Mode waiting({"phase", "waiting"}), running({"phase","running"});
        BodyPresentationMessage rp(rid,10,{{0,1},{1,2}},{1.0,0.5});
        BodyStateMessage rs(rid, waiting, {{Point(0, 0, 0)}, {Point(0, 2, 0)}, {Point(0, 4, 0)}}, 3000000000);
        BodyPresentationMessage hp(hid,{{0,1},{1,2}},{1.0,0.5});
        BodyStateMessage hs(hid,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3200000000);
        auto bp_publisher = access.make_body_presentation_publisher();
        auto bs_publisher = access.make_body_state_publisher();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bp_publisher->put(rp);
        bp_publisher->put(hp);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(receiver.num_pending_human_robot_pairs(),1)
        bs_publisher->put(rs);
        bs_publisher->put(hs);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(waiting_jobs.size(),0)
        OPERA_TEST_EQUALS(sleeping_jobs.size(),0)
        OPERA_TEST_EQUALS(receiver.num_pending_human_robot_pairs(),1)

        BodyStateMessage rs2(rid,running,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3100000000);
        bs_publisher->put(rs2);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        OPERA_TEST_EQUALS(registry.robot_history(rid).snapshot_at(3100000000).modes_with_samples().size(), 1)

        BodyStateMessage rs3(rid,waiting,{{Point(0,0,0)},{Point(0,2,0)},{Point(0,4,0)}},3200000000);
        bs_publisher->put(rs3);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        OPERA_TEST_EQUALS(waiting_jobs.size(),4)

        delete bp_publisher;
        delete bs_publisher;
        MemoryBroker::instance().clear();
    }
};

int main() {
    TestRuntimeIO().test();
    return OPERA_TEST_FAILURES;
}
