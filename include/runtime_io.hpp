/***************************************************************************
 *            runtime_io.hpp
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

#ifndef OPERA_RUNTIME_IO_HPP
#define OPERA_RUNTIME_IO_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <tuple>
#include <thread>
#include "thread.hpp"
#include "macros.hpp"

#include "broker.hpp"
#include "body_registry.hpp"
#include "lookahead_job_factory.hpp"
#include "synchronised_queue.hpp"

namespace Opera {

//! \brief Utility class to receive messages and apply them
class RuntimeReceiver {
    struct HumanRobotIdPair {
        HumanRobotIdPair(BodyIdType const& h, BodyIdType const& r) : human(h), robot(r) { }
        BodyIdType human;
        BodyIdType robot;
    };
  public:
    //! \brief Create starting from subscribers and a \a registry to fill, along with \a waiting_jobs
    //! to populate as soon as the registry has history for the corresponding human-robot pair, and \a sleeping_jobs
    //! to move to waiting_jobs as soon as a new human state is received
    RuntimeReceiver(Pair<BrokerAccess,BodyPresentationTopic> const& bp_subscriber, Pair<BrokerAccess,HumanStateTopic> const& hs_subscriber,
                    Pair<BrokerAccess,RobotStateTopic> const& rs_subscriber,
                    LookAheadJobFactory const& factory, BodyRegistry& registry,
                    SynchronisedQueue<LookAheadJob>& waiting_jobs, SynchronisedQueue<LookAheadJob>& sleeping_jobs);

    //! \brief The current number of created human-robot pairs, not yet put into the waiting jobs
    SizeType num_pending_human_robot_pairs() const;

    //! \brief Return the factory
    LookAheadJobFactory const& factory() const { return _factory; }

    //! \brief [TEST] Get the number of state messages received
    SizeType __num_state_messages_received() const { return _num_state_messages_received; }

    //! \brief Destroys subscribers
    ~RuntimeReceiver() noexcept;

  private:
    //! \brief Possibly move any human-robot pair into a mix of sleeping jobs (if the specific human sample is empty) or waiting jobs (otherwise)
    void _promote_pairs_to_jobs(BodyRegistry const& registry, SynchronisedQueue<LookAheadJob>& sleeping_jobs, SynchronisedQueue<LookAheadJob>& waiting_jobs);
    //! \brief Move sleeping jobs to waiting jobs or discard them, as a result of a new human state
    void _move_sleeping_jobs_to_waiting_jobs(BodyRegistry const& registry, SynchronisedQueue<LookAheadJob>& sleeping_jobs, SynchronisedQueue<LookAheadJob>& waiting_jobs);

  private:
    List<HumanRobotIdPair> _pending_human_robot_pairs;
    mutable std::mutex _pairs_mux;
    mutable std::mutex _state_received_mux;

    LookAheadJobFactory const& _factory;

    SubscriberInterface<BodyPresentationMessage>* _bp_subscriber;
    SubscriberInterface<HumanStateMessage>* _hs_subscriber;
    SubscriberInterface<RobotStateMessage>* _rs_subscriber;

    std::atomic<SizeType> _num_state_messages_received = 0;
};

//! \brief Utility class to send messages from the runtime
//! \details Currently only CollisionNotificationMessage is applicable
class RuntimeSender {
  public:
    //! \brief Create starting from a publisher
    explicit RuntimeSender(Pair<BrokerAccess,CollisionNotificationTopic> const& publisher);

    //! \brief Put a message to be sent
    template<class T> void put(T const& msg) { _put(msg); _availability_condition.notify_one(); }

    //! \brief Stop waiting for messages
    ~RuntimeSender() noexcept;

  private:
    template<class T> void _put(T const& msg);

  private:
    SynchronisedQueue<CollisionNotificationMessage> _collision_notifications;
    std::condition_variable _availability_condition;
    std::mutex _availability_mutex;
    PublisherInterface<CollisionNotificationMessage>* _publisher;
    bool _stop;
    Thread _thr;
};

}

#endif // OPERA_RUNTIME_IO_HPP