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
        OPERA_TEST_CALL(test_humanstatemessage_make())
        OPERA_TEST_CALL(test_robotstatemessage_make())
        OPERA_TEST_CALL(test_collisiondetectionmessage_make())
    }

    void test_bodypresentationmessage_make_human() {
        Deserialiser<BodyPresentationMessage> d1(Resources::path("json/examples/presentation/human0.json"));
        auto p1 = d1.make();
        OPERA_TEST_ASSERT(p1.is_human())
        Deserialiser<BodyPresentationMessage> d2("{\n"
                                   "  \"id\": \"h0\",\n"
                                   "  \"isHuman\": true,\n"
                                   "  \"segmentPairs\": [[\"nose\",\"nose\"],[\"left_wrist\",\"left_wrist\"],[\"right_wrist\",\"right_wrist\"],"
                                   "  [\"nose\",\"neck\"],[\"left_shoulder\",\"right_shoulder\"],[\"left_shoulder\",\"left_elbow\"],[\"left_elbow\",\"left_wrist\"],"
                                   "  [\"right_shoulder\",\"right_elbow\"],[\"right_elbow\",\"right_wrist\"],[\"right_shoulder\",\"right_hip\"],"
                                   "  [\"left_shoulder\",\"left_hip\"],[\"left_hip\",\"right_hip\"],[\"right_hip\",\"right_knee\"],[\"right_knee\",\"right_ankle\"],"
                                   "  [\"left_hip\",\"left_knee\"],[\"left_knee\",\"left_ankle\"],[\"chest\",\"mid_hip\"]],\n"
                                   "  \"thicknesses\": [0.13,0.085,0.085,0.058,0.12,0.05,0.042,0.05,0.042,0.1,0.1,0.125,0.097,0.066,0.097,0.066,0.1]\n"
                                   "}");

        auto p2 = d2.make();
        OPERA_TEST_EQUALS(p1.id(),p2.id())
        OPERA_TEST_EQUALS(p1.segment_pairs().size(),p2.segment_pairs().size())
    }
    void test_bodypresentationmessage_make_robot() {
        Deserialiser<BodyPresentationMessage> d1(Resources::path("json/examples/presentation/robot0.json"));
        auto p1 = d1.make();
        Deserialiser<BodyPresentationMessage> d2("{\n"
                            "  \"id\": \"r0\",\n"
                            "  \"isHuman\": false,\n"
                            "  \"messageFrequency\": 10,\n"
                            "  \"segmentPairs\": [[\"0\",\"1\"],[\"1\",\"2\"],[\"2\",\"3\"],[\"3\",\"4\"],[\"4\",\"5\"],[\"5\",\"6\"],[\"6\",\"7\"]],\n"
                            "  \"thicknesses\": [1,1,1,1,1,1,1]\n"
                            "}");
        auto p2 = d2.make();
        OPERA_TEST_EQUALS(p1.id(),p2.id())
        OPERA_TEST_EQUALS(p1.segment_pairs().size(),p2.segment_pairs().size())
    }

    void test_humanstatemessage_make() {
        Deserialiser<HumanStateMessage> d(Resources::path("json/examples/state/humans.json"));
        auto p = d.make();
        OPERA_TEST_EQUALS(p.bodies().size(),1)
        OPERA_TEST_EQUALS(p.timestamp(),328903)
        auto const& bd = p.bodies().at(0);
        OPERA_TEST_EQUALS(bd.first,"h0")
        OPERA_TEST_EQUALS(bd.second.size(),8)
        OPERA_TEST_EQUALS(bd.second.at("hip").size(),0)
        OPERA_TEST_EQUALS(bd.second.at("head").size(),1)
        OPERA_TEST_EQUALS(bd.second.at("left_wrist").size(),2)
        OPERA_TEST_EQUALS(bd.second.at("right_wrist").size(),1)
        OPERA_TEST_EQUALS(bd.second.at("left_knee").size(),1)
        OPERA_TEST_EQUALS(bd.second.at("right_knee").size(),1)
        OPERA_TEST_EQUALS(bd.second.at("left_elbow").size(),1)
        OPERA_TEST_EQUALS(bd.second.at("right_elbow").size(),1)
    }

    void test_robotstatemessage_make() {
        Mode loc({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}});
        Deserialiser<RobotStateMessage> d(Resources::path("json/examples/state/robot0.json"));
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
        OPERA_TEST_EQUALS(p.timestamp(),328903)
    }

    void test_collisiondetectionmessage_make() {
        Mode loc({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}});
        Deserialiser<CollisionNotificationMessage> d(Resources::path("json/examples/notification/notification0.json"));
        auto p = d.make();
        OPERA_TEST_EQUALS(p.human_id(),"h0")
        OPERA_TEST_EQUALS(p.human_segment().first, "nose")
        OPERA_TEST_EQUALS(p.human_segment().second, "neck")
        OPERA_TEST_EQUALS(p.robot_id(),"r0")
        OPERA_TEST_EQUALS(p.robot_segment().first, "5")
        OPERA_TEST_EQUALS(p.robot_segment().second, "6")
        OPERA_TEST_EQUALS(p.current_time(),32890)
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
