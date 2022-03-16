/***************************************************************************
 *            test_trace_sample_range.cpp
 *
 *  Copyright  2022  Luca Geretti
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

#include "trace_sample_range.hpp"

#include "test.hpp"

using namespace Opera;

class TestTraceSampleRange {
  public:
    void test() {
        OPERA_TEST_CALL(test_construction())
        OPERA_TEST_CALL(test_add())
        OPERA_TEST_CALL(test_increase())
        OPERA_TEST_CALL(test_scale_down())
        OPERA_TEST_CALL(test_trim_down())
    }

    void test_construction() {
        TraceSampleRange r({0, 0});
        OPERA_TEST_EQUALS(r.size(),1)
        OPERA_TEST_ASSERT(not r.is_empty())
        OPERA_TEST_PRINT(r)
        OPERA_TEST_EQUALS(r.initial().trace,0)
        OPERA_TEST_EQUALS(r.initial().sample,0)
        OPERA_TEST_EQUALS(r.upper_bound(0),0)
        OPERA_TEST_EQUALS(r.maximum_trace_index(),0)
        OPERA_TEST_EQUALS(r.maximum_sample_index(),0)
    }

    void test_add() {
        TraceSampleRange r({2, 3});
        r.add(10);
        OPERA_TEST_PRINT(r)
        r.add(5);
        OPERA_TEST_PRINT(r)
        OPERA_TEST_EQUALS(r.initial().trace,2)
        OPERA_TEST_EQUALS(r.initial().sample,3)
        OPERA_TEST_FAIL(r.upper_bound(1))
        OPERA_TEST_FAIL(r.upper_bound(5))
        OPERA_TEST_EQUALS(r.upper_bound(4),5)
        OPERA_TEST_EQUALS(r.maximum_trace_index(),4)
        OPERA_TEST_EQUALS(r.maximum_sample_index(),5)
    }

    void test_increase() {
        TraceSampleRange r({3, 2});
        r.update(3);
        OPERA_TEST_EQUALS(r.maximum_sample_index(),3)
        r.increase_trace_index();
        OPERA_TEST_EQUALS(r.maximum_trace_index(),4)
        OPERA_TEST_EQUALS(r.maximum_sample_index(),0)
    }

    void test_scale_down() {
        auto r = TraceSampleRange({3, 2}).update(3).increase_trace_index().update(2).increase_trace_index().update(1);
        OPERA_TEST_PRINT(r)
        r.scale_down_trace_of(1);
        OPERA_TEST_EQUALS(r.initial().trace,2)
        OPERA_TEST_EQUALS(r.initial().sample,2)
        OPERA_TEST_EQUALS(r.maximum_trace_index(),4)
        OPERA_TEST_EQUALS(r.size(),3)
        r.scale_down_trace_of(3);
        OPERA_TEST_EQUALS(r.initial().trace,0)
        OPERA_TEST_EQUALS(r.initial().sample,0)
        OPERA_TEST_EQUALS(r.maximum_trace_index(),1)
        OPERA_TEST_EQUALS(r.size(),2)
        r.scale_down_trace_of(1);
        OPERA_TEST_EQUALS(r.maximum_trace_index(),0)
        OPERA_TEST_EQUALS(r.size(),1)
        r.scale_down_trace_of(1);
        OPERA_TEST_ASSERT(r.is_empty())
        OPERA_TEST_FAIL(r.maximum_trace_index())
        OPERA_TEST_FAIL(r.maximum_sample_index())
    }

    void test_trim_down() {
        auto r = TraceSampleRange({3, 2}).update(3).increase_trace_index().update(2).increase_trace_index().update(1);
        OPERA_TEST_EQUALS(r.maximum_sample_index(),1)
        r.trim_down_trace_to(4);
        OPERA_TEST_EQUALS(r.size(),2)
        OPERA_TEST_EQUALS(r.maximum_sample_index(),2)
        r.trim_down_trace_to(2);
        OPERA_TEST_ASSERT(r.is_empty())
    }
};

int main() {
    TestTraceSampleRange().test();
    return OPERA_TEST_FAILURES;
}
