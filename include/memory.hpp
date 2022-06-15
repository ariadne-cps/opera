/***************************************************************************
 *            memory.hpp
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

#ifndef OPERA_MEMORY_HPP
#define OPERA_MEMORY_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <tuple>
#include <thread>
#include "thread.hpp"

#include "broker_access.hpp"

namespace Opera {

//! \brief A static class to hold messages synchronously using memory
//! \details Messages are accumulated indefinitely
class MemoryBroker {
  private:
    MemoryBroker() = default;
  public:
    MemoryBroker(MemoryBroker const&) = delete;
    void operator=(MemoryBroker const&) = delete;

    //! \brief The singleton instance of this class
    static MemoryBroker& instance() {
        static MemoryBroker instance;
        return instance;
    }

    //! \brief Put a message in memory
    template<class T> void put(T const& msg);

    //! \brief Get the \a idx element in the template argument list
    template<class T> T get(SizeType const& idx) const;

    //! \brief Size of template argument list
    template<class T> SizeType size() const;

    //! \brief Remove all content
    //! \details Useful to clean up between tests
    void clear();

  private:
    List<BodyPresentationMessage> _body_presentations;
    List<HumanStateMessage> _human_states;
    List<RobotStateMessage> _robot_states;
    List<CollisionNotificationMessage> _collision_notifications;
    mutable std::mutex _mux;
};

//! \brief The publisher of objects to memory
template<class T> class MemoryPublisher : public PublisherInterface<T> {
  public:
    void put(T const& obj) override { MemoryBroker::instance().put(obj); }
};

//! \brief The subscriber to objects published to memory
//! \details The advancement of acquisition is local to the subscriber, but for simplicity
//! a new subscriber starts from the beginning of memory content
template<class T> class MemorySubscriber : public SubscriberInterface<T> {
  public:
    //! \brief Constructor
    MemorySubscriber(CallbackFunction<T> const& callback) : _next_index(MemoryBroker::instance().size<T>()), _exit_future(_exit_promise.get_future()), _callback(callback),
                                                            _thr(Thread([&] {
            while (_exit_future.wait_for(std::chrono::microseconds(100)) == std::future_status::timeout) {
                while (MemoryBroker::instance().size<T>() > _next_index) {
                    _callback(MemoryBroker::instance().get<T>(_next_index));
                    _next_index++;
                }
            }
        },"mem_sub")) { }

    ~MemorySubscriber() {
        _exit_promise.set_value();
    }

  protected:
    SizeType _next_index;
    std::promise<void> _exit_promise;
    std::future<void> _exit_future;
    CallbackFunction<T> const _callback;
    Thread const _thr;
};

//! \brief A broker to handle messages using memory
class MemoryBrokerAccess : public BrokerAccessInterface {
  public:
    PublisherInterface<BodyPresentationMessage>* make_body_presentation_publisher(BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    PublisherInterface<HumanStateMessage>* make_human_state_publisher(HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    PublisherInterface<RobotStateMessage>* make_robot_state_publisher(RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    PublisherInterface<CollisionNotificationMessage>* make_collision_notification_publisher(CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;
    SubscriberInterface<BodyPresentationMessage>* make_body_presentation_subscriber(CallbackFunction<BodyPresentationMessage> const& callback, BodyPresentationTopic const& topic = BodyPresentationTopic::DEFAULT) const override;
    SubscriberInterface<HumanStateMessage>* make_human_state_subscriber(CallbackFunction<HumanStateMessage> const& callback, HumanStateTopic const& topic = HumanStateTopic::DEFAULT) const override;
    SubscriberInterface<RobotStateMessage>* make_robot_state_subscriber(CallbackFunction<RobotStateMessage> const& callback, RobotStateTopic const& topic = RobotStateTopic::DEFAULT) const override;
    SubscriberInterface<CollisionNotificationMessage>* make_collision_notification_subscriber(CallbackFunction<CollisionNotificationMessage> const& callback, CollisionNotificationTopic const& topic = CollisionNotificationTopic::DEFAULT) const override;
};

}

#endif // OPERA_MEMORY_HPP