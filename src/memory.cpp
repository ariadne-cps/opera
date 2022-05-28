/***************************************************************************
 *            memory.cpp
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

#include "memory.tpl.hpp"

namespace Opera {

void MemoryBroker::clear() {
    _body_presentations.clear();
    _human_states.clear();
    _robot_states.clear();
    _collision_notifications.clear();
}

PublisherInterface<BodyPresentationMessage>* MemoryBrokerAccess::make_body_presentation_publisher(BodyPresentationTopic const&) const {
    return new MemoryPublisher<BodyPresentationMessage>();
}

PublisherInterface<HumanStateMessage>* MemoryBrokerAccess::make_human_state_publisher(HumanStateTopic const&) const {
    return new MemoryPublisher<HumanStateMessage>();
}

PublisherInterface<RobotStateMessage>* MemoryBrokerAccess::make_robot_state_publisher(RobotStateTopic const&) const {
    return new MemoryPublisher<RobotStateMessage>();
}

PublisherInterface<CollisionNotificationMessage>* MemoryBrokerAccess::make_collision_notification_publisher(CollisionNotificationTopic const&) const {
    return new MemoryPublisher<CollisionNotificationMessage>();
}

SubscriberInterface<BodyPresentationMessage>* MemoryBrokerAccess::make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const&) const {
    return new MemorySubscriber<BodyPresentationMessage>(callback);
}

SubscriberInterface<HumanStateMessage>* MemoryBrokerAccess::make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const&) const {
    return new MemorySubscriber<HumanStateMessage>(callback);
}

SubscriberInterface<RobotStateMessage>* MemoryBrokerAccess::make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const&) const {
    return new MemorySubscriber<RobotStateMessage>(callback);
}

SubscriberInterface<CollisionNotificationMessage>* MemoryBrokerAccess::make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const&) const {
    return new MemorySubscriber<CollisionNotificationMessage>(callback);
}

}