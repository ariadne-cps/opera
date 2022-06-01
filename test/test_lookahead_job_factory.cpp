/***************************************************************************
 *            test_lookahead_job_factory.cpp
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

#include "lookahead_job_factory.hpp"
#include "body_registry.hpp"

#include "test.hpp"

using namespace Opera;

class TestLookAheadJobFactory {
  public:
    void test() {
        OPERA_TEST_CALL(test_discard_create_job())
        OPERA_TEST_CALL(test_discard_create_next_jobs())
        OPERA_TEST_CALL(test_discard_wake_job())
        OPERA_TEST_CALL(test_reuse_wake_job_without_barriersequencesection())
        OPERA_TEST_CALL(test_reuse_wake_job_with_barriersequencesection())
    }

    void test_discard_create_job() {
        LookAheadJobFactory factory = DiscardLookAheadJobFactory();

        Human h0("h0", {{"nose", "neck"}}, {1.0});
        auto s1 = h0.segment(0).create_sample();
        s1.update({Point(-0.5, 1.0, 1.25)},{});

        Robot r("r0", 1, {{"0","1"}}, {1.0});
        RobotStateHistory history(r);
        Mode original_mode({"phase","running"});
        Mode updated_mode({"phase","sleeping"});
        Mode final_mode({"phase","nothing"});

        TimestampType original_time = 1000;
        TimestampType updated_time = 2000;
        TimestampType final_time = 3000;

        history.acquire(original_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},original_time);
        history.acquire(updated_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},updated_time);
        history.acquire(final_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},final_time);

        LookAheadJobIdentifier id("h0",1,"r0",3);

        auto job = factory.create_new_job(id, original_time, s1, ModeTrace().push_back(original_mode), LookAheadJobPath());
        OPERA_TEST_EQUALS(job.id(),id)
        OPERA_TEST_EQUALS(job.initial_time(),original_time)
        OPERA_TEST_EQUALS(job.prediction_trace().ending_mode(),original_mode)

        auto s2 = h0.segment(0).create_sample();
        s2.update({{-0.5, 1.0, 1.25}},{{1.0,2.0,3.0}});

        auto woken = factory.awaken(job, updated_time, s2, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
        auto const& wj = woken.at(0).first;
        OPERA_TEST_EQUALS(wj.id(), id)
        OPERA_TEST_EQUALS(wj.initial_time(),updated_time)
        OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),updated_mode)
    }

    void test_discard_create_next_jobs() {

        Robot r0("r0",10,{{"0", "1"},{"1", "2"}}, {1.0,0.5});
        RobotStateHistory h(r0);

        LookAheadJobFactory factory = DiscardLookAheadJobFactory();

        Human h0("h0", {{"nose", "neck"}}, {1.0});
        auto human_sample = h0.segment(0).create_sample();
        human_sample.update({{-0.5, 1.0, 1.25}},{{1.0,2.0,3.0}});

        LookAheadJobIdentifier id(h0.id(),1,r0.id(),0);
        TimestampType time = 0;
        Mode initial_mode({"phase","running"});
        auto incorrect_job = factory.create_new_job(id, time, human_sample, ModeTrace().push_back(initial_mode), LookAheadJobPath());
        OPERA_TEST_FAIL(factory.create_next_jobs(incorrect_job, h))

        Mode contract({{"s", "contract"}});
        Mode endup({{"s", "endup"}});
        Mode kneedown({{"s", "kneedown"}});
        Mode fullright({{"s", "fullright"}});
        Mode expand({{"s", "xpand"}});

        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(10,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(4,0,1)}},{"2",{Point(9,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(3,0,2)}},{"2",{Point(8,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(2,0,3)}},{"2",{Point(7,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(1,0,4)}},{"2",{Point(6,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,0)}}}}, ++time);

        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,1)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,2)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,3)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,4)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,5)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(4,0,6)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(3,0,7)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(2,0,8)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(1,0,9)}}}}, ++time);
        h.acquire(endup, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(0,0,10)}}}}, ++time);

        h.acquire(kneedown, {{{"0",{Point(0,0,0)}},{"1",{Point(1,0,4)}},{"2",{Point(1,0,9)}}}}, ++time);
        h.acquire(kneedown, {{{"0",{Point(0,0,0)}},{"1",{Point(2,0,3)}},{"2",{Point(2,0,8)}}}}, ++time);
        h.acquire(kneedown, {{{"0",{Point(0,0,0)}},{"1",{Point(3,0,2)}},{"2",{Point(3,0,7)}}}}, ++time);
        h.acquire(kneedown, {{{"0",{Point(0,0,0)}},{"1",{Point(4,0,1)}},{"2",{Point(4,0,6)}}}}, ++time);
        h.acquire(kneedown, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(5,0,5)}}}}, ++time);

        h.acquire(fullright, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(6,0,4)}}}}, ++time);
        h.acquire(fullright, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(7,0,3)}}}}, ++time);
        h.acquire(fullright, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(8,0,2)}}}}, ++time);
        h.acquire(fullright, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(9,0,1)}}}}, ++time);
        h.acquire(fullright, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(10,0,0)}}}}, ++time);

        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(10,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(4,0,1)}},{"2",{Point(9,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(3,0,2)}},{"2",{Point(8,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(2,0,3)}},{"2",{Point(7,0,0)}}}}, ++time);

        auto job = factory.create_new_job(id, time, human_sample, ModeTrace().push_back(contract), LookAheadJobPath());
        auto next_jobs = factory.create_next_jobs(job, h);

        OPERA_TEST_EQUALS(next_jobs.size(),1)
        OPERA_TEST_PRINT(next_jobs)

        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(1,0,4)}},{"2",{Point(6,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,0)}}}}, ++time);

        h.acquire(expand, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,0)}}}}, ++time);
        h.acquire(expand, {{{"0",{Point(0,0,0)}},{"1",{Point(1,0,4)}},{"2",{Point(6,0,0)}}}}, ++time);
        h.acquire(expand, {{{"0",{Point(0,0,0)}},{"1",{Point(2,0,3)}},{"2",{Point(7,0,0)}}}}, ++time);
        h.acquire(expand, {{{"0",{Point(0,0,0)}},{"1",{Point(3,0,2)}},{"2",{Point(8,0,0)}}}}, ++time);
        h.acquire(expand, {{{"0",{Point(0,0,0)}},{"1",{Point(4,0,1)}},{"2",{Point(9,0,0)}}}}, ++time);
        h.acquire(expand, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(10,0,0)}}}}, ++time);

        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(5,0,0)}},{"2",{Point(10,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(4,0,1)}},{"2",{Point(9,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(3,0,2)}},{"2",{Point(8,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(2,0,3)}},{"2",{Point(7,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(1,0,4)}},{"2",{Point(6,0,0)}}}}, ++time);
        h.acquire(contract, {{{"0",{Point(0,0,0)}},{"1",{Point(0,0,5)}},{"2",{Point(5,0,0)}}}}, ++time);

        auto woken = factory.awaken(job, time, human_sample, h);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
        auto const& wj = woken.at(0).first;
        auto next_jobs2 = factory.create_next_jobs(wj, h);
        OPERA_TEST_EQUALS(next_jobs2.size(),2)
        OPERA_TEST_PRINT(next_jobs2)

        auto next_jobs3 = factory.create_next_jobs(next_jobs2.at(0), h);
        for (auto const& j : factory.create_next_jobs(next_jobs2.at(1), h))
            next_jobs3.emplace_back(j);
        OPERA_TEST_EQUALS(next_jobs3.size(),2)
        OPERA_TEST_PRINT(next_jobs3)

        auto next_jobs4 = factory.create_next_jobs(next_jobs3.at(0), h);
        for (auto const& j : factory.create_next_jobs(next_jobs3.at(1), h))
            next_jobs4.emplace_back(j);
        OPERA_TEST_EQUALS(next_jobs4.size(),1)
        OPERA_TEST_PRINT(next_jobs4)

        auto next_jobs5 = factory.create_next_jobs(next_jobs4.at(0), h);
        OPERA_TEST_EQUALS(next_jobs5.size(),1)
        OPERA_TEST_PRINT(next_jobs5)

        auto next_jobs6 = factory.create_next_jobs(next_jobs5.at(0), h);
        OPERA_TEST_EQUALS(next_jobs6.size(),0)
    }

    void test_discard_wake_job() {

        auto factory = DiscardLookAheadJobFactory();

        Robot r("r0", 1, {{"0","1"}}, {1.0});
        RobotStateHistory history(r);
        Mode original_mode({"phase","running"});
        Mode updated_mode({"phase","sleeping"});
        Mode final_mode({"phase","nothing"});

        Human h("h0", {{"nose", "neck"}}, {1.0});

        TimestampType original_time = 1000;
        TimestampType updated_time = 2000;
        TimestampType final_time = 3000;

        history.acquire(original_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},original_time);
        history.acquire(updated_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},updated_time);
        history.acquire(final_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},final_time);

        LookAheadJobIdentifier id("h0",0,"r0",3);
        auto sample = h.segment(0).create_sample({{-0.5, 1.0, 1.25}},{{1.0,2.0,3.0}});
        DiscardLookAheadJob job(id, original_time, sample, ModeTrace().push_back(original_mode), LookAheadJobPath());
        OPERA_TEST_EQUALS(job.id(),id)
        OPERA_TEST_EQUALS(job.initial_time(),original_time)
        OPERA_TEST_EQUALS(job.prediction_trace().ending_mode(),original_mode)
        auto woken = factory.awaken(job, updated_time, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
        auto const& wj = woken.at(0).first;
        OPERA_TEST_EQUALS(wj.id(), id)
        OPERA_TEST_EQUALS(wj.initial_time(),updated_time)
        OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),updated_mode)

        DiscardLookAheadJob job2(id, original_time, sample, ModeTrace().push_back(original_mode).push_back(updated_mode), LookAheadJobPath().add(0, 1));
        woken = factory.awaken(job2, updated_time, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)

        DiscardLookAheadJob job3(id, updated_time, sample, ModeTrace().push_back(updated_mode), LookAheadJobPath().add(0, 1));
        woken = factory.awaken(job3, updated_time, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::UNAFFECTED)

        DiscardLookAheadJob job4(id, original_time, sample, ModeTrace().push_back(original_mode).push_back(updated_mode), LookAheadJobPath().add(1, 1));
        woken = factory.awaken(job4, updated_time, sample, history);
        OPERA_TEST_EQUALS(woken.size(),0)

        auto sample2 = h.segment(0).create_sample({{-0.5, 1.0, 1.25}},{{}});
        DiscardLookAheadJob job5(id, updated_time, sample, ModeTrace().push_back(original_mode).push_back(updated_mode), LookAheadJobPath().add(0, 1));
        woken = factory.awaken(job5, final_time, sample2, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::UNCOMPUTABLE)
    }

    void test_reuse_wake_job_without_barriersequencesection() {

        Robot r("r0", 1, {{"0","1"}}, {1.0});
        RobotStateHistory history(r);
        Mode original_mode({"phase","running"});
        Mode updated_mode({"phase","sleeping"});
        Mode final_mode({"phase","nothing"});

        Human h("h0", {{"nose","neck"}}, {1.0});

        history.acquire(original_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},1000);
        history.acquire(updated_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},2000);
        history.acquire(final_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},3000);
        history.acquire(original_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},4000);
        history.acquire(updated_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},5000);
        history.acquire(final_mode,{{{"0",{Point(0,0,0)}},{"1",{Point(4,4,4)}}}},6000);

        LookAheadJobIdentifier id("h0",0,"r0",0);
        auto sample = h.segment(0).create_sample({{-0.5, 1.0, 1.25}},{{1.0,2.0,3.0}});
        ReuseLookAheadJob job(id, 4000, 4000, sample, ModeTrace().push_back(original_mode), LookAheadJobPath(),
                              MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy()));
        OPERA_TEST_EQUALS(job.id(),id)
        OPERA_TEST_EQUALS(job.initial_time(),4000)
        OPERA_TEST_EQUALS(job.prediction_trace().ending_mode(),original_mode)
        auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
        auto woken = factory.awaken(job, 5000, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
        auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
        OPERA_TEST_EQUALS(wj.id(),id)
        OPERA_TEST_EQUALS(wj.initial_time(),5000)
        OPERA_TEST_EQUALS(wj.snapshot_time(),5000)
        OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),updated_mode)

        auto factory2 = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
        ReuseLookAheadJob job2(id, 4000, 4000, sample, ModeTrace().push_back(original_mode).push_back(updated_mode), LookAheadJobPath().add(0, 1),
                               MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy()));
        woken = factory2.awaken(job2, 5000, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)

        auto factory3 = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
        ReuseLookAheadJob job3(id, 5000, 5000, sample, ModeTrace().push_back(updated_mode), LookAheadJobPath().add(0, 1),
                               MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy()));
        woken = factory3.awaken(job3, 5000, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::UNAFFECTED)

        auto factory4 = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
        ReuseLookAheadJob job4(id, 4000, 4000, sample, ModeTrace().push_back(original_mode).push_back(updated_mode), LookAheadJobPath().add(1, 1),
                               MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy()));
        woken = factory4.awaken(job4, 5000, sample, history);
        OPERA_TEST_EQUALS(woken.size(),1)

        auto factory5 = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
        auto sample2 = h.segment(0).create_sample({{-0.5, 1.0, 1.25}},{{}});
        ReuseLookAheadJob job5(id, 5000, 5000, sample, ModeTrace().push_back(original_mode).push_back(updated_mode), LookAheadJobPath().add(0, 1),
                               MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy()));
        woken = factory5.awaken(job5, 6000, sample2, history);
        OPERA_TEST_EQUALS(woken.size(),1)
        OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::UNCOMPUTABLE)
    }

    void test_reuse_wake_job_with_barriersequencesection() {

        Robot r("r0", 1000, {{"0", "1"}}, {0.25});
        RobotStateHistory history1(r);
        Mode one({"p","1"}), two({"p","2"}), three({"p","3"}), four({"p","4"}), five({"p","5"});

        Human h("h0", {{"nose", "neck"}}, {0.25});

        LookAheadJobIdentifier id("h0",0,"r0",0);

        TimestampType time = 0;

        history1.acquire(one,{{{"0",{Point(1,0,0)}},{"1",{Point(1,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(2,0,0)}},{"1",{Point(2,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(3,0,0)}},{"1",{Point(3,4,0)}}}},++time);
        history1.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history1.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);
        history1.acquire(four,{{{"0",{Point(6,0,0)}},{"1",{Point(6,4,0)}}}},++time);
        history1.acquire(four,{{{"0",{Point(7,0,0)}},{"1",{Point(7,4,0)}}}},++time);
        history1.acquire(five,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(1,0,0)}},{"1",{Point(1,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(2,0,0)}},{"1",{Point(2,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(3,0,0)}},{"1",{Point(3,4,0)}}}},++time);
        history1.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history1.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);
        history1.acquire(four,{{{"0",{Point(6,0,0)}},{"1",{Point(6,4,0)}}}},++time);
        history1.acquire(four,{{{"0",{Point(7,0,0)}},{"1",{Point(7,4,0)}}}},++time);
        history1.acquire(five,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(1,0,0)}},{"1",{Point(1,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(2,0,0)}},{"1",{Point(2,4,0)}}}},++time);
        history1.acquire(one,{{{"0",{Point(3,0,0)}},{"1",{Point(3,4,0)}}}},++time);
        history1.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history1.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);
        history1.acquire(four,{{{"0",{Point(6,0,0)}},{"1",{Point(6,4,0)}}}},++time);

        {
            OPERA_PRINT_TEST_CASE_TITLE("Empty barrier, updating to the same mode and sample")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            ReuseLookAheadJob job(id, 9, 9, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three).push_back(four), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 10, original_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.initial_time(),10)
            OPERA_TEST_EQUALS(wj.snapshot_time(),10)
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),1)
            OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),one)
            OPERA_TEST_ASSERT(wj.barrier_sequence().is_empty())
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Barrier up to collision within a mode, updating to the same mode and sample (WEAK)")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::WEAK);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(7.5,{{0,0}});
            barrier_sequence_section.add_barrier(6.5,{{0,1}});
            barrier_sequence_section.add_barrier(5.5,{{0,2}});
            barrier_sequence_section.add_barrier(4.5,{{1,0}});
            barrier_sequence_section.add_barrier(3.5,{{2,0}});
            barrier_sequence_section.add_barrier(2.5,{{3,0}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 9, 9, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three).push_back(four), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 10, original_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.initial_time(),10)
            OPERA_TEST_EQUALS(wj.snapshot_time(),9)
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),4)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 3)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().minimum_distance(), 2.5)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().range().maximum_sample_index(), 0)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Barrier up to collision within a mode, updating to the same mode and sample (STRONG)")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(7.5,{{0,0}});
            barrier_sequence_section.add_barrier(6.5,{{0,1}});
            barrier_sequence_section.add_barrier(5.5,{{0,2}});
            barrier_sequence_section.add_barrier(4.5,{{1,0}});
            barrier_sequence_section.add_barrier(3.5,{{2,0}});
            barrier_sequence_section.add_barrier(2.5,{{3,0}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 9, 9, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three).push_back(four), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 10, original_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.initial_time(),10)
            OPERA_TEST_EQUALS(wj.snapshot_time(),10)
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),4)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 3)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().minimum_distance(), 2.5)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().range().maximum_sample_index(), 0)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Barrier up to collision at the beginning of the ending mode, updating to the same mode and sample")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{5, 0, 0}},{{5,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(2.5,{{0,1}});
            barrier_sequence_section.add_barrier(1.5,{{0,2}});
            barrier_sequence_section.add_barrier(0.5,{{1,0}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 9, 9, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 10, original_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),3)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 1)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().minimum_distance(), 0.5)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().range().maximum_sample_index(), 0)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("O-loop from in-between a mode, updating to the same mode and sample")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(6.5,{{0,1}});
            barrier_sequence_section.add_barrier(5.5,{{0,2}});
            barrier_sequence_section.add_barrier(4.5,{{1,0}});
            barrier_sequence_section.add_barrier(3.5,{{2,0}});
            barrier_sequence_section.add_barrier(2.5,{{3,0}});
            barrier_sequence_section.add_barrier(1.5,TraceSampleRange({4,0}).add(0));
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 10, 10, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three).push_back(four).push_back(five).push_back(one), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 11, original_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),6)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 5)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().minimum_distance(), 1.5)
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_barrier().range().maximum_sample_index(), 0)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("O-loop from the beginning of a mode, updating to the next mode with same sample")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(4.5,TraceSampleRange({0,0}).add(2).add(0));
            barrier_sequence_section.add_barrier(3.5,{{3,0}});
            barrier_sequence_section.add_barrier(2.5,{{4,0}});
            barrier_sequence_section.add_barrier(1.5,{{4,1}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 16, 16, original_sample, ModeTrace().push_back(five).push_back(one).push_back(two).push_back(three).push_back(four), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 17, original_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),5)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 3)
            OPERA_TEST_EQUALS(wj.prediction_trace().starting_mode(),one)
            OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),five)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("O-loop from the beginning of a mode, updating to the next mode with sample that imposes a reuse earlier")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(4.5,TraceSampleRange({0,0}).add(2).add(0));
            barrier_sequence_section.add_barrier(3.5,{{3,0}});
            barrier_sequence_section.add_barrier(2.5,{{4,0}});
            barrier_sequence_section.add_barrier(1.5,{{4,1}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            auto updated_sample = h.segment(0).create_sample({{6,0,0}},{{6,4,0}});
            ReuseLookAheadJob job(id, 16, 16, original_sample, ModeTrace().push_back(five).push_back(one).push_back(two).push_back(three).push_back(four), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 17, updated_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),4)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 2)
            OPERA_TEST_EQUALS(wj.prediction_trace().starting_mode(),one)
            OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),four)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Barrier such that updating with a large time interval empties it, since new mode is after the last barrier")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(4.5,TraceSampleRange({0,0}).add(2).add(0));
            barrier_sequence_section.add_barrier(3.5,{{3,0}});
            barrier_sequence_section.add_barrier(2.5,{{4,0}});
            barrier_sequence_section.add_barrier(1.5,{{4,1}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 16, 16, original_sample, ModeTrace().push_back(five).push_back(one).push_back(two).push_back(three).push_back(four), LookAheadJobPath(), barrier_sequence);
            auto updated_sample = h.segment(0).create_sample({{5,0,0}},{{5,4,0}});
            auto woken = factory.awaken(job, 22, updated_sample, history1);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.initial_time(),22)
            OPERA_TEST_EQUALS(wj.snapshot_time(),22)
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),1)
            OPERA_TEST_ASSERT(wj.barrier_sequence().is_empty())
        }

        time = 0;
        RobotStateHistory history2(r);

        history2.acquire(one,{{{"0",{Point(1,0,0)}},{"1",{Point(1,4,0)}}}},++time);
        history2.acquire(one,{{{"0",{Point(2,0,0)}},{"1",{Point(2,4,0)}}}},++time);
        history2.acquire(one,{{{"0",{Point(3,0,0)}},{"1",{Point(3,4,0)}}}},++time);
        history2.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history2.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);
        history2.acquire(four,{{{"0",{Point(6,0,0)}},{"1",{Point(6,4,0)}}}},++time);
        history2.acquire(four,{{{"0",{Point(7,0,0)}},{"1",{Point(7,4,0)}}}},++time);
        history2.acquire(five,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history2.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history2.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);
        history2.acquire(one,{{{"0",{Point(1,0,0)}},{"1",{Point(1,4,0)}}}},++time);
        history2.acquire(one,{{{"0",{Point(2,0,0)}},{"1",{Point(2,4,0)}}}},++time);
        history2.acquire(one,{{{"0",{Point(3,0,0)}},{"1",{Point(3,4,0)}}}},++time);
        history2.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history2.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);
        history2.acquire(four,{{{"0",{Point(6,0,0)}},{"1",{Point(6,4,0)}}}},++time);
        history2.acquire(four,{{{"0",{Point(7,0,0)}},{"1",{Point(7,4,0)}}}},++time);
        history2.acquire(five,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history2.acquire(two,{{{"0",{Point(4,0,0)}},{"1",{Point(4,4,0)}}}},++time);
        history2.acquire(three,{{{"0",{Point(5,0,0)}},{"1",{Point(5,4,0)}}}},++time);

        {
            OPERA_PRINT_TEST_CASE_TITLE("P-loop from within a mode, updating to the same mode and sample")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(6.5,{{0,1}});
            barrier_sequence_section.add_barrier(5.5,{{0,2}});
            barrier_sequence_section.add_barrier(4.5,{{1,0}});
            barrier_sequence_section.add_barrier(3.5,{{2,0}});
            barrier_sequence_section.add_barrier(2.5,{{3,0}});
            barrier_sequence_section.add_barrier(1.5,TraceSampleRange({3,1}).add(0));
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 12, 12, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three).push_back(four).push_back(five).push_back(two), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 13, original_sample, history2);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),6)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 4)
            OPERA_TEST_EQUALS(wj.prediction_trace().starting_mode(),one)
            OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),two)
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("P-loop from the beginning of a mode, updating to the same mode and sample")
            auto factory = ReuseLookAheadJobFactory(KeepOneMinimumDistanceBarrierSequenceUpdatePolicy(),ReuseEquivalence::STRONG);
            auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
            CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
            barrier_sequence_section.add_barrier(7.5,{{0,0}});
            barrier_sequence_section.add_barrier(6.5,{{0,1}});
            barrier_sequence_section.add_barrier(5.5,{{0,2}});
            barrier_sequence_section.add_barrier(4.5,{{1,0}});
            barrier_sequence_section.add_barrier(3.5,{{2,0}});
            barrier_sequence_section.add_barrier(2.5,{{3,0}});
            barrier_sequence_section.add_barrier(1.5,{{4,0}});
            auto barrier_sequence = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
            barrier_sequence.add(barrier_sequence_section);
            ReuseLookAheadJob job(id, 11, 11, original_sample, ModeTrace().push_back(one).push_back(two).push_back(three).push_back(four).push_back(five), LookAheadJobPath(), barrier_sequence);
            auto woken = factory.awaken(job, 12, original_sample, history2);
            OPERA_TEST_EQUALS(woken.size(),1)
            OPERA_TEST_ASSERT(woken.at(0).second == JobAwakeningResult::DIFFERENT)
            auto const& wj = dynamic_cast<const ReuseLookAheadJob&>(*woken.at(0).first.ptr());
            OPERA_TEST_EQUALS(wj.prediction_trace().size(),6)
            OPERA_TEST_ASSERT(not wj.barrier_sequence().is_empty())
            OPERA_TEST_EQUALS(wj.barrier_sequence().last_upper_trace_index(), 4)
            OPERA_TEST_EQUALS(wj.prediction_trace().starting_mode(),one)
            OPERA_TEST_EQUALS(wj.prediction_trace().ending_mode(),two)
        }
    }
};

int main() {
    TestLookAheadJobFactory().test();
    return OPERA_TEST_FAILURES;
}
