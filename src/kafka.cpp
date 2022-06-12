/***************************************************************************
 *            kafka.cpp
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

#include "kafka.hpp"

namespace Opera {

KafkaBrokerAccessBuilder::KafkaBrokerAccessBuilder(std::string const& brokers) : _brokers(brokers), _partition(0), _start_offset(RdKafka::Topic::OFFSET_END) { }

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_partition(int partition) {
    _partition = partition;
    return *this;
}

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_start_offset(int64_t start_offset) {
    _start_offset = start_offset;
    return *this;
}

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_topic_prefix(std::string const& topic_prefix) {
    _topic_prefix = topic_prefix;
    return *this;
}

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_sasl_mechanism(std::string const& sasl_mechanism) {
    _sasl_mechanism = sasl_mechanism;
    return *this;
}

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_security_protocol(std::string const& security_protocol) {
    _security_protocol = security_protocol;
    return *this;
}

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_sasl_username(std::string const& sasl_username) {
    _sasl_username = sasl_username;
    return *this;
}

KafkaBrokerAccessBuilder& KafkaBrokerAccessBuilder::set_sasl_password(std::string const& sasl_password) {
    _sasl_password = sasl_password;
    return *this;
}

KafkaBrokerAccess KafkaBrokerAccessBuilder::build() const {
    if (_sasl_mechanism != "" or _security_protocol != "" or _sasl_username != "" or _sasl_password != "") {
        OPERA_ASSERT(_sasl_mechanism != "" and _security_protocol != "" and _sasl_username != "" and _sasl_password != "");
    }
    return KafkaBrokerAccess(_brokers,_partition,_start_offset,_topic_prefix,_sasl_mechanism,_security_protocol,_sasl_username,_sasl_password);
}

KafkaBrokerAccess::KafkaBrokerAccess(std::string const& brokers, int partition, int64_t start_offset, std::string const& topic_prefix,
                                     std::string const& sasl_mechanism, std::string const& security_protocol,
                                     std::string const& sasl_username, std::string const& sasl_password) :
                                     _brokers(brokers), _partition(partition), _start_offset(start_offset), _topic_prefix(topic_prefix),
                                     _sasl_mechanism(sasl_mechanism), _security_protocol(security_protocol),
                                     _sasl_username(sasl_username), _sasl_password(sasl_password) { }

PublisherInterface<BodyPresentationMessage>* KafkaBrokerAccess::make_body_presentation_publisher(BodyPresentationTopic const& topic) const {
    return new KafkaPublisher<BodyPresentationMessage>(_topic_prefix+topic, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

PublisherInterface<HumanStateMessage>* KafkaBrokerAccess::make_human_state_publisher(HumanStateTopic const& topic) const {
    return new KafkaPublisher<HumanStateMessage>(_topic_prefix+topic, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

PublisherInterface<RobotStateMessage>* KafkaBrokerAccess::make_robot_state_publisher(RobotStateTopic const& topic) const {
    return new KafkaPublisher<RobotStateMessage>(_topic_prefix+topic, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

PublisherInterface<CollisionNotificationMessage>* KafkaBrokerAccess::make_collision_notification_publisher(CollisionNotificationTopic const& topic) const {
    return new KafkaPublisher<CollisionNotificationMessage>(_topic_prefix+topic, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

SubscriberInterface<BodyPresentationMessage>* KafkaBrokerAccess::make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic) const {
    return new KafkaSubscriber<BodyPresentationMessage>(_topic_prefix+topic, _partition, _start_offset, callback, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

SubscriberInterface<HumanStateMessage>* KafkaBrokerAccess::make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic) const {
    return new KafkaSubscriber<HumanStateMessage>(_topic_prefix+topic, _partition, _start_offset, callback, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

SubscriberInterface<RobotStateMessage>* KafkaBrokerAccess::make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic) const {
    return new KafkaSubscriber<RobotStateMessage>(_topic_prefix+topic, _partition, _start_offset, callback, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

SubscriberInterface<CollisionNotificationMessage>* KafkaBrokerAccess::make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic) const {
    return new KafkaSubscriber<CollisionNotificationMessage>(_topic_prefix+topic, _partition, _start_offset, callback, _brokers, _sasl_mechanism, _security_protocol, _sasl_username, _sasl_password);
}

}
