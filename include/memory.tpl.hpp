/***************************************************************************
 *            memory.tpl.hpp
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

#ifndef OPERA_MEMORY_TPL_HPP
#define OPERA_MEMORY_TPL_HPP

#include "memory.hpp"

namespace Opera {

template<> void MemoryBroker::put<BodyPresentationMessage>(BodyPresentationMessage const& msg) {
    std::lock_guard<std::mutex> lock(_mux);
    _body_presentations.push_back(msg);
}

template<> void MemoryBroker::put<BodyStateMessage>(BodyStateMessage const& msg) {
    std::lock_guard<std::mutex> lock(_mux);
    _body_states.push_back(msg);
}

template<> void MemoryBroker::put<CollisionNotificationMessage>(CollisionNotificationMessage const& msg) {
    std::lock_guard<std::mutex> lock(_mux);
    _collision_notifications.push_back(msg);
}

template<> BodyPresentationMessage MemoryBroker::get<BodyPresentationMessage>(SizeType const& idx) const { std::lock_guard<std::mutex> lock(_mux); return _body_presentations.at(idx); }
template<> BodyStateMessage MemoryBroker::get<BodyStateMessage>(SizeType const& idx) const { std::lock_guard<std::mutex> lock(_mux); return _body_states.at(idx); }
template<> CollisionNotificationMessage MemoryBroker::get<CollisionNotificationMessage>(SizeType const& idx) const { std::lock_guard<std::mutex> lock(_mux); return _collision_notifications.at(idx); }

template<> SizeType MemoryBroker::size<BodyPresentationMessage>() const { std::lock_guard<std::mutex> lock(_mux); return _body_presentations.size(); }
template<> SizeType MemoryBroker::size<BodyStateMessage>() const { std::lock_guard<std::mutex> lock(_mux); return _body_states.size(); }
template<> SizeType MemoryBroker::size<CollisionNotificationMessage>() const { std::lock_guard<std::mutex> lock(_mux); return _collision_notifications.size(); }

}

#endif // OPERA_MEMORY_TPL_HPP