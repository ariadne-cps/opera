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

#include "broker_access.hpp"
#include "serialisation.hpp"
#include "deserialisation.hpp"
#include "thread.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

//! \brief The publisher of objects to the Kafka broker
template<class T> class KafkaPublisher : public PublisherInterface<T> {
  public:
    KafkaPublisher(std::string const& topic, std::string const& brokers, std::string const& sasl_mechanism, std::string const& security_protocol,
                   std::string const& sasl_username, std::string const& sasl_password) : _topic(topic) {
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

        std::string errstr;

        conf->set("metadata.broker.list", brokers, errstr);
        conf->set("sasl.mechanism", sasl_mechanism, errstr);
        conf->set("security.protocol", security_protocol, errstr);
        conf->set("sasl.username", sasl_username, errstr);
        conf->set("sasl.password", sasl_password, errstr);

        _producer = RdKafka::Producer::create(conf, errstr);
        OPERA_ASSERT_MSG(_producer,"Failed to create producer: " << errstr)
    }

    void put(T const& obj) override {
        auto s_obj = Serialiser<T>(obj).to_string();
        auto resp = _producer->produce(_topic, 0, RdKafka::Producer::RK_MSG_COPY, const_cast<char *>(s_obj.c_str()), s_obj.size(),NULL, 0, 0, NULL);
        OPERA_ASSERT_MSG(resp == RdKafka::ErrorCode::ERR_NO_ERROR,"Failed to publish: " << RdKafka::err2str(resp))
    }

    ~KafkaPublisher() {
        _producer->flush(10000);
        delete _producer;
    }

  private:
    std::string const _topic;
    RdKafka::Producer* _producer;
};

//! \brief The subscriber to objects published to Kafka
template<class T> class KafkaSubscriber : public SubscriberInterface<T> {
  public:
    //! \brief Connects and starts the main asynchronous loop for getting messages
    KafkaSubscriber(std::string const& topic, int partition, int64_t start_offset, CallbackFunction<T> const& callback,
                    std::string const& brokers, std::string const& sasl_mechanism, std::string const& security_protocol,
                    std::string const& sasl_username, std::string const& sasl_password)
        : _stopped(false), _partition(partition)
    {
        RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

        std::string errstr;

        conf->set("metadata.broker.list", brokers, errstr);
        conf->set("sasl.mechanism", sasl_mechanism, errstr);
        conf->set("security.protocol", security_protocol, errstr);
        conf->set("sasl.username", sasl_username, errstr);
        conf->set("sasl.password", sasl_password, errstr);

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

class KafkaBrokerAccess;

//! \brief A builder class for the Kafka broker access,
//! for checking that all fields are properly consistent
class KafkaBrokerAccessBuilder {
  public:
    //! \brief Construct with the minimum necessary info
    KafkaBrokerAccessBuilder(std::string const& brokers);

    //! \brief Set partition (default: 0)
    KafkaBrokerAccessBuilder& set_partition(int partition);
    //! \brief Set start offset (default: RdKafka::Topic::OFFSET_END)
    KafkaBrokerAccessBuilder& set_start_offset(int64_t start_offset);
    //! \brief Set a prefix for topics with respect to the provided value when making a publisher or subscriber
    KafkaBrokerAccessBuilder& set_topic_prefix(std::string const& topic_prefix);
    //! \brief Set the SASL mechanism
    KafkaBrokerAccessBuilder& set_sasl_mechanism(std::string const& sasl_mechanism);
    //! \brief Set the security protocol
    KafkaBrokerAccessBuilder& set_security_protocol(std::string const& security_protocol);
    //! \brief Set the SASL username
    KafkaBrokerAccessBuilder& set_sasl_username(std::string const& sasl_username);
    //! \brief Set the SASL password
    KafkaBrokerAccessBuilder& set_sasl_password(std::string const& sasl_password);

    //! \brief Build the object
    KafkaBrokerAccess build() const;

  private:
    std::string const _brokers;
    int _partition;
    int64_t _start_offset;
    std::string _topic_prefix;
    std::string _sasl_mechanism;
    std::string _security_protocol;
    std::string _sasl_username;
    std::string _sasl_password;
};

//! \brief Access to a broker to handle Opera-specific messages using Kafka
class KafkaBrokerAccess : public BrokerAccessInterface {
    friend class KafkaBrokerAccessBuilder;
  protected:
    KafkaBrokerAccess(std::string const& brokers, int partition, int64_t start_offset, std::string const& topic_prefix,
                      std::string const& sasl_mechanism, std::string const& security_protocol, std::string const& sasl_username, std::string const& sasl_password);
  public:
    PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher(BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    PublisherInterface<HumanStateMessage>* make_human_state_publisher(HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    PublisherInterface<RobotStateMessage>* make_robot_state_publisher(RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher(CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;
    SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    SubscriberInterface<HumanStateMessage>* make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    SubscriberInterface<RobotStateMessage>* make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;

  private:
    std::string const _brokers;
    int const _partition;
    int64_t const _start_offset;
    std::string const _topic_prefix;
    std::string const _sasl_mechanism;
    std::string const _security_protocol;
    std::string const _sasl_username;
    std::string const _sasl_password;
};

}

#endif // OPERA_KAFKA_HPP
