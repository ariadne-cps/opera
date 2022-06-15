/***************************************************************************
 *            broker_access_manager.hpp
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

#ifndef OPERA_BROKER_ACCESS_MANAGER_HPP
#define OPERA_BROKER_ACCESS_MANAGER_HPP

#include "memory.hpp"
#include "mqtt.hpp"
#include "kafka.hpp"

namespace Opera {

//! \brief A static manager for the broker access to use
class BrokerAccessManager {
  public:

    //! \brief Destructor removes the access
    ~BrokerAccessManager() { delete _access; };

    //! \brief Singleton object
    static BrokerAccessManager& instance() {
        static BrokerAccessManager manager;
        return manager;
    }

    //! \brief Configure to assign the access
    void configure(std::string broker_type, std::string address, int arg) {
        //if (broker_type == "kafka") _access = new BrokerAccess(KafkaBrokerAccess(arg,address,RdKafka::Topic::OFFSET_END));
        if (broker_type == "memory") _access = new BrokerAccess(MemoryBrokerAccess());
        else if (broker_type == "mqtt") _access = new BrokerAccess(MqttBrokerAccess(address,arg));
    }

    BrokerAccess const& get_access() const {
        OPERA_ASSERT_MSG(_access != nullptr,"First configure the broker access manager.")
        return *_access;
    }

  private:
    BrokerAccess* _access;
};

}

#endif // OPERA_BROKER_ACCESS_MANAGER_HPP