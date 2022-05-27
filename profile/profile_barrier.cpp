/***************************************************************************
 *            profile_barrier.hpp
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

#include "barrier.hpp"
#include "profile.hpp"

using namespace Opera;

struct ProfileBarrier : public Profiler {

    ProfileBarrier() : Profiler(100000) { }

    void run() {
        profile_apply_to_sequence_section();
        profile_sequence_section_reuse_index();
        profile_sequence_section_reuse_or_not();
    }

    void profile_apply_to_sequence_section() {
        Robot r("r0", 10, {{0, 1}}, {1.0});
        Human h("h0", {{0, 1}}, {1.0});
        auto hs = h.segment(0).create_sample({Point(0,0,0)},{Point(2,0,0)});

        Mode first({r.id(), "first"});
        auto sequence1 = SphereMinimumDistanceBarrierSequenceSection(hs);
        List<BodySegmentSample> rss;
        for (SizeType i=num_tries(); i>0; --i) {
            rss.push_back(r.segment(0).create_sample({Point(0,FloatType(5+i),0)},{Point(2,FloatType(6+i),0)}));
        }

        profile("Update section with sample (decreasing segment_distance)",[&](SizeType i){
            sequence1.check_and_update(rss.at(i), {0, i}); });

        auto sequence2 = SphereMinimumDistanceBarrierSequenceSection(hs);
        rss.clear();
        for (SizeType i=0; i<num_tries(); ++i) {
            rss.push_back(r.segment(0).create_sample({Point(FloatType(4+i),4,0)},{Point(FloatType(6+i),4,0)}));
        }

        profile("Update section with sample (increasing segment_distance)",[&](SizeType i){
            sequence2.check_and_update(rss.at(i), {0, i}); });
    }

    void profile_sequence_section_reuse_index() {
        const SizeType ns = 1000;
        const SizeType override_num_tries = 100;
        Robot r("r0", 10, {{0, 1}}, {1.0});
        Human h("h0", {{0, 1}}, {0.5});
        auto hs = h.segment(0).create_sample({Point(FloatType(ns),FloatType(ns),0)},{Point(FloatType(ns+2),FloatType(ns),0)});

        Mode first({r.id(), "first"});
        List<SphereMinimumDistanceBarrierSequenceSection> ts;
        List<BodySegmentSample> beginning_hs, middle_hs, end_hs;
        for (SizeType i=0; i<override_num_tries; ++i) {
            auto section = SphereMinimumDistanceBarrierSequenceSection(hs);
            Point last_head(0,0,0);
            for (SizeType j=0; j<ns;++j) {
                Point new_head(last_head.x+rnd().get(0.99,1.01),last_head.y+rnd().get(0.99,1.01),0.0);
                Point new_tail(new_head.x+2,new_head.y,new_head.z);
                section.check_and_update(r.segment(0).create_sample({new_head}, {new_tail}), {0, j});
                last_head = new_head;
            }
            ts.push_back(section);

            Point beginning_head_sa(rnd().get(0.9,1.1),rnd().get(0.9,1.1),0.0);
            Point beginning_tail_sa(beginning_head_sa.x+2,beginning_head_sa.y,beginning_head_sa.z);
            beginning_hs.push_back(
                    h.segment(0).create_sample({beginning_head_sa}, {beginning_tail_sa}));

            Point middle_head_sa(FloatType(ns)/2*rnd().get(0.9,1.1),FloatType(ns)/2*rnd().get(0.9,1.1),0.0);
            Point middle_tail_sa(middle_head_sa.x+2,middle_head_sa.y,middle_head_sa.z);
            middle_hs.push_back(h.segment(0).create_sample({middle_head_sa}, {middle_tail_sa}));

            Point end_head_sa(FloatType(ns)*rnd().get(0.9,1.1),FloatType(ns)*rnd().get(0.9,1.1),0.0);
            Point end_tail_sa(end_head_sa.x+2,end_head_sa.y,end_head_sa.z);
            end_hs.push_back(h.segment(0).create_sample({end_head_sa}, {end_tail_sa}));
        }

        profile("Find reuse element (strictly beginning of sequence)", [&](SizeType i){ ts.at(i)._reuse_element(beginning_hs.at(i)); }, override_num_tries);
        profile("Find reuse element (middle of sequence)", [&](SizeType i){ ts.at(i)._reuse_element(middle_hs.at(i)); }, override_num_tries);
        profile("Find reuse element (near end of sequence)", [&](SizeType i){ ts.at(i)._reuse_element(end_hs.at(i)); }, override_num_tries);
    }

    void profile_sequence_section_reuse_or_not() {
        const SizeType ns = 200;
        const SizeType override_num_tries = 1;
        Robot r("r0", 10, {{0, 1}}, {1.0});
        Human h("h0", {{0, 1}}, {0.5});
        auto hs = h.segment(0).create_sample({Point(ns,ns,0)},{Point(ns+2,ns,0)});

        Mode first({r.id(), "first"});
        Mode second({r.id(), "second"});
        auto section = SphereMinimumDistanceBarrierSequenceSection(hs);
        RobotStateHistory history(r);
        for (SizeType i=0; i<ns; ++i) {
            history.acquire(first,{{Point(FloatType(i),0,0)},{Point(FloatType(i),2,0)}},static_cast<TimestampType>(i*100));
        }
        history.acquire(second,{{Point(FloatType(ns),0,0)},{Point(FloatType(ns),2,0)}},static_cast<TimestampType>(ns*100));

        List<BodySegmentSample> hss;
        for (SizeType i=ns; i>0; --i) {
            hss.push_back(h.segment(0).create_sample({Point(FloatType(ns+i),0,0)},{Point(FloatType(ns+i),2,0)}));
        }

        auto const& samples = history.snapshot_at(ns*100).samples(first).at(0);

        profile("Using resuming for segments intersection detection",[&](auto){
            SizeType reuse_idx = 0;
            MinimumDistanceBarrierSequenceSection tr = SphereMinimumDistanceBarrierSequenceSection(hss.at(0));
            for (SizeType i=0; i<ns; ++i) {
                bool update_section = true;
                for (SizeType j=reuse_idx; j<ns; ++j) {
                    if (update_section and not tr.check_and_update(samples.at(j), {0, j}))
                        update_section = false;
                    if (hss.at(i).intersects(samples.at(j))) break;
                }
                if (i<ns-1) {
                    tr.reset(hss.at(i + 1), {0,0}, i);
                    reuse_idx = tr.last_barrier().range().maximum_sample_index();
                }
            }
        },override_num_tries);

        profile("Not using resuming for segments intersection detection",[&](auto){
            for (SizeType i=0; i<ns; ++i) {
                for (SizeType j=i; j<ns; ++j) {
                    if (hss.at(i).intersects(samples.at(j))) break;
                }
            }
        },override_num_tries);
    }
};

int main() {
    ProfileBarrier().run();
}
