/***************************************************************************
 *            runtime.hpp
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

#ifndef OPERA_RUNTIME_HPP
#define OPERA_RUNTIME_HPP

#include "topic.hpp"
#include "runtime_io.hpp"
#include "lookahead_job_factory.hpp"
#include "synchronised_queue.hpp"

namespace Opera {

//! \brief The configuration of the runtime
class RuntimeConfiguration {
  public:
    //! \brief Default configuration
    RuntimeConfiguration();

    LookAheadJobFactory const& get_job_factory() const;
    TimestampType const& get_history_retention() const;
    TimestampType const& get_history_purge_period() const;
    SizeType const& get_concurrency() const;

    RuntimeConfiguration& set_job_factory(LookAheadJobFactory const& factory);
    RuntimeConfiguration& set_history_retention(TimestampType const& retention);
    RuntimeConfiguration& set_history_purge_period(TimestampType const& purge_period);
    RuntimeConfiguration& set_concurrency(SizeType const& concurrency);

  private:
    //! \brief The factory for look-ahead jobs
    LookAheadJobFactory _job_factory;

    //! \brief The time length (in seconds) for history to remain stored
    //! \details This value is added to history_purge_period to get the actual length
    TimestampType _history_retention;

    //! \brief The periodicity (in seconds) with which the history is cleared
    TimestampType _history_purge_period;

    //! \brief The number of threads used for concurrent execution
    //! \details A value of zero is allowed for testing purposes, where no processing
    //! can be done automatically
    SizeType _concurrency;
};

//! \brief The runtime for performing collision detection
class Runtime {
  public:
    //! \brief Create using a generic broker access and a \a configuration
    Runtime(BrokerAccess const& access, RuntimeConfiguration const& configuration = RuntimeConfiguration());
    //! \brief Create using specific accesses/topics and a \a configuration
    Runtime(Pair<BrokerAccess,BodyPresentationTopic> const& bp_subscriber, Pair<BrokerAccess,HumanStateTopic> const& hs_subscriber,
            Pair<BrokerAccess,RobotStateTopic> const& rs_subscriber, Pair<BrokerAccess,CollisionNotificationTopic> const& cn_publisher,
            RuntimeConfiguration const& configuration = RuntimeConfiguration());

    Runtime(Runtime const&) = delete;
    void operator=(Runtime const&) = delete;

    //! \brief Get the number of registered pairs pending promotion to waiting jobs
    SizeType num_pending_human_robot_pairs() const;

    //! \brief Get the number of segment pairs that would need to be processed, given the registered bodies
    SizeType num_segment_pairs() const;

    //! \brief Get the number of waiting jobs in the queue
    SizeType num_waiting_jobs() const;

    //! \brief Get the number of sleeping jobs in the queue
    SizeType num_sleeping_jobs() const;

    //! \brief Stop the threads
    ~Runtime() noexcept;

  public:

    //! \brief [TEST] Reserve a job and process it
    void __test__process_one_working_job();
    bool __all_done() const { std::unique_lock<std::mutex> lock(_processing_mutex); return _num_processing == 0 and _waiting_jobs.size() == 0; }
    SizeType __num_processing() const { std::lock_guard<std::mutex> lock(_processing_mutex); return _num_processing; }
    SizeType __num_processed() const { return _num_processed; }
    SizeType __num_completed() const { return _num_completed; }
    SizeType __num_collisions() const { return _num_collisions; }
    SizeType __num_state_messages_received() const { return _receiver.__num_state_messages_received(); }

  private:
    //! \brief Process one element from the working jobs queue and possibly send a notification
    void _process_one_working_job();

  private:
    std::mutex mutable _availability_mutex;
    std::condition_variable _availability_condition;
    SynchronisedQueue<LookAheadJob> _waiting_jobs;
    SynchronisedQueue<LookAheadJob> _sleeping_jobs;

    bool _stop;

    BodyRegistry _registry;
    RuntimeReceiver _receiver;
    RuntimeSender _sender;

    List<SharedPointer<Thread>> _threads;

    std::mutex mutable _processing_mutex;

    std::atomic<SizeType> _num_processing;
    std::atomic<SizeType> _num_processed;
    std::atomic<SizeType> _num_completed;
    std::atomic<SizeType> _num_collisions;

    RuntimeConfiguration const _configuration;
};

}

#endif //OPERA_RUNTIME_HPP
