/***************************************************************************
 *            test_thread.cpp
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

#include "thread.hpp"
#include "declarations.hpp"
#include "utility.hpp"
#include "test.hpp"

using namespace Opera;

using namespace std::chrono_literals;

class TestThread {
  public:

    void test_create() const {
        Thread thread1([]{}, "thr");
        OPERA_TEST_EXECUTE(thread1.id())
        OPERA_TEST_EQUALS(thread1.name(),"thr")
        Thread thread2([]{});
        OPERA_TEST_EQUALS(to_string(thread2.id()),thread2.name())
    }

    void test_destroy_before_completion() const {
        Thread thread([] { std::this_thread::sleep_for(100ms); });
    }

    void test_task() const {
        int a = 0;
        Thread thread([&a] { a++; });
        std::this_thread::sleep_for(10ms);
        OPERA_TEST_EQUALS(a,1)
    }

    void test_atomic_multiple_threads() const {
        SizeType n_threads = 10*std::thread::hardware_concurrency();
        OPERA_TEST_PRINT(n_threads)
        List<std::shared_ptr<Thread>> threads;

        std::atomic<SizeType> a = 0;
        for (SizeType i=0; i<n_threads; ++i) {
            threads.push_back(std::make_shared<Thread>([&a] { a++; }));
        }

        std::this_thread::sleep_for(100ms);
        OPERA_TEST_EQUALS(a,n_threads)
        threads.clear();
    }

    void test() {
        OPERA_TEST_CALL(test_create())
        OPERA_TEST_CALL(test_destroy_before_completion())
        OPERA_TEST_CALL(test_task())
        OPERA_TEST_CALL(test_atomic_multiple_threads())
    }

};

int main() {
    TestThread().test();
    return OPERA_TEST_FAILURES;
}
