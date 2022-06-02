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
#include "thread.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

//! \brief The publisher of objects to the Kafka broker
template<class T> class KafkaPublisher : public PublisherInterface<T> {
  public:
    KafkaPublisher(std::string const& topic, std::string const& brokers) : _topic(topic), _brokers(brokers) {
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        std::string errstr;
        conf->set("metadata.broker.list", brokers, errstr);
        _producer = RdKafka::Producer::create(conf, errstr);
        OPERA_ASSERT_MSG(_producer,"Failed to create producer: " << errstr)
    }

    void put(T const& obj) override {
        auto s_obj = Serialiser<T>(obj).to_string();
        auto resp = _producer->produce(_topic, 0, RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(s_obj.c_str()), s_obj.size(),NULL, 0, 0, NULL);
        OPERA_ASSERT_MSG(resp == RdKafka::ErrorCode::ERR_NO_ERROR,"Failed to publish: " << RdKafka::err2str(resp))
    }

    ~KafkaPublisher() {
        delete _producer;
    }

  private:
    std::string const _topic;
    std::string const _brokers;
    RdKafka::Producer* _producer;
};

//! \brief The subscriber to objects published to Kafka
template<class T> class KafkaSubscriber : public SubscriberInterface<T> {
  public:
    //! \brief Connects and starts the main asynchronous loop for getting messages
    KafkaSubscriber(std::string const& topic, int partition, std::string const& brokers, int start_offset, CallbackFunction<T> const& callback)
        : _stopped(false), _partition(partition)
    {
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

        std::string errstr;

        conf->set("metadata.broker.list", brokers, errstr);

        _consumer = RdKafka::Consumer::create(conf, errstr);
        OPERA_ASSERT_MSG(_consumer, "Failed to create consumer: " << errstr)

        _topic = RdKafka::Topic::create(_consumer, topic, tconf, errstr);
        OPERA_ASSERT_MSG(_topic,"Failed to create topic: " << errstr)

        RdKafka::ErrorCode resp = _consumer->start(_topic, partition, start_offset);
        OPERA_ASSERT_MSG(resp == RdKafka::ERR_NO_ERROR,"Failed to start consumer: " << RdKafka::err2str(resp))

        _thr = new Thread([=,this]{
            int timeout_ms = 1;
            while (not _stopped) {
                RdKafka::Message* message = _consumer->consume(_topic,_partition,timeout_ms);
                if (message->err() == RdKafka::ERR_NO_ERROR) {
                    Deserialiser<T> deserialiser(std::string((char *)message->payload(),message->len()).c_str());
                    callback(deserialiser.make());
                }
                delete message;
            }
        }, "cnsm");
    }

    virtual ~KafkaSubscriber() {
        _stopped = true;
        delete _thr;
        _consumer->stop(_topic, _partition);
        delete _topic;
        delete _consumer;
    }

  protected:
    bool _stopped;
    Thread* _thr;
    int const _partition;
    RdKafka::Consumer* _consumer;
    RdKafka::Topic* _topic;
};

//! \brief Access to a broker to handle Opera-specific messages using Kafka
class KafkaBrokerAccess : public BrokerAccessInterface {
  public:
    KafkaBrokerAccess(int partition, std::string brokers, int start_offset);
    PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher(BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    PublisherInterface<HumanStateMessage>* make_human_state_publisher(HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    PublisherInterface<RobotStateMessage>* make_robot_state_publisher(RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher(CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;
    SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    SubscriberInterface<HumanStateMessage>* make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    SubscriberInterface<RobotStateMessage>* make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;

  private:
    int const _partition;
    std::string const _brokers;
    int const _start_offset;
};

}

#endif // OPERA_KAFKA_HPP
