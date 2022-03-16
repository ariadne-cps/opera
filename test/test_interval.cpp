/***************************************************************************
 *            test_interval.cpp
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

#include "interval.hpp"

using namespace Opera;

class TestInterval {
public:
    void test() {
        OPERA_TEST_CALL(test_construct())
        OPERA_TEST_CALL(test_invalid())
        OPERA_TEST_CALL(test_equality())
    }

    void test_construct() {
        Interval<FloatType> interval(0.1,2.0);
        OPERA_TEST_EQUAL(interval.lower(),0.1)
        OPERA_TEST_EQUAL(interval.upper(),2.0)
        OPERA_TEST_PRINT(interval)

        Interval<FloatType> interval2(1);
        OPERA_TEST_PRINT(interval2)
    }

    void test_invalid() {
        OPERA_TEST_FAIL(Interval<FloatType>(1.0,0.5))
        Interval<FloatType> ivl(0,1);
        OPERA_TEST_FAIL(ivl.set_lower(2))
        OPERA_TEST_FAIL(ivl.set_upper(-1))
    }

    void test_equality() {
        Interval<FloatType> i1(0.1,2.0);
        Interval<FloatType> i2(0.0,1.0);
        OPERA_TEST_ASSERT(i1 != i2)
        i2.set_lower(0.1);
        i2.set_upper(2.0);
        OPERA_TEST_ASSERT(i1 == i2)
    }
};


int main() {
    TestInterval().test();

    return OPERA_TEST_FAILURES;
}
