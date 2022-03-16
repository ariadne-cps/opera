/***************************************************************************
 *            test_mode.cpp
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

#include "mode.hpp"

#include "test.hpp"

using namespace Opera;

class TestMode {
  public:
    void test() {
        OPERA_TEST_CALL(test_mode_construction())
        OPERA_TEST_CALL(test_mode_comparison())
        OPERA_TEST_CALL(test_mode_trace_creation())
        OPERA_TEST_CALL(test_mode_trace_compare())
        OPERA_TEST_CALL(test_mode_trace_has_looped())
        OPERA_TEST_CALL(test_mode_trace_merge())
        OPERA_TEST_CALL(test_mode_trace_indexes())
        OPERA_TEST_CALL(test_mode_trace_reduce_between())
        OPERA_TEST_CALL(test_mode_trace_next_modes())
    }

    void test_mode_construction() {
        Mode state1;
        OPERA_TEST_ASSERT(state1.is_empty())
        OPERA_TEST_PRINT(state1)
        Mode state2({"robot", "first"});
        OPERA_TEST_ASSERT(not state2.is_empty())
        OPERA_TEST_PRINT(state2)
        Mode state3({{"phase", "preparing"}, {"source", "table"}});
        OPERA_TEST_EQUAL(state3.values().size(),2)
        OPERA_TEST_PRINT(state3)
    }

    void test_mode_comparison() {
        Mode state1({"robot", "first"});
        Mode state2({"robot", "first"});
        Mode state3({"robot", "second"});
        Mode state4({"other", "first"});
        OPERA_TEST_EQUAL(state1,state2)
        OPERA_TEST_ASSERT(state1 < state3)
        OPERA_TEST_ASSERT(state4 < state3)
        OPERA_TEST_ASSERT(not (state1 == state3))
        try {
            bool result = (state4 == state2);
            OPERA_PRINT_TEST_COMMENT("Error: expected exception, got " << result)
            ++OPERA_TEST_FAILURES;
        } catch (...) {
            OPERA_PRINT_TEST_COMMENT("Exception caught as expected")
        }
        try {
            bool result = (state2 == state4);
            OPERA_PRINT_TEST_COMMENT("Error: expected exception, got " << result)
            ++OPERA_TEST_FAILURES;
        } catch (...) {
            OPERA_PRINT_TEST_COMMENT("Exception caught as expected")
        }
    }

    void test_mode_trace_creation() {
        String robot("robot");
        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}), fourth({robot, "fourth"});

        ModeTrace trace;
        trace.push_front(second).push_back(first,1.0).push_back(second,1.0).push_front(third);
        OPERA_TEST_EQUALS(trace.size(),4)

        List<Mode> modes = {third, second, first, second};

        OPERA_TEST_ASSERT(trace.contains(first))
        OPERA_TEST_ASSERT(trace.contains(second))
        OPERA_TEST_ASSERT(trace.contains(third))
        OPERA_TEST_ASSERT(not trace.contains(fourth))
        OPERA_TEST_EQUALS(trace.at(0).mode,third)
        OPERA_TEST_EQUALS(trace.at(1).mode,second)
        OPERA_TEST_EQUALS(trace.at(2).mode,first)
        OPERA_TEST_EQUALS(trace.at(3).mode,second)
        OPERA_TEST_EQUALS(trace.likelihood(), 1)

        ModeTrace trace2;
        trace2 = trace;
        OPERA_TEST_EQUALS(trace2,trace)
    }

    void test_mode_trace_compare() {
        String robot("robot");
        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"});

        auto trace1 = ModeTrace().push_back(first,1.0).push_back(second,1.0).push_back(third,0.5);
        auto trace2 = ModeTrace().push_back(first,1.0).push_back(second,1.0).push_back(third,1.0);
        auto trace3 = ModeTrace().push_back(first,1.0).push_back(second,1.0).push_back(third,1.0);
        auto trace4 = ModeTrace().push_back(second,1.0).push_back(first,1.0).push_back(third,0.5);

        OPERA_TEST_ASSERT(not(trace1 == trace2))
        OPERA_TEST_ASSERT(not(trace1 == trace4))
        OPERA_TEST_ASSERT(trace2 == trace3)
    }

    void test_mode_trace_has_looped() {
        String robot("robot");
        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"});

        auto trace1 = ModeTrace();
        OPERA_TEST_ASSERT(not trace1.has_looped())
        auto trace2 = ModeTrace().push_back(first);
        OPERA_TEST_ASSERT(not trace2.has_looped())
        auto trace3 = ModeTrace().push_back(first).push_back(second);
        OPERA_TEST_ASSERT(not trace3.has_looped())
        auto trace4 = ModeTrace().push_back(first).push_back(first);
        OPERA_TEST_ASSERT(trace4.has_looped())
        auto trace5 = ModeTrace().push_back(first).push_back(second).push_back(first);
        OPERA_TEST_ASSERT(trace5.has_looped())
        auto trace6 = ModeTrace().push_back(first).push_back(second).push_back(third).push_back(second);
        OPERA_TEST_ASSERT(trace6.has_looped())
        auto trace7 = ModeTrace().push_back(first).push_back(second).push_back(first).push_back(third);
        OPERA_TEST_ASSERT(not trace7.has_looped())
    }

    void test_mode_trace_merge() {
        String robot("robot");
        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}),
                fourth({robot,"fourth"}), fifth({robot,"fifth"}), sixth({robot,"sixth"});

        auto trace1 = ModeTrace().push_back(first,1.0).push_back(second,1.0).push_back(third,0.5);
        auto trace2 = ModeTrace().push_back(second,1.0).push_back(fourth,0.8);
        auto trace4 = ModeTrace().push_back(fourth,1.0).push_back(fifth,1.0);

        auto merge12 = merge(trace1,trace2);
        OPERA_TEST_EQUAL(merge12.likelihood(),0.8)
        OPERA_TEST_PRINT(merge12)
        OPERA_TEST_EQUAL(merge12.size(),5)

        auto merge21 = merge(trace2,trace1);
        OPERA_TEST_EQUAL(merge21.likelihood(),0.5)
        OPERA_TEST_PRINT(merge21)
        OPERA_TEST_EQUAL(merge21.size(),5)
    }

    void test_mode_trace_indexes() {
        String robot("robot");
        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}),
                fourth({robot,"fourth"}), fifth({robot,"fifth"});

        auto trace = ModeTrace().push_back(first).push_back(second).push_back(third).push_back(second).push_back(fifth);

        OPERA_TEST_EQUALS(trace.forward_index(fourth),-1)
        OPERA_TEST_EQUALS(trace.backward_index(fourth),-1)
        OPERA_TEST_EQUALS(trace.forward_index(third),2)
        OPERA_TEST_EQUALS(trace.backward_index(third),2)
        OPERA_TEST_EQUALS(trace.forward_index(first),0)
        OPERA_TEST_EQUALS(trace.backward_index(first),0)
        OPERA_TEST_EQUALS(trace.forward_index(second),1)
        OPERA_TEST_EQUALS(trace.backward_index(second),3)
    }

    void test_mode_trace_reduce_between() {
        String robot("robot");
        Mode first({robot, "first"}), second({robot, "second"}), third({robot, "third"}),
                fourth({robot,"fourth"}), fifth({robot,"fifth"}), sixth({robot,"sixth"});

        auto trace1 = ModeTrace().push_back(first).push_back(second).push_back(third);
        auto trace2 = trace1;
        auto trace3 = trace1;

        OPERA_TEST_FAIL(trace1.reduce_between(first,fourth))
        OPERA_TEST_FAIL(trace1.reduce_between(fourth,third))
        OPERA_TEST_FAIL(trace1.reduce_between(second,first))
        trace1.reduce_between(first,third);
        OPERA_TEST_EQUALS(trace1.size(),3)
        trace1.reduce_between(first,first);
        OPERA_TEST_EQUALS(trace1.size(),1)
        OPERA_TEST_EQUALS(trace1.ending_mode(),first)
        trace2.reduce_between(first,second);
        OPERA_TEST_EQUALS(trace2.size(),2)
        OPERA_TEST_EQUALS(trace2.starting_mode(),first)
        OPERA_TEST_EQUALS(trace2.ending_mode(),second)
        trace3.reduce_between(second,third);
        OPERA_TEST_EQUALS(trace3.starting_mode(),second)
        OPERA_TEST_EQUALS(trace3.ending_mode(),third)
        OPERA_TEST_EQUALS(trace3.size(),2)
    }

    void test_mode_trace_next_modes() {
        String r("r");
        Mode a({r, "a"}), b({r, "b"}), c({r, "c"}), d({r, "d"});

        // abcabd -> *****d -> {}
        auto next1 = ModeTrace().push_back(a).push_back(b).push_back(c).push_back(a).push_back(b).
                push_back(d).next_modes();
        OPERA_TEST_EQUALS(next1.size(),0)

        // abcabdacbcabcdac -> ******ac******ac -> {b}
        auto next2 = ModeTrace().push_back(a).push_back(b).push_back(c).push_back(a).push_back(b).
                push_back(d).push_back(a).push_back(c).push_back(b).push_back(c).push_back(a).
                push_back(b).push_back(c).push_back(d).push_back(a).push_back(c).next_modes();
        OPERA_TEST_EQUALS(next2.size(),1)
        OPERA_TEST_ASSERT(next2.has_key(b))
        OPERA_TEST_EQUALS(next2.at(b),1.0)

        // abcbacabcbacbacb -> **cb*****cb*cb*cb -> {a}
        auto next3 = ModeTrace().push_back(a).push_back(b).push_back(c).push_back(b).push_back(a).
                push_back(c).push_back(a).push_back(b).push_back(c).push_back(b).push_back(a).
                push_back(c).push_back(b).push_back(a).push_back(c).push_back(b).next_modes();
        OPERA_TEST_EQUALS(next3.size(),1)
        OPERA_TEST_ASSERT(next3.has_key(a))
        OPERA_TEST_EQUALS(next3.at(a),1.0)

        // abdabcabcdabadbc -> ****bc*bc*****bc -> {a,d}
        auto next4 = ModeTrace().push_back(a).push_back(b).push_back(d).push_back(a).push_back(b).
                push_back(c).push_back(a).push_back(b).push_back(c).push_back(d).push_back(a).
                push_back(b).push_back(a).push_back(d).push_back(b).push_back(c).next_modes();
        OPERA_TEST_EQUALS(next4.size(),2)
        OPERA_TEST_ASSERT(next4.has_key(a))
        OPERA_TEST_ASSERT(next4.has_key(d))
        OPERA_TEST_EQUALS(next4.at(a),0.5)
        OPERA_TEST_EQUALS(next4.at(d),0.5)

        // dcbadcbdcbdcbcdcb -> dcb*dcbdcbdcb*dcb -> {a(0.25),c(0.25),d(0.5)}
        auto trace5 = ModeTrace().push_back(d).push_back(c).push_back(b).push_back(a).push_back(d).
                push_back(c).push_back(b).push_back(d).push_back(c).push_back(b).push_back(d).
                push_back(c).push_back(b).push_back(c).push_back(d).push_back(c).push_back(b);
        auto next5 = trace5.next_modes();
        OPERA_TEST_EQUALS(next5.size(),3)
        OPERA_TEST_ASSERT(next5.has_key(a))
        OPERA_TEST_ASSERT(next5.has_key(c))
        OPERA_TEST_ASSERT(next5.has_key(d))
        OPERA_TEST_EQUALS(next5.at(a),0.25)
        OPERA_TEST_EQUALS(next5.at(c),0.25)
        OPERA_TEST_EQUALS(next5.at(d),0.5)

        // adding each next state
        auto trace5a = trace5;
        trace5a.push_back(a,0.25);
        auto trace5c = trace5;
        trace5c.push_back(c,0.25);
        auto trace5d = trace5;
        trace5d.push_back(d,0.5);
        auto next5a = trace5a.next_modes();
        auto next5c = trace5c.next_modes();
        auto next5d = trace5d.next_modes();
        OPERA_TEST_EQUALS(next5a.size(),1)
        OPERA_TEST_EQUALS(next5c.size(),1)
        OPERA_TEST_EQUALS(next5d.size(),1)
        OPERA_TEST_ASSERT(next5a.has_key(d))
        OPERA_TEST_ASSERT(next5c.has_key(d))
        OPERA_TEST_ASSERT(next5d.has_key(c))

        // adding each next state
        auto trace5ad = trace5a.push_back(d);
        auto trace5cd = trace5c.push_back(d);
        auto trace5dc = trace5d.push_back(c);
        OPERA_TEST_EQUALS(trace5ad.likelihood(),0.25)
        OPERA_TEST_EQUALS(trace5cd.likelihood(),0.25)
        OPERA_TEST_EQUALS(trace5dc.likelihood(),0.5)
        auto next5ad = trace5ad.next_modes();
        auto next5cd = trace5cd.next_modes();
        auto next5dc = trace5dc.next_modes();
        OPERA_TEST_EQUALS(next5ad.size(),1)
        OPERA_TEST_EQUALS(next5cd.size(),1)
        OPERA_TEST_EQUALS(next5dc.size(),1)
        OPERA_TEST_ASSERT(next5ad.has_key(c))
        OPERA_TEST_ASSERT(next5cd.has_key(c))
        OPERA_TEST_ASSERT(next5dc.has_key(b))

        // adding each next state
        auto trace5adc = trace5ad.push_back(c);
        auto trace5cdc = trace5cd.push_back(c);
        auto trace5dcb = trace5dc.push_back(b);
        auto next5adc = trace5adc.next_modes();
        auto next5cdc = trace5cdc.next_modes();
        auto next5dcb = trace5dcb.next_modes();
        OPERA_TEST_EQUALS(next5adc.size(),1)
        OPERA_TEST_EQUALS(next5cdc.size(),1)
        OPERA_TEST_EQUALS(next5dcb.size(),2)
        OPERA_TEST_ASSERT(next5adc.has_key(b))
        OPERA_TEST_ASSERT(next5cdc.has_key(b))
        OPERA_TEST_ASSERT(next5dcb.has_key(c))
        OPERA_TEST_ASSERT(next5dcb.has_key(d))
        auto trace5dcbc = trace5dcb;
        trace5dcbc.push_back(c,next5dcb.at(c));
        OPERA_TEST_EQUALS(trace5dcbc.likelihood(),0.25)
        auto trace5dcbd = trace5dcb;
        trace5dcbd.push_back(d,next5dcb.at(d));
        OPERA_TEST_EQUALS(trace5dcbd.likelihood(),0.25)
    }
};

int main() {
    TestMode().test();
    return OPERA_TEST_FAILURES;
}
