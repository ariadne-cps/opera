/***************************************************************************
 *            test_body_registry.cpp
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

#include "body_registry.hpp"

#include "test.hpp"

using namespace Opera;

class TestBodyRegistry {
  public:
    void test() {
        OPERA_TEST_CALL(test_creation())
        OPERA_TEST_CALL(test_insert_remove_clear())
        OPERA_TEST_CALL(test_instance_distance())
    }

    void test_creation() {
        BodyRegistry registry;
        OPERA_TEST_EQUAL(registry.num_humans(),0)
        OPERA_TEST_EQUAL(registry.num_robots(),0)
        OPERA_TEST_EQUAL(registry.num_segment_pairs(),0)
    }

    void test_insert_remove_clear() {
        BodyRegistry registry;
        BodyPresentationMessage h("h0",{{"nose","neck"},{"neck","mid_hip"}},{1.0,0.5});
        BodyPresentationMessage r("r0",10,{{"0","1"},{"1","2"}},{1.0,0.5});

        registry.insert(h);
        registry.insert(r);
        OPERA_TEST_EQUALS(registry.num_humans(),1)
        OPERA_TEST_EQUALS(registry.num_robots(),1)
        OPERA_TEST_EQUALS(registry.num_segment_pairs(),4)
        OPERA_TEST_ASSERT(registry.contains(h.id()))
        OPERA_TEST_ASSERT(registry.contains(r.id()))
        OPERA_TEST_EQUALS(registry.human_ids().size(),1)
        OPERA_TEST_EQUALS(registry.robot_ids().size(),1)
        OPERA_TEST_FAIL(registry.human("h"))
        OPERA_TEST_FAIL(registry.robot("r"))
        OPERA_TEST_FAIL(registry.latest_human_instance_within("h0",0))
        OPERA_TEST_FAIL(registry.latest_human_timestamp("h0"))
        OPERA_TEST_EQUALS(registry.human_history_size("h0"),0)
        OPERA_TEST_FAIL(registry.robot_history("r"))
        auto const& human = registry.human(h.id());
        auto const& robot = registry.robot(r.id());
        OPERA_TEST_EQUAL(human.id(),h.id())
        OPERA_TEST_EQUAL(robot.id(),r.id())
        OPERA_TEST_FAIL(registry.latest_human_instance_within(h.id(),0))
        auto& history = registry.robot_history(r.id());
        OPERA_TEST_ASSERT(history.snapshot_at(0).modes_with_samples().empty())

        OPERA_TEST_FAIL(registry.acquire_state(HumanStateMessage({{"h", {{}}}}, 0u)))

        registry.acquire_state({{{h.id(), {{{"nose",{Point(0,0,0)}},{"neck",{Point(4,4,4)}},{"mid_hip",{Point(0,2,0)}}}}}}, 34289023});
        auto const& last_state = registry.latest_human_instance_within(h.id(),34289023);
        OPERA_TEST_EQUALS(last_state.timestamp(),34289023)
        OPERA_TEST_EQUALS(registry.instance_number(h.id(),34289023),0)
        OPERA_TEST_EQUALS(registry.instance_at(h.id(),0).timestamp(),34289023)

        registry.insert(h);
        registry.insert(r);
        OPERA_TEST_EQUALS(registry.num_humans(),1)
        OPERA_TEST_EQUALS(registry.num_robots(),1)

        registry.remove(h.id());
        registry.remove(r.id());
        OPERA_TEST_FAIL(registry.remove(h.id()))
        OPERA_TEST_FAIL(registry.remove(r.id()))
        OPERA_TEST_EQUALS(registry.num_humans(),0)
        OPERA_TEST_EQUALS(registry.num_robots(),0)
        OPERA_TEST_EQUALS(registry.num_segment_pairs(),0)

        registry.insert(h);
        registry.insert(r);
        registry.clear();
        OPERA_TEST_EQUALS(registry.num_humans(),0)
        OPERA_TEST_EQUALS(registry.num_robots(),0)
    }

    void test_instance_distance() {
        BodyRegistry registry;
        BodyPresentationMessage h("h0",{{"nose","neck"},{"neck","mid_hip"}},{1.0,0.5});
        BodyPresentationMessage r("r0",10,{{"0","1"},{"1","2"}},{1.0,0.5});

        registry.insert(h);
        registry.insert(r);

        registry.acquire_state({{{h.id(), {{{"nose",{Point(0,0,0)}},{"neck",{Point(4,4,4)}},{"mid_hip",{Point(0,2,0)}}}}}}, 1000});
        registry.acquire_state({{{h.id(), {{{"nose",{Point(0,0,0)}},{"neck",{Point(4,4,4)}},{"mid_hip",{Point(0,2,0)}}}}}}, 2000});
        registry.acquire_state({{{h.id(), {{{"nose",{Point(0,0,0)}},{"neck",{Point(4,4,4)}},{"mid_hip",{Point(0,2,0)}}}}}}, 3000});

        OPERA_TEST_EQUALS(registry.instance_distance(h.id(),1000,3000),2)
        OPERA_TEST_EQUALS(registry.instance_distance(h.id(),1000,2000),1)
        OPERA_TEST_EQUALS(registry.instance_distance(h.id(),1000,1000),0)
        OPERA_TEST_FAIL(registry.instance_distance(h.id(),2000,1000))
        OPERA_TEST_FAIL(registry.instance_distance(h.id(),1000,1001))
        OPERA_TEST_FAIL(registry.instance_distance(h.id(),1001,2000))
    }
};

int main() {
    TestBodyRegistry().test();
    return OPERA_TEST_FAILURES;
}
