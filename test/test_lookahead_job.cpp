/***************************************************************************
 *            test_lookahead_job.cpp
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

#include "lookahead_job.hpp"
#include "body_registry.hpp"
#include "message.hpp"

#include "test.hpp"

using namespace Opera;

class TestLookAheadJob {
  public:
    void test() {
        OPERA_TEST_CALL(test_lookaheadjobid())
        OPERA_TEST_CALL(test_lookaheadjobid_comparison())
        OPERA_TEST_CALL(test_lookaheadjobpath_removal())
        OPERA_TEST_CALL(test_lookaheadjob_create_basic())
        OPERA_TEST_CALL(test_lookaheadjob_create_with_path())
        OPERA_TEST_CALL(test_lookaheadjob_earliest_collision_index())
    }

    void test_lookaheadjobid() {
        LookAheadJobIdentifier id("h0",2,"r0",3);
        OPERA_TEST_PRINT(id)
        OPERA_TEST_EQUALS(id.human(), "h0")
        OPERA_TEST_EQUALS(id.human_segment(), 2u)
        OPERA_TEST_EQUALS(id.robot(), "r0")
        OPERA_TEST_EQUALS(id.robot_segment(), 3u)
    }

    void test_lookaheadjobid_comparison() {
        LookAheadJobIdentifier id1("h0",2,"r0",3);
        LookAheadJobIdentifier id2("a0",2,"r0",3);
        OPERA_TEST_ASSERT(id1 == id1)
        OPERA_TEST_ASSERT(not (id1 == id2))
        LookAheadJobIdentifier id3("h1",2,"r0",3);
        LookAheadJobIdentifier id4("h0",1,"r0",3);
        LookAheadJobIdentifier id5("h0",3,"r0",3);
        LookAheadJobIdentifier id6("h0",2,"a0",3);
        LookAheadJobIdentifier id7("h0",2,"s0",3);
        LookAheadJobIdentifier id8("h0",2,"r0",2);
        LookAheadJobIdentifier id9("h0",2,"r0",4);
        OPERA_TEST_ASSERT(not (id1 < id1))
        OPERA_TEST_ASSERT(not (id1 < id2))
        OPERA_TEST_ASSERT(id1 < id3)
        OPERA_TEST_ASSERT(not (id1 < id4))
        OPERA_TEST_ASSERT(id1 < id5)
        OPERA_TEST_ASSERT(not (id1 < id6))
        OPERA_TEST_ASSERT(id1 < id7)
        OPERA_TEST_ASSERT(not (id1 < id8))
        OPERA_TEST_ASSERT(id1 < id9)
    }

    void test_lookaheadjobpath_removal() {
        auto path1 = LookAheadJobPath()
                    .add(0,1)
                    .add(0, 3)
                    .add(1, 4);
        path1.remove_g_than(5);
        OPERA_TEST_EQUALS(path1.size(),3)
        path1.remove_g_than(3);
        OPERA_TEST_EQUALS(path1.size(),2)
        path1.remove_g_than(2);
        OPERA_TEST_EQUALS(path1.size(),1)

        auto path2 = LookAheadJobPath()
                    .add(0,3);
        path2.remove_g_than(2);
        OPERA_TEST_EQUALS(path2.size(),0)

        auto path4 = LookAheadJobPath()
                .add(1,3)
                .add(0,5);
        path4.remove_le_than(2);
        OPERA_TEST_EQUALS(path4.size(),2)
        path4.remove_g_than(2);
        OPERA_TEST_EQUAL(path4.size(),1)
        path4.remove_g_than(0);
        OPERA_TEST_EQUAL(path4.size(),0)

        auto path5 = LookAheadJobPath()
                .add(1,3)
                .add(0,5);
        path5.reduce_between(0, 6);
        OPERA_TEST_PRINT(path5)
        path5.reduce_between(1, 6);
        OPERA_TEST_EQUALS(path5.size(),2)
        OPERA_TEST_PRINT(path5)
        path5.reduce_between(2, 3);
        OPERA_TEST_EQUAL(path5.size(),0)
    }

    void test_lookaheadjob_create_basic() {
        TimestampType const initial_time = 349234;
        LookAheadJobIdentifier id("h0", 2u, "r0", 3u);
        Human h("h0", {{0, 1}}, {1.0});
        auto sample = h.segment(0).create_sample({{-0.5, 1.0, 1.25}},{{}});
        OPERA_TEST_PRINT(id)
        LookAheadJob job = DiscardLookAheadJob(id, initial_time, sample, ModeTrace().push_back({{"robot", "first"}}), LookAheadJobPath());
        OPERA_TEST_EQUALS(job.id(), id)
        OPERA_TEST_EQUALS(job.initial_time(), initial_time)
        OPERA_TEST_EQUALS(job.snapshot_time(), initial_time)
        OPERA_TEST_EQUALS(job.prediction_trace().size(), 1u)
        OPERA_TEST_EQUALS(job.path().size(), 0u)
    }

    void test_lookaheadjob_create_with_path() {
        TimestampType const initial_time = 349234;
        LookAheadJobPath path;
        path.add(3,1);
        LookAheadJobIdentifier id("h0", 2u, "r0", 3u);
        Human h("h0", {{0, 1}}, {1.0});
        auto sample = h.segment(0).create_sample({{-0.5, 1.0, 1.25}},{{}});
        LookAheadJob job = DiscardLookAheadJob(id, initial_time, sample, ModeTrace().push_back({{"robot", "first"}}), path);

        OPERA_TEST_PRINT(job)
    }

    void test_lookaheadjob_earliest_collision_index() {
        Human h0("h0", {{0,1}}, {0.1});
        Robot r0("r0", 1000, {{0, 1},{1, 2}}, {0.1,0.1});

        Mode contract({{"s", "contract"}});
        Mode endup({{"s", "endup"}});
        Mode kneedown({{"s", "kneedown"}});
        Mode fullright({{"s", "fullright"}});
        Mode expand({{"s", " xpand"}});

        RobotStateHistory h(r0);

        TimestampType time = 0;

        h.acquire(contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 0}}}, ++time);

        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 1}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 2}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 3}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 4}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{5, 0, 5}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{4, 0, 6}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{3, 0, 7}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{2, 0, 8}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{1, 0, 9}}}, ++time);
        h.acquire(endup, {{{0, 0, 0}}, {{0, 0, 5}}, {{0, 0, 10}}}, ++time);

        h.acquire(kneedown, {{{0, 0, 0}}, {{1, 0, 4}}, {{1, 0, 9}}}, ++time);
        h.acquire(kneedown, {{{0, 0, 0}}, {{2, 0, 3}}, {{2, 0, 8}}}, ++time);
        h.acquire(kneedown, {{{0, 0, 0}}, {{3, 0, 2}}, {{3, 0, 7}}}, ++time);
        h.acquire(kneedown, {{{0, 0, 0}}, {{4, 0, 1}}, {{4, 0, 6}}}, ++time);
        h.acquire(kneedown, {{{0, 0, 0}}, {{5, 0, 0}}, {{5, 0, 5}}}, ++time);

        h.acquire(fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{6, 0, 4}}}, ++time);
        h.acquire(fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{7, 0, 3}}}, ++time);
        h.acquire(fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{8, 0, 2}}}, ++time);
        h.acquire(fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{9, 0, 1}}}, ++time);
        h.acquire(fullright, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time);

        h.acquire(contract, {{{0, 0, 0}}, {{5, 0, 0}}, {{10, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{4, 0, 1}}, {{9, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{3, 0, 2}}, {{8, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{2, 0, 3}}, {{7, 0, 0}}}, ++time);
        h.acquire(contract, {{{0, 0, 0}}, {{1, 0, 4}}, {{6, 0, 0}}}, ++time);

        OPERA_TEST_EQUALS(h.snapshot_at(time).checked_sample_index(h.mode_at(time), time), 4)

        LookAheadJobIdentifier id(h0.id(),0,r0.id(),0);
        auto sample = h0.segment(0).create_sample({{2,0,1}},{{2,0,2}});

        LookAheadJob job = DiscardLookAheadJob(id, time, sample, ModeTrace().push_back(contract), LookAheadJobPath());
        OPERA_TEST_EQUALS(job.earliest_collision_index(h),-1)

        job = DiscardLookAheadJob(id, time, sample, ModeTrace().push_back(contract).push_back(endup).push_back(kneedown).push_back(fullright).push_back(contract), LookAheadJobPath());
        OPERA_TEST_EQUALS(job.earliest_collision_index(h),2)

        job = DiscardLookAheadJob(id, time, sample, ModeTrace().push_back(contract).push_back(endup).push_back(kneedown), LookAheadJobPath());
        OPERA_TEST_EQUALS(job.earliest_collision_index(h),2)
    }
};

int main() {
    TestLookAheadJob().test();
    return OPERA_TEST_FAILURES;
}
