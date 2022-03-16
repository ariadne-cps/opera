/***************************************************************************
 *            test_mqtt.cpp
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
#include "mqtt.hpp"

using namespace Opera;

class TestBrokerFailure {
  public:
    void test() {
        OPERA_TEST_CALL(test_failed_connection())
    }

    void test_failed_connection() {
        BrokerAccess access = MqttBrokerAccess("localhost",1900);
        OPERA_TEST_FAIL(access.make_body_presentation_publisher())
        OPERA_TEST_FAIL(access.make_body_presentation_subscriber([](auto){}))
    }
};

int main() {
    TestBrokerFailure().test();

    BrokerAccess access = MqttBrokerAccess(MQTT_HOST,1883);
    TestBrokerAccess(access).test();
    return OPERA_TEST_FAILURES;
}
