/***************************************************************************
 *            profile_geometry.cpp
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

#include "geometry.hpp"
#include "profile.hpp"

using namespace Opera;

struct ProfileGeometry : public Profiler {

    ProfileGeometry() : Profiler(100000) { }

    void run() {
        profile_centre();
        profile_hull();
        profile_average_geometric_median();
        profile_circle_radius();
        profile_point_point_distance();
        profile_point_segment_distance();
        profile_segment_segment_distance();
        profile_ternary_segment_distance_check();
        profile_spherical_ternary_segment_distance_check();
    }

    void profile_centre() {
        List<Box> bbs;
        List<Point> heads, tails;
        for (SizeType i=0; i<num_tries(); ++i) {
            auto pt1 = Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            auto pt2 = Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            heads.push_back(pt1);
            tails.push_back(pt2);
            bbs.push_back(hull(pt1,pt2));
        }

        profile("Centre of a box",[&](SizeType i){ bbs.at(i).centre(); });
        profile("Centre of two points",[&](SizeType i){ centre(heads.at(i),tails.at(i)); });
    }

    void profile_hull() {
        List<Point> heads, tails;
        for (SizeType i=0; i<num_tries(); ++i) {
            heads.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            tails.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
        }

        profile("Hull of two points",[&](SizeType i){ hull(heads.at(i),heads.at(i)); });
    }

    void profile_average_geometric_median() {
        SizeType const MAX_POINTS = 5;
        List<List<List<Point>>> pts;
        for (SizeType i=0; i<num_tries(); ++i) {
            pts.push_back(List<List<Point>>());
            for (SizeType j=0; j<=MAX_POINTS-2; ++j) {
                pts.at(i).push_back(List<Point>());
                for (SizeType k=0; k<j+2; ++k) {
                    pts.at(i).at(j).emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
                }
            }
        }

        for (SizeType j=2; j<=MAX_POINTS; ++j) {
            profile("Average of " + std::to_string(j) + " points",[&](SizeType i){ average(pts.at(i).at(j-2)); });
            profile("Geometric median of " + std::to_string(j) + " points",[&](SizeType i){ geometric_median(pts.at(i).at(j-2)); });
        }
    }

    void profile_circle_radius() {
        List<Box> bbs;
        for (SizeType i=0; i<num_tries(); ++i) {
            auto pt1 = Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            auto pt2 = Point(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            bbs.push_back(hull(pt1,pt2));
        }

        profile("Bounding box circle radius",[&](SizeType i){ bbs.at(i).circle_radius(); });
    }

    void profile_point_point_distance() {
        List<Point> p1, p2;
        for (SizeType i=0; i<num_tries(); ++i) {
            p1.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            p2.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
        }

        profile("Point-point segment_distance",[&](SizeType i){ distance(p1.at(i),p2.at(i)); });
    }

    void profile_point_segment_distance() {
        List<Point> points, heads, tails;
        for (SizeType i=0; i<num_tries(); ++i) {
            points.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            heads.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            tails.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
        }

        profile("Point-segment sphere_capsule_distance using segment segment_distance",[&](SizeType i){ distance(points.at(i),points.at(i),heads.at(i),tails.at(i)); });
        profile("Point-segment segment_distance using dedicated sphere_capsule_distance",[&](SizeType i){ distance(points.at(i),heads.at(i),tails.at(i)); });
    }

    void profile_segment_segment_distance() {
        Point s1h(1.0,3.0,-2.0);
        Point s1t(4.0,1.2,0);
        List<Point> heads, tails;
        for (SizeType i=0; i<num_tries(); ++i) {
            heads.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
            tails.emplace_back(rnd().get(-5.0,5.0),rnd().get(-5.0,5.0),rnd().get(-5.0,5.0));
        }

        profile("Segment-segment segment_distance",[&](SizeType i){ distance(s1h,s1t,heads.at(i),tails.at(i)); });
    }

    void profile_ternary_segment_distance_check() {
        Point s1h(0.0,0.0,0.0);
        Point s1t(1.0,0.0,0.0);

        Point s2h(0.0,1.0,0.0);
        Point s2t(1.0,1.0,0.0);

        Point s3h(0.0,4.0,0.0);
        Point s3t(1.0,4.0,0.0);

        FloatType result;

        profile("Ternary segment segment_distance check",[&](auto){
            result = distance(s1h,s1t,s3h,s3t) - std::max(distance(s2h,s3h,s3t),distance(s2t,s3h,s3t));
        });
    }

    void profile_spherical_ternary_segment_distance_check() {
        Point s1h(0.0,0.0,0.0);
        Point s1t(1.0,0.0,0.0);

        Point s2h(0.0,1.0,0.0);
        Point s2t(1.0,1.0,0.0);

        Point s3h(0.0,4.0,0.0);
        Point s3t(1.0,4.0,0.0);

        FloatType result;

        profile("Spherical ternary segment segment_distance check",[&](auto){
            auto c1 = (s1h+s1t)/2;
            auto c2 = (s2h+s2t)/2;
            result = distance(c1,s3h,s3t) - distance(c1,c2);
        });
    }
};


int main() {
    ProfileGeometry().run();
}
