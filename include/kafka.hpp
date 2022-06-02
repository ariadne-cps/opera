/***************************************************************************
 *            kafka.hpp
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

#ifndef OPERA_KAFKA_HPP
#define OPERA_KAFKA_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <tuple>
#include <thread>
#include <librdkafka/rdkafkacpp.h>

#include "broker.hpp"
#include "serialisation.hpp"
#include "deserialisation.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

//! \brief The publisher of objects to the Kafka broker
template<class T> class KafkaPublisher : public PublisherInterface<T> {
  public:
    KafkaPublisher(std::string const& topic, std::string const& hostname, int port) : _topic(topic) {
    }

    void put(T const& obj) override {

    }

    ~KafkaPublisher() {
    }

  private:
    std::string const _topic;
};

//! \brief The subscriber to objects published to Kafka
template<class T> class KafkaSubscriber : public SubscriberInterface<T> {
  public:
    //! \brief Connects and starts the main asynchronous loop for getting messages
    KafkaSubscriber(std::string const& topic, std::string const& hostname, int port, CallbackFunction<T> const& callback)
        : _topic(topic), _hostname(hostname), _port(port), _callback(callback)
    {

    }

    virtual ~KafkaSubscriber() {

    }

  protected:
    std::string const _topic;
    std::string const _hostname;
    int const _port;
    CallbackFunction<T> const _callback;
};

//! \brief Access to a broker to handle Opera-specific messages using Kafka
class KafkaBrokerAccess : public BrokerAccessInterface {
  public:
    KafkaBrokerAccess(std::string const& hostname, int port);
    PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher(BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    PublisherInterface<HumanStateMessage>* make_human_state_publisher(HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    PublisherInterface<RobotStateMessage>* make_robot_state_publisher(RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher(CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;
    SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    SubscriberInterface<HumanStateMessage>* make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    SubscriberInterface<RobotStateMessage>* make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;
    ~KafkaBrokerAccess();

  private:
    std::string const _hostname;
    int const _port;
};

}

#endif // OPERA_KAFKA_HPP
