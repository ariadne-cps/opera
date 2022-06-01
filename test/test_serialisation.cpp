/***************************************************************************
 *            test_serialisation.cpp
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
#include "serialisation.hpp"

using namespace Opera;

class TestSerialisation {
public:
    void test() {
        OPERA_TEST_CALL(test_bodypresentationmessage_human())
        OPERA_TEST_CALL(test_bodypresentationmessage_robot())
        OPERA_TEST_CALL(test_humanstatemessage())
        OPERA_TEST_CALL(test_robotstatemessage())
        OPERA_TEST_CALL(test_collisionnotificationmessage())
    }

    void test_bodypresentationmessage_human() {
        BodyPresentationMessage p("human1", {{"nose","neck"},{"left_shoulder","right_shoulder"}}, {1.0,0.5});
        Serialiser<BodyPresentationMessage> serialiser(p);
        serialiser.to_file(Resources::path("json/examples/presentation/" + p.id() + ".tmp.json"));
        OPERA_TEST_EQUALS(serialiser.to_string(),"{\"id\":\"human1\",\"isHuman\":true,\"segmentPairs\":[[\"nose\",\"neck\"],[\"left_shoulder\",\"right_shoulder\"]],\"thicknesses\":[1.0,0.5]}")
    }

    void test_bodypresentationmessage_robot() {
        BodyPresentationMessage p("robot1", 30, {{"0", "1"},{"3", "2"},{"4", "2"}}, {1.0,0.5, 0.5});
        Serialiser<BodyPresentationMessage> serialiser(p);
        serialiser.to_file(Resources::path("json/examples/presentation/" + p.id() + ".tmp.json"));
        OPERA_TEST_EQUALS(serialiser.to_string(),"{\"id\":\"robot1\",\"isHuman\":false,\"messageFrequency\":30,\"segmentPairs\":[[\"0\",\"1\"],[\"3\",\"2\"],[\"4\",\"2\"]],\"thicknesses\":[1.0,0.5,0.5]}")
    }

    void test_humanstatemessage() {
        HumanStateMessage p({{"human0",{{{"nose",{Point(0.4,2.1,0.2)}},{"neck",{Point(0,-1,0.1),Point(0.3,3.1,-1.2)}},{"left_shoulder",{Point(0.4,0.1,1.2)}},{"right_shoulder",{Point(0,0,1)}}}}}},3423235);
        Serialiser<HumanStateMessage> serialiser(p);
        serialiser.to_file(Resources::path("json/examples/state/humans.tmp.json"));
        OPERA_TEST_EQUALS(serialiser.to_string(),"{\"bodies\":[{\"bodyId\":\"human0\",\"keypoints\":{\"left_shoulder\":[{\"x\":0.4,\"y\":0.1,\"z\":1.2}],\"neck\":[{\"x\":0.0,\"y\":-1.0,\"z\":0.1},{\"x\":0.3,\"y\":3.1,\"z\":-1.2}],\"nose\":[{\"x\":0.4,\"y\":2.1,\"z\":0.2}],\"right_shoulder\":[{\"x\":0.0,\"y\":0.0,\"z\":1.0}]}}],\"timestamp\":3423235}")
    }

    void test_robotstatemessage() {
        RobotStateMessage p("robot0", Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), {{}, {Point(0, -1, 0.1), Point(0.3, 3.1, -1.2)}, {}}, 93249);
        Serialiser<RobotStateMessage> serialiser(p);
        serialiser.to_file(Resources::path("json/examples/state/" + p.id() + ".tmp.json"));
        OPERA_TEST_EQUALS(serialiser.to_string(),"{\"bodyId\":\"robot0\",\"mode\":{\"destination\":\"2\",\"origin\":\"3\",\"phase\":\"pre\"},\"continuousState\":[[],[[0.0,-1.0,0.1],[0.3,3.1,-1.2]],[]],\"timestamp\":93249}")
    }

    void test_collisionnotificationmessage() {
        CollisionNotificationMessage p("h0", {"nose","neck"}, "r0", {"0","1"}, 32890, Interval<TimestampType>(72, 123), Mode({{"origin", "3"}, {"destination", "2"}, {"phase", "pre"}}), 0.5);
        Serialiser<CollisionNotificationMessage> serialiser(p);
        serialiser.to_file(Resources::path("json/examples/notification/notification0.tmp.json"));
        OPERA_TEST_EQUALS(serialiser.to_string(),"{\"human\":{\"bodyId\":\"h0\",\"segmentId\":[\"nose\",\"neck\"]},\"robot\":{\"bodyId\":\"r0\",\"segmentId\":[\"0\",\"1\"]},\"currentTime\":32890,\"collisionDistance\":{\"lower\":72,\"upper\":123},\"collisionMode\":{\"destination\":\"2\",\"origin\":\"3\",\"phase\":\"pre\"},\"likelihood\":0.5}")
    }
};


int main() {
    TestSerialisation().test();

    return OPERA_TEST_FAILURES;
}
