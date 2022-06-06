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
        OPERA_TEST_CALL(test_human_robot_distance())
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
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fifth,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fifth,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fifth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fifth,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(first,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(fourth,{{Point(4,6,3)},{Point(5,4,4)}},ts); ts+= 100000000;
        history.acquire(second,{{Point(4,5,3)},{Point(4,4,4)}},ts); ts+= 100000000;
        history.acquire(third,{{Point(4,6,3)},{Point(4,4,4)}},ts);

        {
            auto timing_prediction = RobotPredictTiming(history.snapshot_at(ts+1), first);
            auto second_timing_prediction = RobotPredictTiming(history, first);
            OPERA_TEST_EQUALS(timing_prediction.nanoseconds_to_mode, 100000000000);
            OPERA_TEST_EQUAL(timing_prediction.nanoseconds_to_mode, second_timing_prediction.nanoseconds_to_mode);
            std::cout << timing_prediction << std::endl;
        }
        {
            Mode sixth = Mode({robot, "sixth"});
            auto third_timing_prediction = RobotPredictTiming(history.snapshot_at(ts+1), sixth);
            OPERA_ASSERT(third_timing_prediction.impossible_prediction_flag);
            std::cout << third_timing_prediction << std::endl << std::endl;
        }
    }

    void test_distance_0(float precision){
        // Standard test section (adjusting parameters)
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory robot_history(r);
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateHistory human_history(h);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0;

                                                        // 0
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(first,{{Point(2,2,2)},{Point(3,3,3)}},ts); ts+= 100000000;

                                                        // 10
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(4,4,4)},{Point(5,5,5)}},ts);
        robot_history.acquire(second,{{Point(6,6,6)},{Point(7,7,7)}},ts); ts+= 100000000;

                                                        // 20
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(8,8,8)},{Point(9,9,9)}},ts);
        robot_history.acquire(third,{{Point(10,10,10)},{Point(11,11,11)}},ts); ts+= 100000000;

                                                        // 30
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(12,12,12)},{Point(13,13,13)}},ts);
        robot_history.acquire(fourth,{{Point(0,0,0)},{Point(0,0,0)}},ts); ts+= 100000000;

                                                        // 40
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(15,15,15)},{Point(16,16,16)}},ts);
        robot_history.acquire(fifth,{{Point(17,17,17)},{Point(18,18,18)}},ts); ts+= 100000000;



        RobotStateHistorySnapshot robot_snapshot = robot_history.snapshot_at(ts);
        HumanRobotDistance hrd = HumanRobotDistance(human_history, robot_snapshot, 0, 0, 0, ts);

        // results checking section

        Interval<FloatType> theoric_result = Interval<FloatType>(0.232051, 26.2128);
        Interval<FloatType> first_result = hrd.get_min_max_distances();

        {
            OPERA_TEST_ASSERT(abs(theoric_result.lower() - first_result.lower()) < precision);
            OPERA_TEST_ASSERT(abs(theoric_result.upper() - first_result.upper()) < precision);
        }
    }

    void test_distance_1(float precision){
        // 1) umano e robot collidono ad un certo punto (restituisce distanza nulla)


        // Standard test section (adjusting parameters)
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory robot_history(r);
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateHistory human_history(h);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0;

                                                        // 0
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(first,{{Point(2,2,2)},{Point(3,3,3)}},ts); ts+= 100000000;

                                                        // 10
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(4,4,4)},{Point(5,5,5)}},ts);
        robot_history.acquire(second,{{Point(6,6,6)},{Point(7,7,7)}},ts); ts+= 100000000;

                                                        // 20
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(8,8,8)},{Point(9,9,9)}},ts);
        robot_history.acquire(third,{{Point(12,12,12)},{Point(14,14,14)}},ts); ts+= 100000000;

                                                        // 30
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(12,12,12)},{Point(13,13,13)}},ts);
        robot_history.acquire(fourth,{{Point(0,0,0)},{Point(0,0,0)}},ts); ts+= 100000000;

                                                        // 40
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(15,15,15)},{Point(16,16,16)}},ts);
        robot_history.acquire(fifth,{{Point(17,17,17)},{Point(18,18,18)}},ts); ts+= 100000000;



        RobotStateHistorySnapshot robot_snapshot = robot_history.snapshot_at(ts);
        HumanRobotDistance hrd = HumanRobotDistance(human_history, robot_snapshot, 0, 0, 0, ts);
        //std::cout << hrd << std::endl << precision;

        // results checking section



        Interval<FloatType> second_result = hrd.get_min_max_distances();
        /*std::cout << "theoric_result: " << theoric_result << std::endl;
        std::cout << "real_result: " << real_result << std::endl;
        std::cout << "upper diff: " << real_result.upper() - theoric_result.upper() << std::endl;
        */
        {
            OPERA_TEST_ASSERT(abs(0 - second_result.lower()) < precision);
        }

    }

    void test_distance_2(float precision){
        //2) umano e robot monotonicamente si allontanano (restituisce il primo valore nella sequenza)

        // Standard test section (adjusting parameters)
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory robot_history(r);
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateHistory human_history(h);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0;

                                                        // 0
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(first,{{Point(3,3,3)},{Point(4,4,4)}},ts); ts+= 100000000;

                                                        // 10
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(second,{{Point(6,6,6)},{Point(7,7,7)}},ts); ts+= 100000000;

                                                        // 20
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(third,{{Point(12,12,12)},{Point(14,14,14)}},ts); ts+= 100000000;

                                                        // 30
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(fourth,{{Point(15,15,15)},{Point(16,16,16)}},ts); ts+= 100000000;

                                                        // 40
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(0,0,0)},{Point(1,1,1)}},ts);
        robot_history.acquire(fifth,{{Point(17,17,17)},{Point(18,18,18)}},ts); ts+= 100000000;



        RobotStateHistorySnapshot robot_snapshot = robot_history.snapshot_at(ts);
        HumanRobotDistance hrd = HumanRobotDistance(human_history, robot_snapshot, 0, 0, 0, ts);

        // results checking section
        // 3.464102 - thickness = 3.464102 - (1.0 + 0.5) = 1.9641

        Interval<FloatType> theoric_result = Interval<FloatType>(1.9641, 22.7487);
        Interval<FloatType> third_result = hrd.get_min_max_distances();

        {
            OPERA_TEST_ASSERT(abs(theoric_result.lower() - third_result.lower()) < precision);
            OPERA_TEST_ASSERT(abs(theoric_result.upper() - third_result.upper()) < precision);
        }

    }

    void test_distance_3(float precision){
        // 3) umano e robot monotonicamente si avvicinano (restituisce l'ultimo valore nella sequenza)
        // Standard test section (adjusting parameters)
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory robot_history(r);
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateHistory human_history(h);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0;

                                                        // 0
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(100,100,100)},{Point(101,101,101)}},ts);
        robot_history.acquire(first,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 10
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(55,55,55)},{Point(56,56,56)}},ts);
        robot_history.acquire(second,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 20
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(40,40,40)},{Point(41,41,41)}},ts);
        robot_history.acquire(third,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 30
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(21,21,21)},{Point(22,22,22)}},ts);
        robot_history.acquire(fourth,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 40
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(15,15,15)},{Point(16,16,16)}},ts);
        robot_history.acquire(fifth,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;



        RobotStateHistorySnapshot robot_snapshot = robot_history.snapshot_at(ts);
        HumanRobotDistance hrd = HumanRobotDistance(human_history, robot_snapshot, 0, 0, 0, ts);
        // minimum = 24.248711 - 1,5 = 22,748711
        // maximum = 93.530744 - 1,5 = 92,030744

        // results checking section

        Interval<FloatType> theoric_result = Interval<FloatType>(22.748711, 92.030744);
        Interval<FloatType> fourth_result = hrd.get_min_max_distances();

        {
            OPERA_TEST_ASSERT(abs(theoric_result.lower() - fourth_result.lower()) < precision);
            OPERA_TEST_ASSERT(abs(theoric_result.upper() - fourth_result.upper()) < precision);
        }


    }

    void test_distance_4(float precision){
        // Standard test section (adjusting parameters)
        String robot("robot");
        Robot r("r0", 10, {{0,1}}, {1.0});
        RobotStateHistory robot_history(r);
        Human h("h0", {{3,2},{1,0}}, {0.5,1.0});
        HumanStateHistory human_history(h);

        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"}), fifth({robot, "fifth"});

        TimestampType ts = 0;

                                                        // 0
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(100,100,100)},{Point(101,101,101)}},ts);
        robot_history.acquire(first,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 10
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(55,55,55)},{Point(56,56,56)}},ts);
        robot_history.acquire(second,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 20
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(40,40,40)},{Point(41,41,41)}},ts);
        robot_history.acquire(third,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 30
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(21,21,21)},{Point(22,22,22)}},ts);
        robot_history.acquire(fourth,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;

                                                        // 40
        human_history.acquire({{Point(0,0,0)},{Point(4,4,4)},{Point(15,15,15)},{Point(16,16,16)}},ts);
        robot_history.acquire(fifth,{{Point(0,0,0)},{Point(1,1,1)}},ts); ts+= 100000000;



        RobotStateHistorySnapshot robot_snapshot = robot_history.snapshot_at(ts);
        HumanRobotDistance hrd = HumanRobotDistance(human_history, robot_snapshot, 0, 0, 0, ts);
        // minimum = 24.248711 - 1,5 = 22,748711
        // maximum = 93.530744 - 1,5 = 92,030744

        // results checking section

        Interval<FloatType> theoric_result = Interval<FloatType>(22.748711, 92.030744);
        Interval<FloatType> fourth_result = hrd.get_min_max_distances();

        {
            OPERA_TEST_ASSERT(abs(theoric_result.lower() - fourth_result.lower()) < precision);
            OPERA_TEST_ASSERT(abs(theoric_result.upper() - fourth_result.upper()) < precision);
        }
    }

    void test_human_robot_distance(){
        /*
1) umano e robot collidono ad un certo punto (restituisce distanza nulla)
2) umano e robot monotonicamente si allontanano (restituisce il primo valore nella sequenza)
3) umano e robot monotonicamente si avvicinano (restituisce l'ultimo valore nella sequenza)
4) umano e robot raggiungono un minimo locale di distanza, per poi raggiungere più avanti un minimo ancora più basso (ovvero il valore inferiore dell'intervallo è più basso)
5) umano e robot raggiungono un minimo locale di distanza, per poi raggiungere più avanti un altro minimo con valore inferiore dell'intervallo uguale, ma valore superiore maggiore (in quel caso ha senso che l'intervallo venga esteso)
        */
        float precision = 1e-4;
        //test_distance_0(precision);
        //test_distance_1(precision);
        //test_distance_2(precision);
        //test_distance_3(precision);
        test_distance_4(precision);

    }

};


int main() {
    TestState().test();

    return OPERA_TEST_FAILURES;
}
