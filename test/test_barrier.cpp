/***************************************************************************
 *            test_barrier.cpp
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

#include <cmath>
#include "barrier.hpp"

using namespace Opera;

class TestBarrier {
  public:
    void test() {
        OPERA_TEST_CALL(test_barrier_sequence_section_create())
        OPERA_TEST_CALL(test_barrier_sequence_section_add_remove())
        OPERA_TEST_CALL(test_barrier_sequence_section_populate())
        OPERA_TEST_CALL(test_barrier_sequence_section_reuse_element())
        OPERA_TEST_CALL(test_sphere_barrier_sequence_section_reset())
        OPERA_TEST_CALL(test_capsule_barrier_sequence_section_reset_without_modes())
        OPERA_TEST_CALL(test_capsule_barrier_sequence_section_reset_with_modes())
        OPERA_TEST_CALL(test_barrier_sequence_section_reset_from_gtz())
        OPERA_TEST_CALL(test_capsule_barrier_sequence_single_section())
        OPERA_TEST_CALL(test_capsule_barrier_sequence_multiple_sections())
    }

    void test_barrier_sequence_section_create() {
        Human h("h0", {{"nose", "neck"},{"left_shoulder", "right_shoulder"}}, {1.0,1.0});
        auto hs = h.segment(1).create_sample({Point(0,0,0)},{Point(2,2,2)});
        auto section = SphereMinimumDistanceBarrierSequenceSectionFactory().create(hs);
        OPERA_TEST_PRINT(section)
        OPERA_TEST_EQUALS(section.size(),0)
        OPERA_TEST_ASSERT(section.is_empty())
        OPERA_TEST_ASSERT(not section.reaches_collision())
        OPERA_TEST_ASSERT(std::isinf(section.current_minimum_distance()))

        auto section_copy = SphereMinimumDistanceBarrierSequenceSectionFactory().copy(section);
        OPERA_TEST_EQUALS(section.human_sample(),section_copy.human_sample())

        MinimumDistanceBarrierSequenceSection section_handle = section;
        OPERA_TEST_PRINT(section_handle)
    }

    void test_barrier_sequence_section_add_remove() {
        Human h("h0", {{"nose", "neck"}}, {1.0});
        auto hs = h.segment(0).create_sample({Point(0,0,0)},{Point(2,2,2)});
        PositiveFloatType distance = 0.5;
        auto section = SphereMinimumDistanceBarrierSequenceSection(hs);
        section.add_barrier(distance,{{0,0}});
        OPERA_TEST_ASSERT(not section.is_empty())
        auto barrier = section.barrier(0);
        OPERA_TEST_ASSERT(barrier.minimum_distance() == distance)
        OPERA_TEST_EQUALS(barrier.range().maximum_trace_index(),0)
        OPERA_TEST_EQUALS(barrier.range().maximum_sample_index(),0)
        OPERA_TEST_PRINT(barrier)
        section.remove_first_barrier();
        OPERA_TEST_ASSERT(section.is_empty())
        OPERA_TEST_FAIL(section.remove_first_barrier())
        section.add_barrier(distance,{{0,0}});
        OPERA_TEST_ASSERT(not section.is_empty())
        section.add_barrier(0.4, {{0,1}});
        OPERA_TEST_EQUALS(section.size(),2)
        OPERA_TEST_EQUALS(section.barrier(1).range().maximum_sample_index(),1)
        section.clear();
        OPERA_TEST_ASSERT(section.is_empty())
    }

    void test_barrier_sequence_section_populate() {
        Robot r("r0", 10, {{"0", "1"}}, {1.0});
        Human h("h0", {{"nose", "neck"}}, {1.0});

        auto hs = h.segment(0).create_sample({Point(1,3,0)},{Point(2,3,0)});

        auto section1 = CapsuleMinimumDistanceBarrierSequenceSection(hs);
        List<BodySegmentSample> robot_samples;

        robot_samples.push_back(r.segment(0).create_sample({Point(-3,7,0)},{Point(-2,7,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,6,0)},{Point(-1,6,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-1,5,0)},{Point(0,5,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(0,4,0)},{Point(1,4,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(1,3,0)},{Point(2,3,0)}));
        SizeType idx = 0;
        for (auto const& s : robot_samples) if (not section1.check_and_update(s, {0, idx++})) break;
        OPERA_TEST_ASSERT(not section1.check_and_update(robot_samples.at(3), {0, idx}))
        OPERA_TEST_EQUALS(section1.size(),4)
        OPERA_TEST_ASSERT(section1.reaches_collision())
        OPERA_TEST_EQUALS(section1.last_barrier().range().maximum_sample_index(),3)
        OPERA_TEST_PRINT(section1)

        OPERA_TEST_ASSERT(not section1.are_colliding(hs,robot_samples.at(0)))
        OPERA_TEST_ASSERT(section1.are_colliding(hs,robot_samples.at(4)))

        auto section2 = CapsuleMinimumDistanceBarrierSequenceSection(hs);
        robot_samples.clear();
        robot_samples.push_back(r.segment(0).create_sample({Point(-3,7,0)},{Point(-2,7,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,6,0)},{Point(-1,6,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-3,6,0)},{Point(-2,6,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,5,0)},{Point(-1,5,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,4,0)},{Point(-1,4,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(1,3,0)},{Point(2,3,0)}));
        idx = 0;
        for (auto const& s : robot_samples) if (not section2.check_and_update(s, {0, idx++})) break;
        OPERA_TEST_EQUALS(section2.size(),5)
        OPERA_TEST_EQUALS(section2.last_barrier().range().maximum_sample_index(),5)
        OPERA_TEST_PRINT(section2)

        auto section3 = CapsuleMinimumDistanceBarrierSequenceSection(hs);
        robot_samples.clear();
        robot_samples.push_back(r.segment(0).create_sample({Point(-3,7,0)},{Point(-2,7,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,6,0)},{Point(-1,6,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-1,5,0)},{Point(0,5,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,5,0)},{Point(-1,5,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-3,5,0)},{Point(0,5,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-2,5,0)},{Point(-1,5,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(-3,5,0)},{Point(-2,5,0)}));
        idx = 0;
        for (auto const& s : robot_samples) if (not section3.check_and_update(s, {0, idx++})) break;
        OPERA_TEST_EQUALS(section3.size(),3)
        OPERA_TEST_EQUALS(section3.last_barrier().range().maximum_sample_index(),6)
        OPERA_TEST_PRINT(section3)
    }

    void test_barrier_sequence_section_reuse_element() {
        Robot r("r0", 10, {{"0", "1"}}, {1.0});
        Human h("h0", {{"nose", "neck"}}, {1.0});

        auto hs1 = h.segment(0).create_sample({Point(4,5,0)},{Point(5,5,0)});

        Mode first({r.id(), "first"});
        Mode second({r.id(), "second"});
        Mode third({r.id(), "third"});
        auto section = CapsuleMinimumDistanceBarrierSequenceSection(hs1);
        RobotStateHistory history(r);

        history.acquire(first,{{{"0",{Point(-3,7,0)}},{"1",{Point(-2,7,0)}}}},0);
        history.acquire(first,{{{"0",{Point(-2,6,0)}},{"1",{Point(-1,6,0)}}}},100);
        history.acquire(first,{{{"0",{Point(-1,5,0)}},{"1",{Point(0,5,0)}}}},200);
        history.acquire(first,{{{"0",{Point(-2,6,0)}},{"1",{Point(0,5,0)}}}},300);
        history.acquire(first,{{{"0",{Point(-1,5,0)}},{"1",{Point(0,5,0)}}}},400);
        history.acquire(first,{{{"0",{Point(0,4,0)}},{"1",{Point(1,4,0)}}}},500);
        history.acquire(first,{{{"0",{Point(0,4,0)}},{"1",{Point(1,4,0)}}}},600);
        history.acquire(first,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},700);
        history.acquire(first,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},800);
        history.acquire(second,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},900);
        history.acquire(third,{{{"0",{Point(2,3,0)}},{"1",{Point(3,3,0)}}}},1000);
        history.acquire(first,{{{"0",{Point(-3,7,0)}},{"1",{Point(-2,7,0)}}}},1100);

        SizeType idx = 0;
        auto snapshot = history.snapshot_at(1100);
        for (auto const& s : snapshot.samples(first).at(0)) if (not section.check_and_update(s, {0, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(second).at(0)) if (not section.check_and_update(s, {1, idx++})) break;
        OPERA_TEST_PRINT(section)

        auto hs2 = h.segment(0).create_sample({Point(5, 5, 0)}, {Point(5, 5, 0)});
        auto element = section._reuse_element(hs2);
        OPERA_TEST_EQUALS(element,(int)section.size()-1)

        auto hs3 = h.segment(0).create_sample({Point(1, 3, 0)}, {Point(2, 3, 0)});
        element = section._reuse_element(hs3);
        OPERA_TEST_PRINT(element)
        OPERA_TEST_ASSERT(element < (int)section.size()-1)

        auto hs4 = h.segment(0).create_sample({Point(10, 10, 0)}, {Point(10, 10, 0)});
        element = section._reuse_element(hs4);
        OPERA_TEST_EQUALS(element,-1)
    }

    void test_sphere_barrier_sequence_section_reset() {
        Robot r("r0", 10, {{"0", "1"}}, {1.0});
        Human h("h0", {{"nose", "neck"}}, {1.0});

        auto hs1 = h.segment(0).create_sample({Point(4,5,0)},{Point(5,5,0)});

        Mode first({r.id(), "first"});
        Mode second({r.id(), "second"});
        Mode third({r.id(), "third"});
        Mode fourth({r.id(), "fourth"});
        auto section1 = SphereMinimumDistanceBarrierSequenceSection(hs1);
        RobotStateHistory history(r);
        history.acquire(first,{{{"0",{Point(-3,7,0)}},{"1",{Point(-2,7,0)}}}},0);
        history.acquire(first,{{{"0",{Point(-2,6,0)}},{"1",{Point(-1,6,0)}}}},100);
        history.acquire(first,{{{"0",{Point(-1,5,0)}},{"1",{Point(0,5,0)}}}},200);
        history.acquire(first,{{{"0",{Point(-2,6,0)}},{"1",{Point(0,5,0)}}}},300);
        history.acquire(first,{{{"0",{Point(-1,5,0)}},{"1",{Point(0,5,0)}}}},400);
        history.acquire(first,{{{"0",{Point(0,4,0)}},{"1",{Point(1,4,0)}}}},500);
        history.acquire(first,{{{"0",{Point(0,4,0)}},{"1",{Point(1,4,0)}}}},600);
        history.acquire(first,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},700);
        history.acquire(first,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},800);
        history.acquire(second,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},900);
        history.acquire(third,{{{"0",{Point(2,3,0)}},{"1",{Point(3,3,0)}}}},1000);
        history.acquire(fourth,{{{"0",{Point(3,2,0)}},{"1",{Point(4,2,0)}}}},1100);
        history.acquire(first,{{{"0",{Point(-3,7,0)}},{"1",{Point(-2,7,0)}}}},1200);
        SizeType idx = 0;
        auto snapshot = history.snapshot_at(1200);
        for (auto const& s : snapshot.samples(first).at(0)) if (not section1.check_and_update(s, {0, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(second).at(0)) if (not section1.check_and_update(s, {1, idx++})) break;

        auto section2 = section1;
        auto section3 = section1;

        auto hs1_new = h.segment(0).create_sample({Point(5,5,0)},{Point(5,5,0)});
        OPERA_TEST_PRINT(section1)
        section1.reset(hs1_new,{0,5},0);
        OPERA_TEST_PRINT(section1)
        OPERA_TEST_EQUALS(section1.last_barrier().range().initial().trace,0)
        OPERA_TEST_EQUALS(section1.last_barrier().range().maximum_trace_index(),1)
        OPERA_TEST_EQUALS(section1.last_barrier().range().maximum_sample_index(),0)

        auto hs2_new = h.segment(0).create_sample({Point(1,3,0)},{Point(2,3,0)});
        section2.reset(hs2_new,{0,5},0);
        OPERA_TEST_PRINT(section2)
        OPERA_TEST_EQUALS(section2.last_upper_trace_index(),0)

        auto hs3_new = h.segment(0).create_sample({Point(10,10,0)},{Point(10,10,0)});
        section3.reset(hs3_new,{0,5},0);
        OPERA_TEST_ASSERT(section3.is_empty())
    }

    void test_capsule_barrier_sequence_section_reset_without_modes() {
        Robot r("r0", 10, {{"0", "1"}}, {0.5});
        Human h("h0", {{"nose", "neck"}}, {0.5});

        auto hs = h.segment(0).create_sample({Point(0,0,0)},{Point(1,0,0)});

        List<BodySegmentSample> robot_samples;
        robot_samples.push_back(r.segment(0).create_sample({Point(3,2,0)},{Point(4,2,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(3,1,0)},{Point(4,1,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(3,0,0)},{Point(4,0,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(3,-1,0)},{Point(4,-1,0)}));
        robot_samples.push_back(r.segment(0).create_sample({Point(3,-2,0)},{Point(4,-2,0)}));

        auto section1 = CapsuleMinimumDistanceBarrierSequenceSection(hs);
        SizeType idx = 0;
        for (auto const& s : robot_samples) if (not section1.check_and_update(s, {0, idx++})) break;
        OPERA_TEST_PRINT(section1)
        OPERA_TEST_EQUALS(section1.size(),3)

        auto section2 = section1;
        auto section3 = section1;
        auto section4 = section1;
        auto section5 = section1;
        auto section6 = section1;

        auto hs1 = h.segment(0).create_sample({Point(1,0,0)},{Point(2,0,0)});
        section1.reset(hs1,{0,5},0);
        OPERA_TEST_PRINT(section1)
        OPERA_TEST_EQUALS(section1.last_barrier().range().maximum_sample_index(),1)

        auto hs2 = h.segment(0).create_sample({Point(2,2,0)},{Point(3,2,0)});
        section2.reset(hs2,{0,5},0);
        OPERA_TEST_ASSERT(section2.is_empty())

        auto hs3 = h.segment(0).create_sample({Point(-3,0,0)},{Point(-2,0,0)});
        section3.reset(hs3,{0,5},0);
        OPERA_TEST_ASSERT(section3.is_empty())

        section4.reset(hs,{0,5},0);
        OPERA_TEST_EQUALS(section4.last_barrier().range().maximum_sample_index(),4)

        section5.reset(hs1,{1,5},0);
        OPERA_TEST_ASSERT(section5.is_empty())

        section6.reset(hs1,{0,5},5);
        OPERA_TEST_ASSERT(section6.is_empty())
    }

    void test_capsule_barrier_sequence_section_reset_with_modes() {
        Robot r("r0", 10, {{"0", "1"}}, {1.0});
        Human h("h0", {{"nose", "neck"}}, {1.0});

        auto hs1 = h.segment(0).create_sample({Point(4,5,0)},{Point(5,5,0)});

        Mode first({r.id(), "first"});
        Mode second({r.id(), "second"});
        Mode third({r.id(), "third"});
        Mode fourth({r.id(), "fourth"});
        auto section1 = CapsuleMinimumDistanceBarrierSequenceSection(hs1);
        RobotStateHistory history(r);
        history.acquire(first,{{{"0",{Point(-3,7,0)}},{"1",{Point(-2,7,0)}}}},0);
        history.acquire(first,{{{"0",{Point(-2,6,0)}},{"1",{Point(-1,6,0)}}}},100);
        history.acquire(first,{{{"0",{Point(-1,5,0)}},{"1",{Point(0,5,0)}}}},200);
        history.acquire(first,{{{"0",{Point(-2,6,0)}},{"1",{Point(0,5,0)}}}},300);
        history.acquire(first,{{{"0",{Point(-1,5,0)}},{"1",{Point(0,5,0)}}}},400);
        history.acquire(first,{{{"0",{Point(0,4,0)}},{"1",{Point(1,4,0)}}}},500);
        history.acquire(first,{{{"0",{Point(0,4,0)}},{"1",{Point(1,4,0)}}}},600);
        history.acquire(first,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},700);
        history.acquire(first,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},800);
        history.acquire(second,{{{"0",{Point(1,3,0)}},{"1",{Point(2,3,0)}}}},900);
        history.acquire(third,{{{"0",{Point(2,3,0)}},{"1",{Point(3,3,0)}}}},1000);
        history.acquire(fourth,{{{"0",{Point(3,2,0)}},{"1",{Point(4,2,0)}}}},1100);
        history.acquire(first,{{{"0",{Point(-3,7,0)}},{"1",{Point(-2,7,0)}}}},1200);
        SizeType idx = 0;
        auto snapshot = history.snapshot_at(1200);
        for (auto const& s : snapshot.samples(first).at(0)) if (not section1.check_and_update(s, {0, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(second).at(0)) if (not section1.check_and_update(s, {1, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(third).at(0)) if (not section1.check_and_update(s, {2, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(fourth).at(0)) if (not section1.check_and_update(s, {3, idx++})) break;

        auto section2 = section1;
        auto section3 = section1;
        auto section4 = section1;

        auto hs1_new = h.segment(0).create_sample({Point(5,5,0)},{Point(5,5,0)});
        OPERA_TEST_PRINT(section1)
        section1.reset(hs1_new,{0,7},0);
        OPERA_TEST_PRINT(section1)
        OPERA_TEST_EQUALS(section1.last_barrier().range().maximum_trace_index(),3)
        OPERA_TEST_EQUALS(section1.last_barrier().range().maximum_sample_index(),0)
        OPERA_TEST_EQUALS(section4.size(),6)

        auto hs2_new = h.segment(0).create_sample({Point(1,3,0)},{Point(2,3,0)});
        section2.reset(hs2_new,{0,7},0);
        OPERA_TEST_PRINT(section2)
        OPERA_TEST_EQUALS(section2.last_barrier().range().maximum_trace_index(),0)

        auto hs3_new = h.segment(0).create_sample({Point(10,10,0)},{Point(10,10,0)});
        section3.reset(hs3_new,{0,7},0);
        OPERA_TEST_ASSERT(section3.is_empty())

        section4.reset(hs1_new,{0,7},4);
        OPERA_TEST_PRINT(section4)
        OPERA_TEST_EQUALS(section4.size(),4)
    }

    void test_barrier_sequence_section_reset_from_gtz() {
        Human h("h0", {{"nose", "neck"}}, {1.0});
        auto original_sample = h.segment(0).create_sample({{9, 0, 0}},{{9,4,0}});
        CapsuleMinimumDistanceBarrierSequenceSection barrier_sequence_section(original_sample);
        barrier_sequence_section.add_barrier(7.5,{{3,0}});
        barrier_sequence_section.add_barrier(6.5,{{3,1}});
        barrier_sequence_section.reset(original_sample,{2,2},0);
        OPERA_TEST_ASSERT(barrier_sequence_section.is_empty())
    }

    void test_capsule_barrier_sequence_single_section() {
        Robot r("r0", 10, {{"0", "1"}}, {0.25});
        Human h("h0", {{"nose", "neck"}}, {0.25});

        Mode first({r.id(), "first"});
        Mode second({r.id(), "second"});
        Mode third({r.id(), "third"});
        Mode fourth({r.id(), "fourth"});
        auto sequence1 = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),KeepOneMinimumDistanceBarrierSequenceUpdatePolicy());
        RobotStateHistory history(r);
        history.acquire(first,{{{"0",{Point(-3,0,0)}},{"1",{Point(-2,0,0)}}}},0);
        history.acquire(first,{{{"0",{Point(-2,0,0)}},{"1",{Point(-1,0,0)}}}},100);
        history.acquire(first,{{{"0",{Point(-1,0,0)}},{"1",{Point(0,0,0)}}}},200);
        history.acquire(first,{{{"0",{Point(-2,0,0)}},{"1",{Point(-1,0,0)}}}},300);
        history.acquire(first,{{{"0",{Point(-1,0,0)}},{"1",{Point(0,0,0)}}}},400);
        history.acquire(first,{{{"0",{Point(0,0,0)}},{"1",{Point(1,0,0)}}}},500);
        history.acquire(first,{{{"0",{Point(0,0,0)}},{"1",{Point(1,0,0)}}}},600);
        history.acquire(first,{{{"0",{Point(1,0,0)}},{"1",{Point(2,0,0)}}}},700);
        history.acquire(first,{{{"0",{Point(1,0,0)}},{"1",{Point(2,0,0)}}}},800);
        history.acquire(second,{{{"0",{Point(1,0,0)}},{"1",{Point(2,0,0)}}}},900);
        history.acquire(third,{{{"0",{Point(2,0,0)}},{"1",{Point(3,0,0)}}}},1000);
        history.acquire(fourth,{{{"0",{Point(3,0,0)}},{"1",{Point(4,0,0)}}}},1100);
        history.acquire(first,{{{"0",{Point(-3,0,0)}},{"1",{Point(-2,0,0)}}}},1200);

        auto hs1 = h.segment(0).create_sample({Point(8,0,0)},{Point(9,0,0)});
        auto hs2 = h.segment(0).create_sample({Point(7,0,0)},{Point(8,0,0)});
        auto hs3 = h.segment(0).create_sample({Point(6,0,0)},{Point(7,0,0)});
        auto hs4 = h.segment(0).create_sample({Point(5,0,0)},{Point(6,0,0)});

        SizeType idx = 0;
        auto snapshot = history.snapshot_at(1200);
        for (auto const& s : snapshot.samples(first).at(0)) if (not sequence1.check_and_update(hs1, s, {0, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(second).at(0)) if (not sequence1.check_and_update(hs2, s, {1, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(third).at(0)) if (not sequence1.check_and_update(hs3, s, {2, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(fourth).at(0)) if (not sequence1.check_and_update(hs4, s, {3, idx++})) break;

        OPERA_TEST_PRINT(sequence1)
        OPERA_TEST_EQUALS(sequence1.num_sections(),1)
        OPERA_TEST_EQUALS(sequence1.num_barriers(),7)

        auto const& last_section = sequence1.last_section();
        OPERA_TEST_EQUALS(last_section.size(),7)

        auto hs5 = h.segment(0).create_sample({Point(4.6,0,0)},{Point(5.6,0,0)});
        auto hs6 = h.segment(0).create_sample({Point(4,0,0)},{Point(5,0,0)});
        auto hs7 = h.segment(0).create_sample({Point(-2.5,0,0)},{Point(-1.5,0,0)});
        auto hs8 = h.segment(0).create_sample({Point(18,0,0)},{Point(19,0,0)});

        auto sequence5 = sequence1;
        auto sequence6 = sequence1;
        auto sequence7 = sequence1;
        auto sequence8 = sequence1;
        auto sequence9 = sequence1;

        sequence5.reset(hs5,{0,7},0);
        OPERA_TEST_PRINT(sequence5)
        OPERA_TEST_EQUALS(sequence5.num_barriers(),7)

        sequence7.reset(hs7,{0,7},0);
        OPERA_TEST_PRINT(sequence7)

        sequence8.reset(hs8,{0,7},0);
        OPERA_TEST_PRINT(sequence8)

        OPERA_TEST_EQUALS(sequence1.num_barriers(),7)

        sequence1 = sequence8;
        OPERA_TEST_ASSERT(sequence1.is_empty())

        sequence9.reset(hs5,{0,2},0);
        OPERA_TEST_EQUALS(sequence9.last_barrier().range().maximum_trace_index(),2)
        OPERA_TEST_PRINT(sequence9)

        sequence9.reset(hs5,{0,0},0);
        OPERA_TEST_EQUALS(sequence9.num_barriers(),5)
        OPERA_TEST_PRINT(sequence9)
    }

    void test_capsule_barrier_sequence_multiple_sections() {
        Robot r("r0", 10, {{"0", "1"}}, {0.25});
        Human h("h0", {{"nose", "neck"}}, {0.25});

        Mode first({r.id(), "first"});
        Mode second({r.id(), "second"});
        Mode third({r.id(), "third"});
        Mode fourth({r.id(), "fourth"});
        auto sequence1 = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),AddWhenNecessaryMinimumDistanceBarrierSequenceUpdatePolicy());
        RobotStateHistory history(r);
        history.acquire(first,{{{"0",{Point(-3,0,0)}},{"1",{Point(-2,0,0)}}}},0);
        history.acquire(first,{{{"0",{Point(-2,0,0)}},{"1",{Point(-1,0,0)}}}},100);
        history.acquire(first,{{{"0",{Point(-1,0,0)}},{"1",{Point(0,0,0)}}}},200);
        history.acquire(first,{{{"0",{Point(-2,0,0)}},{"1",{Point(-1,0,0)}}}},300);
        history.acquire(first,{{{"0",{Point(-1,0,0)}},{"1",{Point(0,0,0)}}}},400);
        history.acquire(first,{{{"0",{Point(0,0,0)}},{"1",{Point(1,0,0)}}}},500);
        history.acquire(first,{{{"0",{Point(0,0,0)}},{"1",{Point(1,0,0)}}}},600);
        history.acquire(first,{{{"0",{Point(1,0,0)}},{"1",{Point(2,0,0)}}}},700);
        history.acquire(first,{{{"0",{Point(1,0,0)}},{"1",{Point(2,0,0)}}}},800);
        history.acquire(second,{{{"0",{Point(1,0,0)}},{"1",{Point(2,0,0)}}}},900);
        history.acquire(third,{{{"0",{Point(2,0,0)}},{"1",{Point(3,0,0)}}}},1000);
        history.acquire(fourth,{{{"0",{Point(3,0,0)}},{"1",{Point(4,0,0)}}}},1100);
        history.acquire(first,{{{"0",{Point(-3,0,0)}},{"1",{Point(-2,0,0)}}}},1200);

        auto hs1 = h.segment(0).create_sample({Point(3,0,0)},{Point(4,0,0)});
        auto hs2 = h.segment(0).create_sample({Point(4,0,0)},{Point(5,0,0)});
        auto hs3 = h.segment(0).create_sample({Point(2,0,0)},{Point(3,0,0)});
        auto hs4 = h.segment(0).create_sample({Point(1,0,0)},{Point(2,0,0)});

        SizeType idx = 0;
        auto snapshot = history.snapshot_at(1200);
        for (auto const& s : snapshot.samples(first).at(0)) if (not sequence1.check_and_update(hs1, s, {0, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(second).at(0)) if (not sequence1.check_and_update(hs2, s, {1, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(third).at(0)) if (not sequence1.check_and_update(hs3, s, {2, idx++})) break;
        idx = 0;
        for (auto const& s : snapshot.samples(fourth).at(0)) if (not sequence1.check_and_update(hs4, s, {3, idx++})) break;

        OPERA_TEST_PRINT(sequence1)
        OPERA_TEST_EQUALS(sequence1.num_sections(),2)
        OPERA_TEST_EQUALS(sequence1.num_barriers(),6)

        auto hs5 = h.segment(0).create_sample({Point(2,0,0)},{Point(3,0,0)});
        auto hs6 = h.segment(0).create_sample({Point(-2.5,0,0)},{Point(-1.5,0,0)});

        auto sequence5 = sequence1;
        auto sequence6 = MinimumDistanceBarrierSequence(CapsuleMinimumDistanceBarrierSequenceSectionFactory(),AddWhenNecessaryMinimumDistanceBarrierSequenceUpdatePolicy());
        sequence6 = sequence1;

        sequence5.reset(hs5,{0,7},0);
        OPERA_TEST_PRINT(sequence5)
        OPERA_TEST_EQUALS(sequence5.num_sections(),1)

        sequence6.reset(hs6,{0,7},0);
        OPERA_TEST_ASSERT(sequence6.is_empty())
    }
};

int main() {
    TestBarrier().test();
    return OPERA_TEST_FAILURES;
}
