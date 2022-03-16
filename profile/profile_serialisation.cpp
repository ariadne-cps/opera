/***************************************************************************
 *            profile_serialisation.cpp
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

#include "profile.hpp"

#include "utility.hpp"
#include "deserialisation.hpp"
#include "serialisation.hpp"

using namespace Opera;

class ProfileSerialisation : public Profiler {
  public:
    ProfileSerialisation() : Profiler(100000) { }

    void run() {
        profile_bodypresentationmessage();
        profile_bodystatemessage();
        profile_collisionnotificationmessage();
    }

    void profile_bodypresentationmessage() {
        SizeType num_samples = 1000;
        List<BodyPresentationMessage> messages;
        for (SizeType i=0; i<num_tries(); ++i)
            messages.push_back(Deserialiser<BodyPresentationMessage>(Resources::path("json/examples/presentation/human0.json")).make());

        profile("Serialisation of a BodyPresentationMessage into a human presentation JSON String",[&](SizeType i){
            Serialiser<BodyPresentationMessage>(messages.at(i)).to_string();
            },num_samples);
    }

    void profile_bodystatemessage() {
        SizeType num_samples = 4259;

        List<BodyStateMessage> messages;
        for (SizeType i=0; i<num_samples; ++i)
            messages.push_back(Deserialiser<BodyStateMessage>(Resources::path("json/scenarios/nocollision/h0/"+std::to_string(i+1)+".json")).make());

        profile("Serialisation of a BodyStateMessage into a human sample JSON String",[&](SizeType i){
            Serialiser<BodyStateMessage>(messages.at(i)).to_string();
        },num_samples);
    }

    void profile_collisionnotificationmessage() {
        SizeType num_samples = 1000;
        List<CollisionNotificationMessage> messages;
        for (SizeType i=0; i<num_tries(); ++i)
            messages.push_back(Deserialiser<CollisionNotificationMessage>(Resources::path("json/examples/notification/notification0.json")).make());

        profile("Serialisation of a CollisionNotificationMessage into a human presentation JSON String",[&](SizeType i){
            Serialiser<CollisionNotificationMessage>(messages.at(i)).to_string();
            },num_samples);
    }
};

int main() {
    ProfileSerialisation().run();
}
