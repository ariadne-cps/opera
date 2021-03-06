/***************************************************************************
 *            profile_state.hpp
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

#include "state.hpp"
#include "message.hpp"
#include "profile.hpp"

using namespace Opera;

struct ProfileState : public Profiler {

    ProfileState() : Profiler(100000) { }

    void run() {
        profile_human_instance_acquirement();
        profile_robot_history_acquirement_and_update();
    }

    void profile_human_instance_acquirement() {
        FloatType thickness = 1.0;
        Human h("h0", {{"nose", "neck"}}, {thickness});

        List<HumanStateMessage> pkts;
        for (SizeType i=0; i<num_tries(); ++i) {
            pkts.push_back(HumanStateMessage({{h.id(),{{{"nose",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}},
                                      {"neck",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}}}}}},
                                              static_cast<TimestampType>(10*i)));
        }

        profile("Make human state instance from message fields",[&](SizeType i){ HumanStateInstance hsi(h,pkts.at(i).bodies().at(0).second,pkts.at(i).timestamp()); });
    }

    void profile_robot_history_acquirement_and_update() {
        FloatType thickness = 1.0;
        Robot r("r0", 10, {{"0", "1"}}, {thickness});
        RobotStateHistory history(r);

        List<Map<KeypointIdType,List<Point>>> pts;
        for (SizeType i=0; i<num_tries(); ++i) {
            pts.push_back({{{"0",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}},
                            {"1",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}}}});
        }

        profile("Acquire robot message for new mode",[&](SizeType i){ history.acquire(Mode({"robot", "first"}),pts.at(i),static_cast<TimestampType>(10*i)); });

        history.acquire(Mode({"robot", "second"}),
                        {{{"0",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}},
                         {"1",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}}}},
                        10000010);

        pts.clear();
        for (SizeType i=0; i<num_tries(); ++i) {
            pts.push_back({{{"0",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}},
                             {"1",{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))}}}});
        }

        profile("Acquire robot message for existing mode",[&](SizeType i){ history.acquire(Mode({"robot", "first"}),pts.at(i),static_cast<TimestampType>(10000020+10*i)); });
    }
};

int main() {
    ProfileState().run();
}
