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

namespace Opera {

template<class T> using CallbackFunction = std::function<void(T const&)>;

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
    virtual PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher() const = 0;
    virtual PublisherInterface<BodyStateMessage>* make_body_state_publisher() const = 0;
    virtual PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher() const = 0;

    virtual SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback) const = 0;
    virtual SubscriberInterface<BodyStateMessage>* make_body_state_subscriber(CallbackFunction<BodyStateMessage> const& callback) const = 0;
    virtual SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback) const = 0;

    //! \brief Default destructor to avoid destructor not being called on objects of this type
    virtual ~BrokerAccessInterface() = default;
};

//! \brief Handle for a broker access
class BrokerAccess : public Handle<BrokerAccessInterface> {
  public:
    using Handle<BrokerAccessInterface>::Handle;
    PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher() const { return _ptr->make_body_presentation_publisher(); }
    PublisherInterface<BodyStateMessage>* make_body_state_publisher() const { return _ptr->make_body_state_publisher(); }
    PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher() const { return _ptr->make_collision_notification_publisher(); }
    SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback) const { return _ptr->make_body_presentation_subscriber(callback); }
    SubscriberInterface<BodyStateMessage>* make_body_state_subscriber(CallbackFunction<BodyStateMessage> const& callback) const { return _ptr->make_body_state_subscriber(callback); }
    SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback) const { return _ptr->make_collision_notification_subscriber(callback); }
};

}

#endif // OPERA_BROKER_HPP