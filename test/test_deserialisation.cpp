/***************************************************************************
 *            test_deserialisation.cpp
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
#include "utility.hpp"
#include "deserialisation.hpp"

using namespace Opera;

class TestDeserialisation {
public:
    void test() {
        OPERA_TEST_CALL(test_bodypresentationmessage_make_human())
        OPERA_TEST_CALL(test_bodypresentationmessage_make_robot())
        OPERA_TEST_CALL(test_bodystatemessage_make())
        OPERA_TEST_CALL(test_collisiondetectionmessage_make())
    }

    void test_bodypresentationmessage_make_human() {
        Deserialiser<BodyPresentationMessage> d1(Resources::path("json/examples/presentation/human0.json"));
        auto p1 = d1.make();
        OPERA_TEST_ASSERT(p1.is_human())
        Deserialiser<BodyPresentationMessage> d2("{\n"
                                   "  \"id\": \"h0\",\n"
                                   "  \"isHuman\": true,\n"
                                   "  \"pointIds\": [[14,12],[11,12],[5,7],[6,8],[7,9],[8,10],[1,2],[0,1],[0,2],[1,3],[2,4],[3,5],[4,6],[17,0],[17,5],[17,6],[17,11],[17,12]],\n"
                                   "  \"thicknesses\": [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]\n"
                                   "}");
        auto p2 = d2.make();
        OPERA_TEST_EQUALS(p1.id(),p2.id())
    }
    void test_bodypresentationmessage_make_robot() {
        Deserialiser<BodyPresentationMessage> d1(Resources::path("json/examples/presentation/robot0.json"));
        auto p1 = d1.make();
        Deserialiser<BodyPresentationMessage> d2("{\n"
                            "  \"id\": \"r0\",\n"
                            "  \"isHuman\": false,\n"
                            "  \"messageFrequency\": 10,\n"
                            "  \"pointIds\": [[0,1],[1,2],[2,3],[3,4],[4,5],[5,6],[6,7]],\n"
                            "  \"thicknesses\": [1,1,1,1,1,1,1]\n"
                            "}");
        auto p2 = d2.make();
        OPERA_TEST_EQUALS(p1.id(),p2.id())
    }

    void test_bodystatemessage_make() {
        Mode loc({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}});
        Deserialiser<BodyStateMessage> d(Resources::path("json/examples/state/robot0.json"));
        auto p = d.make();
        OPERA_TEST_EQUALS(p.id(),"r0")
        OPERA_TEST_EQUALS(p.mode(),loc)
        OPERA_TEST_PRINT(p.points())
        OPERA_TEST_EQUALS(p.points().size(),8)
        OPERA_TEST_EQUALS(p.points().at(0).size(),1)
        OPERA_TEST_EQUALS(p.points().at(1).size(),1)
        OPERA_TEST_EQUALS(p.points().at(2).size(),2)
        OPERA_TEST_EQUALS(p.points().at(3).size(),1)
        OPERA_TEST_EQUALS(p.points().at(4).size(),1)
        OPERA_TEST_EQUALS(p.points().at(5).size(),1)
        OPERA_TEST_EQUALS(p.points().at(6).size(),1)
        OPERA_TEST_EQUALS(p.points().at(7).size(),1)
        OPERA_TEST_EQUALS(p.timestamp(),328903284232)
    }

    void test_collisiondetectionmessage_make() {
        Mode loc({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}});
        Deserialiser<CollisionNotificationMessage> d(Resources::path("json/examples/notification/notification0.json"));
        auto p = d.make();
        OPERA_TEST_EQUALS(p.human_id(),"h0")
        OPERA_TEST_EQUALS(p.human_segment_id(),0)
        OPERA_TEST_EQUALS(p.robot_id(),"r0")
        OPERA_TEST_EQUALS(p.robot_segment_id(),3)
        OPERA_TEST_EQUALS(p.current_time(),32890592300)
        OPERA_TEST_EQUALS(p.collision_distance().lower(), 72)
        OPERA_TEST_EQUALS(p.collision_distance().upper(), 123)
        OPERA_TEST_EQUALS(p.collision_mode(), loc)
        OPERA_TEST_EQUALS(p.likelihood(),0.5)
    }
};

int main() {
    TestDeserialisation().test();
    return OPERA_TEST_FAILURES;
}
