/***************************************************************************
 *            broker.hpp
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

#ifndef OPERA_BROKER_HPP
#define OPERA_BROKER_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <functional>

#include "handle.hpp"
#include "message.hpp"
#include "topic.hpp"

namespace Opera {

template<class T> using CallbackFunction = std::function<void(T const&)>;

//! \brief Struct for holding context for a callback, including data for proper registration of the callback thread (otherwise inaccessible)
template<class T> struct CallbackContext {
    CallbackContext(CallbackFunction<T> f, int pll, std::string ptn) :
            function(f), parent_logger_level(pll), parent_thread_name(ptn), thread_id(std::this_thread::get_id()), registered(false) { }
    CallbackFunction<T> function;
    int parent_logger_level;
    std::string parent_thread_name;
    std::thread::id thread_id;
    bool registered;
};

//! \brief An interface for publishing objects
template<class T> class PublisherInterface {
  public:
    //! \brief Publish the \a obj
    virtual void put(T const& obj) = 0;
    //! \brief Default destructor to avoid destructor not being called on objects of this type
    virtual ~PublisherInterface() = default;
};

//! \brief An interface for subscribing to objects published
template<class T> class SubscriberInterface {
  public:
    //! \brief Default destructor to avoid destructor not being called on objects of this type
    virtual ~SubscriberInterface() = default;
};

//! \brief Interface for access to a communication broker
class BrokerAccessInterface {
  public:
    virtual PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher(BodyPresentationTopic const& topic) const = 0;
    virtual PublisherInterface<HumanStateMessage>* make_human_state_publisher(HumanStateTopic const& topic) const = 0;
    virtual PublisherInterface<RobotStateMessage>* make_robot_state_publisher(RobotStateTopic const& topic) const = 0;
    virtual PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher(CollisionNotificationTopic const& topic) const = 0;

    virtual SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic) const = 0;
    virtual SubscriberInterface<HumanStateMessage>* make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic) const = 0;
    virtual SubscriberInterface<RobotStateMessage>* make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic) const = 0;
    virtual SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic) const = 0;

    //! \brief Default destructor to avoid destructor not being called on objects of this type
    virtual ~BrokerAccessInterface() = default;
};

//! \brief Handle for a broker access
class BrokerAccess : public Handle<BrokerAccessInterface> {
  public:
    using Handle<BrokerAccessInterface>::Handle;
    PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher(BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const { return _ptr->make_body_presentation_publisher(topic); }
    PublisherInterface<HumanStateMessage>* make_human_state_publisher(HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const { return _ptr->make_human_state_publisher(topic); }
    PublisherInterface<RobotStateMessage>* make_robot_state_publisher(RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const { return _ptr->make_robot_state_publisher(topic); }
    PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher(CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const { return _ptr->make_collision_notification_publisher(topic); }
    SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const { return _ptr->make_body_presentation_subscriber(callback,topic); }
    SubscriberInterface<HumanStateMessage>* make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const { return _ptr->make_human_state_subscriber(callback,topic); }
    SubscriberInterface<RobotStateMessage>* make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const { return _ptr->make_robot_state_subscriber(callback,topic); }
    SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const { return _ptr->make_collision_notification_subscriber(callback,topic); }
};

}

#endif // OPERA_BROKER_HPP