/***************************************************************************
 *            test_state.cpp
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

#include "state.hpp"

using namespace Opera;

class TestState {
public:
    void test() {
        OPERA_TEST_CALL(test_human_state_instance())
        OPERA_TEST_CALL(test_human_state_history())
        OPERA_TEST_CALL(test_robot_state_history_basics())
        OPERA_TEST_CALL(test_robot_state_history_analytics())
        OPERA_TEST_CALL(test_robot_state_history_can_look_ahead())
        OPERA_TEST_CALL(test_robot_predict_timing())
    }

    void test_human_state_instance() {
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateInstance instance(h,{{Point(0,0,0)},{Point(4,4,4)},{Point(0,2,0)},{Point(1,0,3)}},500000000);

        OPERA_TEST_EQUALS(instance.samples().size(),2)
        OPERA_TEST_EQUALS(instance.timestamp(),500000000)
    }

    void test_human_state_history() {
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateHistory history(h);

        OPERA_TEST_EQUALS(history.size(),0)
        OPERA_TEST_FAIL(history.instance_distance(1000000000,4000000000))
        history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,2,0)},{Point(1,0,3)}},1000000000);
        OPERA_TEST_EQUALS(history.size(),1)
        OPERA_TEST_EXECUTE(history.latest_within(1000000001))
        OPERA_TEST_EXECUTE(history.latest_within(1000000000))
        OPERA_TEST_FAIL(history.latest_within(999999999))
        OPERA_TEST_EQUALS(history.instance_number(1000000000),0)
        OPERA_TEST_FAIL(history.instance_number(1000000001))
        history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,2,0)},{Point(1,0,3)}},2000000000);
        history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,2,0)},{Point(1,0,3)}},3000000000);
        OPERA_TEST_EQUALS(history.size(),3)
        OPERA_TEST_FAIL(history.instance_distance(1000000000,4000000000))
        OPERA_TEST_FAIL(history.instance_distance(10000000,3000000000))
        OPERA_TEST_EQUALS(history.instance_distance(2000000000,2000000000),0)
        OPERA_TEST_EQUALS(history.instance_distance(2000000000,3000000000),1)
        OPERA_TEST_EQUALS(history.instance_distance(1000000000,2000000000),1)
        OPERA_TEST_EQUALS(history.instance_distance(1000000000,3000000000),2)
        OPERA_TEST_EQUALS(history.instance_number(2000000000),1)
        OPERA_TEST_EQUALS(history.instance_number(3000000000),2)
    }

    void test_robot_state_history_basics() {
        String robot("robot");
        Robot r("r0", 10, {{3, 2},{1, 0}}, {1.0, 0.5});
        RobotStateHistory history(r);
        Mode empty_mode, first({robot, "first"}), second({robot, "second"});

        {
            auto snapshot = history.snapshot_at(0);
            OPERA_TEST_EQUALS(snapshot.mode_trace().size(),0)
            OPERA_TEST_ASSERT(snapshot.presences_in(empty_mode).empty())
            OPERA_TEST_ASSERT(snapshot.presences_exiting_into(empty_mode).empty())
            OPERA_TEST_FAIL(snapshot.samples(empty_mode))
            OPERA_TEST_ASSERT(snapshot.modes_with_samples().empty())
        }

        history.acquire(first,{{Point(0,0,0)},{Point(4,4,4)},{Point( 0,2,0)},{Point(1,0,3)}},500000000);

        {
            auto snapshot = history.snapshot_at(500000000);
            OPERA_TEST_FAIL(snapshot.samples(first))
            OPERA_TEST_ASSERT(snapshot.modes_with_samples().empty())
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(500000000))
            OPERA_TEST_EQUALS(snapshot.mode_trace().size(),0)
            auto entrances = snapshot.presences_exiting_into(first);
            OPERA_TEST_EQUAL(entrances.size(),1)
            OPERA_TEST_ASSERT(entrances.back().mode().is_empty())
            OPERA_TEST_EQUALS(entrances.back().to(),500000000)
        }

        history.acquire(first,{{Point(0,0,1)},{Point(4,4,5)},{Point(0,3,0)},{Point(1,1,3)}},600000000);
        history.acquire(second,{{Point(0,0,1.5)},{Point(4,4,5.5)},{Point(0,3.5,0)},{Point(1,1.5,3)}},700000000);

        {
            auto snapshot = history.snapshot_at(700000000);
            OPERA_TEST_EQUALS(snapshot.mode_trace().ending_mode(),first)
            OPERA_TEST_EQUALS(snapshot.modes_with_samples().size(), 1)
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(700000000))
            OPERA_TEST_EQUALS(snapshot.presences_in(first).size(), 1)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(second).size(), 1)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(second).back().mode(), first)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(second).back().from(), 500000000)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(second).back().to(), 700000000)
            OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(first), Interval<SizeType>(2u))
            OPERA_TEST_EQUALS(snapshot.samples(first).at(0).at(0).error(),0)
        }

        history.acquire(first,{{Point(0,0,2),Point(0,0.1,2)},{Point(4,4,6)},{Point(0,4,0)},{Point(1,2,3),Point(1.1,2,3)}},800000000);

        {
            auto snapshot = history.snapshot_at(800000000);
            OPERA_TEST_EQUALS(snapshot.mode_trace().ending_mode(), second)
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(500000000))
            OPERA_TEST_ASSERT(snapshot.can_look_ahead(800000000))
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(800000001))
            OPERA_TEST_EQUALS(snapshot.samples(first).size(), 2)
            OPERA_TEST_PRINT(snapshot.samples(first))
            OPERA_TEST_EQUALS(snapshot.samples(first).at(0).at(0).error(), 0)
            OPERA_TEST_PRINT(snapshot.presences_exiting_into(first))
            OPERA_TEST_EQUALS(snapshot.presences_in(second).size(), 1)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(first).size(), 2)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(first).back().mode(), second)
            OPERA_TEST_EQUALS(snapshot.samples(first).at(0).size(), 2)
            OPERA_TEST_EQUALS(snapshot.samples(second).at(0).size(), 1)
            OPERA_TEST_PRINT(snapshot.samples(second))
        }

        history.acquire(first,{{Point(1,0,2)},{Point(5,4,6)},{Point(1,4,0)},{Point(2,2,3)}},1100000000);
        history.acquire(second,{{Point(1,0,1.5)},{Point(5,4,5.5)},{Point(1,3.5,0)},{Point(2,1.5,3)}},1200000000);

        {
            auto snapshot = history.snapshot_at(1200000000);
            OPERA_TEST_EQUALS(snapshot.samples(first).at(0).size(), 4)
            OPERA_TEST_EQUALS(snapshot.samples(first).at(0).at(1).error(),snapshot.samples(first).at(0).at(2).error())
            OPERA_TEST_EQUALS(snapshot.presences_in(first).size(), 2)
            OPERA_TEST_EQUALS(snapshot.presences_exiting_into(second).size(), 2)
            OPERA_TEST_PRINT(snapshot.samples(first))
            OPERA_TEST_ASSERT(snapshot.samples(first).at(0).at(0).error() > 0)
            auto modes = snapshot.modes_with_samples();
            OPERA_TEST_EQUALS(modes.size(), 2)
            auto history_trace = snapshot.mode_trace();
            OPERA_TEST_PRINT(history_trace)
            auto expected_trace = ModeTrace().push_back(first, 1.0).push_back(second, 1.0).push_back(first, 1.0);
            OPERA_TEST_EQUALS(history_trace, expected_trace)
        }
    }

    void test_robot_state_history_analytics() {
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory history(r);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0u;
        history.acquire(first,{{Point(0,0,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(1,0,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,1,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,2,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,3,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(1,3,1)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(1,3,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,4,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,5,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(2,5,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(3,5,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(4,5,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;

        auto snapshot = history.snapshot_at(ts);

        OPERA_TEST_EQUALS(snapshot.mode_trace().ending_mode(),second)

        OPERA_TEST_EQUALS(snapshot.presences_in(first).size(), 2)
        OPERA_TEST_EQUALS(snapshot.presences_in(second).size(), 3)
        OPERA_TEST_EQUALS(snapshot.presences_in(third).size(), 2)
        OPERA_TEST_EQUALS(snapshot.presences_in(fourth).size(), 0)
        OPERA_TEST_EQUALS(snapshot.presences_in(fifth).size(), 0)

        OPERA_TEST_EQUALS(snapshot.presences_exiting_into(first).size(),2)
        OPERA_TEST_EQUALS(snapshot.presences_exiting_into(second).size(),3)
        OPERA_TEST_EQUALS(snapshot.presences_exiting_into(third).size(),2)
        OPERA_TEST_EQUALS(snapshot.presences_exiting_into(fourth).size(),1)
        OPERA_TEST_EQUALS(snapshot.presences_exiting_into(fifth).size(),0)

        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(first), Interval<SizeType>(2u, 3u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(second), Interval<SizeType>(1u, 3u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(third), Interval<SizeType>(1u, 2u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(fourth), Interval<SizeType>(0u, 0u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(fifth), Interval<SizeType>(0u, 0u))

        OPERA_TEST_EQUALS(snapshot.presences_between(first,third).size(),1)
        OPERA_TEST_EQUALS(snapshot.presences_between(first,second).size(),1)
        OPERA_TEST_EQUALS(snapshot.presences_between(second,third).size(),1)
        OPERA_TEST_EQUALS(snapshot.presences_between(third,second).size(),2)
        OPERA_TEST_EQUALS(snapshot.presences_between(third,first).size(),0)
        OPERA_TEST_EQUALS(snapshot.presences_between(second,fourth).size(),1)

        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(third, first), Interval<SizeType>(0u, 0u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(first, second), Interval<SizeType>(2u, 2u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(first, third), Interval<SizeType>(3u, 3u))
        OPERA_TEST_EQUALS(snapshot.range_of_num_samples_in(third, second), Interval<SizeType>(1u, 2u))
    }

    void test_robot_state_history_can_look_ahead() {
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory history(r);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0u;
        history.acquire(first,{{Point(0,0,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(1,0,0)},{Point(4,4,4)}},ts);

        {
            auto snapshot = history.snapshot_at(ts);
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(ts))
        }

        ts+= 100000000;
        history.acquire(second,{{Point(1,1,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,2,0)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,3,0)},{Point(4,4,4)}},ts);

        {
            auto snapshot = history.snapshot_at(ts);
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(ts))
        }

        ts+= 100000000;
        history.acquire(third,{{Point(1,3,1)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(1,3,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(1,4,2)},{Point(4,4,4)}},ts);

        {
            auto snapshot = history.snapshot_at(ts);
            OPERA_TEST_ASSERT(snapshot.can_look_ahead(ts))
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(ts-600000000))
        }

        ts+= 100000000;
        history.acquire(second,{{Point(1,5,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(2,5,2)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(3,5,2)},{Point(4,4,4)}},ts);

        {
            auto snapshot = history.snapshot_at(ts);
            OPERA_TEST_ASSERT(snapshot.can_look_ahead(ts))
        }

        ts+= 100000000;
        history.acquire(first,{{Point(4,5,2)},{Point(4,4,4)}},ts);

        {
            auto snapshot = history.snapshot_at(ts);
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(ts))
        }

        ts+= 100000000;
        history.acquire(third,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts);

        {
            auto snapshot = history.snapshot_at(ts);
            OPERA_TEST_ASSERT(not snapshot.can_look_ahead(ts))
        }
    }

    // #~#v

    void test_robot_predict_timing(){
        String robot("robot");
        Robot r("r0", 10, {{3, 2},{1, 0}}, {1.0, 0.5});
        RobotStateHistory history(r);
        Mode empty_mode, first({robot, "first"}), second({robot, "second"});

        history.acquire(first,{{Point(0,0,0)},{Point(4,4,4)},{Point( 0,2,0)},{Point(1,0,3)}},500000000);
        history.acquire(first,{{Point(0,0,1)},{Point(4,4,5)},{Point(0,3,0)},{Point(1,1,3)}},600000000);
        history.acquire(second,{{Point(0,0,1.5)},{Point(4,4,5.5)},{Point(0,3.5,0)},{Point(1,1.5,3)}},700000000);
        history.acquire(first,{{Point(0,0,2),Point(0,0.1,2)},{Point(4,4,6)},{Point(0,4,0)},{Point(1,2,3),Point(1.1,2,3)}},800000000);
        history.acquire(first,{{Point(1,0,2)},{Point(5,4,6)},{Point(1,4,0)},{Point(2,2,3)}},1100000000);
        history.acquire(second,{{Point(1,0,1.5)},{Point(5,4,5.5)},{Point(1,3.5,0)},{Point(2,1.5,3)}},1200000000);

        {
            OPERA_TEST_EQUALS(history.most_recent_occurrence(first), 1100000000);
            //OPERA_TEST_EQUALS(history.most_recent_occurrence(first, 500000000), 1100000000);
            //OPERA_TEST_EQUALS(history.most_recent_occurrence(first, 600000000), 500000000);

            OPERA_TEST_EQUALS(history.most_recent_occurrence(second), 700000000);
            OPERA_TEST_EQUALS(history.most_recent_occurrence(second, 700000000), 1200000000);
        }
    }

    // #~#^

};


int main() {
    TestState().test();

    return OPERA_TEST_FAILURES;
}
