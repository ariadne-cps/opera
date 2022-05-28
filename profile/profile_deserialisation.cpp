/***************************************************************************
 *            profile_deserialisation.cpp
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

using namespace Opera;

class ProfileDeserialisation : public Profiler {
  public:
    ProfileDeserialisation() : Profiler(10000) { }

    void run() {
        profile_humanstatemessage();
        profile_robotstatemessage();
        profile_bodypresentationmessage();
        profile_collisionnotificationmessage();
    }

    void profile_humanstatemessage() {
        profile("Deserialisation of a human sample JSON file into HumanStateMessage",[&](auto){
            Deserialiser<HumanStateMessage>(Resources::path("json/examples/state/humans.json")).make();
        });

        List<std::string> json_texts;
        profile("Deserialisation of a human sample JSON file into String",[&](auto){
            json_texts.push_back(Deserialiser<HumanStateMessage>(Resources::path("json/examples/state/humans.json")).to_string());
        });

        profile("Deserialisation of a human sample JSON String into HumanStateMessage",[&](SizeType i){
            Deserialiser<HumanStateMessage>(json_texts.at(i).c_str()).make();
        });
    }

    void profile_robotstatemessage() {
        profile("Deserialisation of a robot sample JSON file into RobotStateMessage",[&](auto){
            Deserialiser<RobotStateMessage>(Resources::path("json/examples/state/robot0.json")).make();
        });

        List<std::string> json_texts;
        profile("Deserialisation of a robot sample JSON file into String",[&](auto){
            json_texts.push_back(Deserialiser<RobotStateMessage>(Resources::path("json/examples/state/robot0.json")).to_string());
        });

        profile("Deserialisation of a robot sample JSON String into RobotStateMessage",[&](SizeType i){
            Deserialiser<RobotStateMessage>(json_texts.at(i).c_str()).make();
        });
    }

    void profile_bodypresentationmessage() {
        profile("Deserialisation of a robot presentation JSON file into BodyPresentationMessage",[&](auto){
            Deserialiser<BodyPresentationMessage>(Resources::path("json/examples/presentation/robot0.json")).make();
            });

        List<std::string> json_texts;
        profile("Deserialisation of a robot presentation JSON file into String",[&](auto){
            json_texts.push_back(Deserialiser<BodyPresentationMessage>(Resources::path("json/examples/presentation/robot0.json")).to_string());
            });

        profile("Deserialisation of a robot presentation JSON String into BodyPresentationMessage",[&](SizeType i){
            Deserialiser<BodyPresentationMessage>(json_texts.at(i).c_str()).make();
            });
    }

    void profile_collisionnotificationmessage() {
        profile("Deserialisation of a collision notification JSON file into CollisionNotificationMessage",[&](auto){
            Deserialiser<CollisionNotificationMessage>(Resources::path("json/examples/notification/notification0.json")).make();
            });

        List<std::string> json_texts;
        profile("Deserialisation of a collision notification JSON file into String",[&](auto){
            json_texts.push_back(Deserialiser<CollisionNotificationMessage>(Resources::path("json/examples/notification/notification0.json")).to_string());
            });

        profile("Deserialisation of a collision notification JSON String into CollisionNotificationMessage",[&](SizeType i){
            Deserialiser<CollisionNotificationMessage>(json_texts.at(i).c_str()).make();
            });
    }

};

int main() {
    ProfileDeserialisation().run();
}
