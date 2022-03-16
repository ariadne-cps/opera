/***************************************************************************
 *            thread.cpp
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

#include "utility.hpp"
#include "thread.hpp"
#include "conclog/include/logging.hpp"

using namespace ConcLog;

namespace Opera {

Thread::Thread(std::function<void()> const& task, std::string const& name)
        : _name(name), _got_id_future(_got_id_promise.get_future()), _registered_thread_future(_registered_thread_promise.get_future())
{
    _thread = std::thread([=,this]() {
        _id = std::this_thread::get_id();
        _got_id_promise.set_value();
        _registered_thread_future.get();
        task();
    });
    _got_id_future.get();
    if (_name.empty()) _name = to_string(_id);
    Logger::instance().register_thread(this->id(),this->name());
    _registered_thread_promise.set_value();
}

std::thread::id const& Thread::id() const {
    return _id;
}

std::string const& Thread::name() const {
    return _name;
}

Thread::~Thread() {
    _thread.join();
    Logger::instance().unregister_thread(this->id());
}

} // namespace Opera
