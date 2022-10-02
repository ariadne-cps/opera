/***************************************************************************
 *            test_kafka.cpp
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

#include "test.hpp"
#include "config.hpp"
#include "test_broker_access.hpp"
#include "kafka.hpp"

using namespace Opera;

int main() {

    BrokerAccess access = KafkaBrokerAccessBuilder(Environment::get("KAFKA_BROKER_URI"))
                          .set_topic_prefix(Environment::get("KAFKA_TOPIC_PREFIX"))
                          .set_sasl_mechanism(Environment::get("KAFKA_SASL_MECHANISM"))
                          .set_security_protocol(Environment::get("KAFKA_SECURITY_PROTOCOL"))
                          .set_sasl_username(Environment::get("KAFKA_USERNAME"))
                          .set_sasl_password(Environment::get("KAFKA_PASSWORD"))
                          .build();
    TestBrokerAccess(access,2500).test();
    return OPERA_TEST_FAILURES;
}
