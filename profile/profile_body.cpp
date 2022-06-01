/***************************************************************************
 *            profile_body.hpp
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

#include "body.hpp"
#include "profile.hpp"

using namespace Opera;

struct ProfileBody : public Profiler {

    ProfileBody() : Profiler(100000) { }

    void run() {
        profile_bodysegment_intersection();
        profile_bodysegment_sample_update();
        profile_approximation_creation();
    }

    void profile_bodysegment_intersection() {
        FloatType thickness = 1.0;
        Robot r("r0", 10, {{"0", "1"}}, {thickness});
        auto segment = r.segment(0);

        auto s1 = segment.create_sample();
        s1.update({Point(0, 0, 0)},{Point(5, 5, 5)});
        auto s2 = segment.create_sample();
        s2.update({Point(0, 3, 0)},{Point(6, 6, 6)});
        auto s3 = segment.create_sample();
        s3.update({Point(0, 8, 0)},{Point(0, 10, 0)});

        auto s1s = s1.bounding_sphere();
        auto s3s = s3.bounding_sphere();

        bool result;

        profile("Box intersection checking",[&](auto){ result = s1.intersects(s3); });
        profile("Sphere intersection checking",[&](auto){ result = (distance(s1s.centre(),s3s.centre()) > s1s.radius()+s3s.radius()); });
        profile("Capsule intersection checking",[&](auto){ result = s1.intersects(s2); });
    }

    void profile_bodysegment_sample_update() {
        FloatType thickness = 1.0;
        Robot r("r0", 10, {{"0", "1"}}, {thickness});
        auto segment = r.segment(0);

        auto s = segment.create_sample();
        s.update({Point(0, 0, 0)}, {Point(5, 5, 5)});

        List<Point> heads, tails;
        for (SizeType i = 0; i < num_tries(); ++i) {
            heads.emplace_back(rnd().get(-5.0, 5.0), rnd().get(-5.0, 5.0), rnd().get(-5.0, 5.0));
            tails.emplace_back(rnd().get(-5.0, 5.0), rnd().get(-5.0, 5.0), rnd().get(-5.0, 5.0));
        }

        profile("Body segment sample update", [&](SizeType i) { s.update({heads.at(i)}, {tails.at(i)}); });
    }

    void profile_approximation_creation() {
        FloatType thickness = 1.0;
        Robot r("r0", 10, {{"0", "1"}}, {thickness});
        auto segment = r.segment(0);

        List<BodySegmentSample> samples;
        for (SizeType i=0; i<num_tries(); ++i) {
            auto s = segment.create_sample();
            s.update({Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))},{Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0))});
            samples.push_back(s);
        }

        profile("Box approximation creation",[&](SizeType i){ samples.at(i).bounding_box(); });
        profile("Sphere approximation creation",[&](SizeType i){ samples.at(i).bounding_sphere(); });
    }
};

int main() {
    ProfileBody().run();
}
