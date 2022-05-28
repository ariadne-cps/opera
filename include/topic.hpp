/***************************************************************************
 *            topic.hpp
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

#ifndef OPERA_TOPIC_HPP
#define OPERA_TOPIC_HPP

#include <string>

namespace Opera {

//! \brief Topic for body presentation
struct BodyPresentationTopic : std::string {
    BodyPresentationTopic(std::string str) : std::string(str) { }
    static const BodyPresentationTopic DEFAULT;
};

//! \brief Topic for human state
struct HumanStateTopic : std::string {
    HumanStateTopic(std::string str) : std::string(str) { }
    static const HumanStateTopic DEFAULT;
};

//! \brief Topic for robot state
struct RobotStateTopic : std::string {
    RobotStateTopic(std::string str) : std::string(str) { }
    static const RobotStateTopic DEFAULT;
};

//! \brief Topic for collision notification
struct CollisionNotificationTopic : std::string {
    CollisionNotificationTopic(std::string str) : std::string(str) { }
    static const CollisionNotificationTopic DEFAULT;
};

}

#endif //OPERA_TOPIC_HPP
