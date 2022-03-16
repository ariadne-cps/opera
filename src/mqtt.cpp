/***************************************************************************
 *            mqtt.cpp
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

#include "mqtt.hpp"

namespace Opera {

const std::string MqttBrokerAccess::BODY_PRESENTATION_TOPIC = "opera/body-presentation";
const std::string MqttBrokerAccess::BODY_STATE_TOPIC = "opera/body-state";
const std::string MqttBrokerAccess::COLLISION_NOTIFICATION_TOPIC = "opera/collision-notification";

MqttBrokerAccess::MqttBrokerAccess(std::string const& hostname, int port) : _hostname(hostname), _port(port) { }

MqttBrokerAccess::~MqttBrokerAccess() {
    mosquitto_lib_cleanup();
}

PublisherInterface<BodyPresentationMessage>* MqttBrokerAccess::make_body_presentation_publisher() const {
    return new MqttPublisher<BodyPresentationMessage>(BODY_PRESENTATION_TOPIC, _hostname, _port);
}

PublisherInterface<BodyStateMessage>* MqttBrokerAccess::make_body_state_publisher() const {
    return new MqttPublisher<BodyStateMessage>(BODY_STATE_TOPIC, _hostname, _port);
}

PublisherInterface<CollisionNotificationMessage>* MqttBrokerAccess::make_collision_notification_publisher() const {
    return new MqttPublisher<CollisionNotificationMessage>(COLLISION_NOTIFICATION_TOPIC, _hostname, _port);
}

SubscriberInterface<BodyPresentationMessage>* MqttBrokerAccess::make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback) const {
    return new MqttSubscriber<BodyPresentationMessage>(BODY_PRESENTATION_TOPIC, _hostname, _port, callback);
}

SubscriberInterface<BodyStateMessage>* MqttBrokerAccess::make_body_state_subscriber(CallbackFunction<BodyStateMessage> const& callback) const {
    return new MqttSubscriber<BodyStateMessage>(BODY_STATE_TOPIC, _hostname, _port, callback);
}

SubscriberInterface<CollisionNotificationMessage>* MqttBrokerAccess::make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback) const {
    return new MqttSubscriber<CollisionNotificationMessage>(COLLISION_NOTIFICATION_TOPIC, _hostname, _port, callback);
}

}
