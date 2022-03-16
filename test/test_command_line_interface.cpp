/***************************************************************************
 *            test_command_line_interface.cpp
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

#include "command_line_interface.hpp"
#include "conclog/include/logging.hpp"
#include "test.hpp"

using namespace Opera;
using namespace ConcLog;

class TestCommandLineInterface {
  public:

    void test() {
        OPERA_TEST_CALL(test_empty_argument_stream())
        OPERA_TEST_CALL(test_nonempty_argument_stream())
        OPERA_TEST_CALL(test_cli_instantiation())
        OPERA_TEST_CALL(test_from_c_arguments())
        OPERA_TEST_CALL(test_scheduler_parsing())
        OPERA_TEST_CALL(test_theme_parsing())
        OPERA_TEST_CALL(test_verbosity_parsing())
        OPERA_TEST_CALL(test_multiple_argument_parsing())
        OPERA_TEST_CALL(test_unrecognised_argument())
        OPERA_TEST_CALL(test_duplicate_argument())
        OPERA_TEST_CALL(test_print_help())
    }

    void test_empty_argument_stream() {
        OPERA_TEST_FAIL(ArgumentStream({}))
    }

    void test_nonempty_argument_stream() {
        auto stream = ArgumentStream({"a","b"});
        OPERA_TEST_ASSERT(not stream.empty())
        OPERA_TEST_EQUALS(stream.size(),2)
        OPERA_TEST_EQUALS(stream.peek(),"a")
        auto val1 = stream.pop();
        OPERA_TEST_EQUALS(val1,"a")
        auto val2 = stream.pop();
        OPERA_TEST_EQUALS(val2,"b")
        OPERA_TEST_ASSERT(stream.empty())
        OPERA_TEST_FAIL(stream.peek())
        OPERA_TEST_FAIL(stream.pop())
    }

    void test_cli_instantiation() {
        OPERA_TEST_EXECUTE(CommandLineInterface::instance())
    }

    void test_from_c_arguments() {
        const char* argv1[] = {""};
        bool success1 = CommandLineInterface::instance().acquire(1,argv1);
        OPERA_TEST_ASSERT(success1)
        const char* argv2[] = {"",""};
        bool success2 = CommandLineInterface::instance().acquire(2,argv2);
        OPERA_TEST_ASSERT(not success2)
    }

    void test_scheduler_parsing() {
        bool success1 = CommandLineInterface::instance().acquire({"", "-s", "immediate"});
        OPERA_TEST_ASSERT(success1)
        bool success2 = CommandLineInterface::instance().acquire({"", "--scheduler", "immediate"});
        OPERA_TEST_ASSERT(success2)
        bool success3 = CommandLineInterface::instance().acquire({"", "-s", "wrong"});
        OPERA_TEST_ASSERT(not success3)
        bool success4 = CommandLineInterface::instance().acquire({"", "-s", "blocking"});
        OPERA_TEST_ASSERT(success4)
        bool success5 = CommandLineInterface::instance().acquire({"", "-s", "nonblocking"});
        OPERA_TEST_ASSERT(success5)
        bool success6 = CommandLineInterface::instance().acquire({"", "-s"});
        OPERA_TEST_ASSERT(not success6)
    }

    void test_theme_parsing() {
        bool success1 = CommandLineInterface::instance().acquire({"", "-t", "none"});
        OPERA_TEST_ASSERT(success1)
        bool success2 = CommandLineInterface::instance().acquire({"", "--theme", "none"});
        OPERA_TEST_ASSERT(success2)
        bool success3 = CommandLineInterface::instance().acquire({"", "-t", "nn"});
        OPERA_TEST_ASSERT(not success3)
        bool success4 = CommandLineInterface::instance().acquire({"", "-t", "light"});
        OPERA_TEST_ASSERT(success4)
        bool success5 = CommandLineInterface::instance().acquire({"", "-t", "dark"});
        OPERA_TEST_ASSERT(success5)
        bool success6 = CommandLineInterface::instance().acquire({"", "-t"});
        OPERA_TEST_ASSERT(not success6)
    }

    void test_verbosity_parsing() {
        bool success1 = CommandLineInterface::instance().acquire({"", "-v", "5"});
        OPERA_TEST_ASSERT(success1)
        OPERA_TEST_EQUALS(Logger::instance().configuration().verbosity(),5)
        bool success2 = CommandLineInterface::instance().acquire({"", "--verbosity", "0"});
        OPERA_TEST_ASSERT(success2)
        OPERA_TEST_EQUALS(Logger::instance().configuration().verbosity(),0)
        bool success3 = CommandLineInterface::instance().acquire({"", "-v", "-2"});
        OPERA_TEST_ASSERT(not success3)
        bool success4 = CommandLineInterface::instance().acquire({"", "-v", "q"});
        OPERA_TEST_ASSERT(not success4)
        bool success5 = CommandLineInterface::instance().acquire({"", "-v"});
        OPERA_TEST_ASSERT(not success5)
    }

    void test_multiple_argument_parsing() {
        bool success = CommandLineInterface::instance().acquire({"", "-t", "dark", "--verbosity", "4"});
        OPERA_TEST_ASSERT(success)
        OPERA_TEST_EQUALS(Logger::instance().configuration().verbosity(),4)
    }

    void test_unrecognised_argument() {
        bool success = CommandLineInterface::instance().acquire({"", "--invalid"});
        OPERA_TEST_ASSERT(not success)
    }

    void test_duplicate_argument() {
        bool success = CommandLineInterface::instance().acquire({"", "--verbosity", "2", "-v", "5"});
        OPERA_TEST_ASSERT(not success)
    }

    void test_print_help() {
        bool success = CommandLineInterface::instance().acquire({"", "-h"});
        OPERA_TEST_ASSERT(not success)
    }

};

int main() {
    TestCommandLineInterface().test();
    return OPERA_TEST_FAILURES;
}

