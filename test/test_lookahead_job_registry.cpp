/***************************************************************************
 *            test_lookahead_job_registry.cpp
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

#include "lookahead_job_registry.hpp"

#include "test.hpp"

using namespace Opera;

class TestLookAheadJobRegistry {
  public:
    void test() {
        OPERA_TEST_CALL(test_lookaheadjob_treenode())
        OPERA_TEST_CALL(test_lookaheadjob_registry_entry())
        OPERA_TEST_CALL(test_lookaheadjob_registry())
    }

    void test_lookaheadjob_treenode() {
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register root first")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(0,LookAheadJobPath()))
            OPERA_TEST_ASSERT(root.has_registered(0, LookAheadJobPath()))
            OPERA_TEST_ASSERT(not root.try_register(0,LookAheadJobPath()))
            OPERA_TEST_ASSERT(not root.try_register(1,LookAheadJobPath().add(0,2)))
            OPERA_TEST_FAIL(root.has_registered(1, LookAheadJobPath()))
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register level 1 child first, and again")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(1,LookAheadJobPath().add(1,2)))
            OPERA_TEST_ASSERT(root.has_registered(1, LookAheadJobPath().add(1, 2)))
            OPERA_TEST_ASSERT(not root.try_register(1,LookAheadJobPath().add(1,2)))
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register level 1 child first, then root")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(1,LookAheadJobPath().add(1,2)))
            OPERA_TEST_ASSERT(root.has_registered(1, LookAheadJobPath().add(1, 2)))
            OPERA_TEST_ASSERT(root.try_register(0,LookAheadJobPath()))
            OPERA_TEST_ASSERT(root.has_registered(0, LookAheadJobPath()))
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register two level 1 children")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(1,LookAheadJobPath().add(1,2)))
            OPERA_TEST_ASSERT(root.has_registered(1, LookAheadJobPath().add(1, 2)))
            OPERA_TEST_ASSERT(root.try_register(1,LookAheadJobPath().add(0,2)))
            OPERA_TEST_ASSERT(root.has_registered(1, LookAheadJobPath().add(1, 2)))
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register two level 2 children")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(2,LookAheadJobPath().add(0,2).add(0,4)))
            OPERA_TEST_ASSERT(root.has_registered(2, LookAheadJobPath().add(0, 2).add(0, 4)))
            OPERA_TEST_ASSERT(root.try_register(2,LookAheadJobPath().add(1,2).add(0,3)))
            OPERA_TEST_ASSERT(root.has_registered(2, LookAheadJobPath().add(1, 2).add(0, 3)))
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register one level 1 child, then one level 2 child on the same branch")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(1,LookAheadJobPath().add(0,1)))
            OPERA_TEST_ASSERT(root.has_registered(1, LookAheadJobPath().add(0, 1)))
            OPERA_TEST_ASSERT(not root.try_register(2,LookAheadJobPath().add(0,2).add(0,3)))
            OPERA_TEST_ASSERT(not root.has_registered(2, LookAheadJobPath().add(0, 2).add(0, 3)))
        }
        {
            OPERA_PRINT_TEST_CASE_TITLE("Register one level 1 child, then one level 2 child on another branch")
            LookAheadJobTreeNode root(0);
            OPERA_TEST_ASSERT(root.try_register(1,LookAheadJobPath().add(0,1)))
            OPERA_TEST_ASSERT(root.has_registered(1, LookAheadJobPath().add(0, 1)))
            OPERA_TEST_ASSERT(root.try_register(2,LookAheadJobPath().add(1,2).add(0,3)))
            OPERA_TEST_ASSERT(root.has_registered(2, LookAheadJobPath().add(1, 2).add(0, 3)))
        }
    }

    void test_lookaheadjob_registry_entry() {
        LookAheadJobRegistryEntry entry(1000);
        LookAheadJobIdentifier id({"h0",0,"r0",1});
        OPERA_TEST_EQUALS(entry.timestamp(),1000)
        OPERA_TEST_ASSERT(entry.try_register(id,LookAheadJobPath().add(0,1)))
        OPERA_TEST_ASSERT(entry.has_registered(id, LookAheadJobPath().add(0, 1)))
        OPERA_TEST_ASSERT(not entry.try_register(id,LookAheadJobPath().add(0,2).add(0,3)))
        OPERA_TEST_ASSERT(not entry.has_registered(id, LookAheadJobPath().add(0, 2).add(0, 3)))
    }

    void test_lookaheadjob_registry() {
        LookAheadJobRegistry registry;
        LookAheadJobIdentifier id1({"h0",0,"r0",1});
        LookAheadJobIdentifier id2({"h1",0,"r0",1});
        OPERA_TEST_ASSERT(registry.try_register(1000,id1,LookAheadJobPath().add(0,1)))
        OPERA_TEST_ASSERT(registry.has_registered(1000, id1, LookAheadJobPath().add(0, 1)))
        OPERA_TEST_ASSERT(not registry.try_register(1000,id1,LookAheadJobPath().add(0,1)))
        OPERA_TEST_ASSERT(registry.try_register(1000,id1,LookAheadJobPath().add(1,2).add(0,3)))
        OPERA_TEST_ASSERT(registry.has_registered(1000, id1, LookAheadJobPath().add(1, 2).add(0, 3)))
        OPERA_TEST_FAIL(registry.try_register(100,id1,LookAheadJobPath().add(0,1)))
        OPERA_TEST_ASSERT(not registry.has_registered(100, id1, LookAheadJobPath().add(0, 1)))
        OPERA_TEST_ASSERT(registry.try_register(2000,id1,LookAheadJobPath().add(0,1)))
        OPERA_TEST_ASSERT(registry.has_registered(2000, id1, LookAheadJobPath().add(0, 1)))
        OPERA_TEST_ASSERT(not registry.try_register(2000,id1,LookAheadJobPath().add(0,1).add(0,2)))
        OPERA_TEST_ASSERT(not registry.has_registered(2000, id1, LookAheadJobPath().add(0, 1).add(0, 2)))
        OPERA_TEST_ASSERT(registry.try_register(2000,id2,LookAheadJobPath().add(0,1).add(0,2)))
        OPERA_TEST_ASSERT(registry.has_registered(2000, id2, LookAheadJobPath().add(0, 1).add(0, 2)))
    }
};

int main() {
    TestLookAheadJobRegistry().test();
    return OPERA_TEST_FAILURES;
}
