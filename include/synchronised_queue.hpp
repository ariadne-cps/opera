/***************************************************************************
 *            synchronised_queue.hpp
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

#ifndef OPERA_SYNCHRONISED_QUEUE_HPP
#define OPERA_SYNCHRONISED_QUEUE_HPP

#include <queue>
#include <mutex>
#include "declarations.hpp"
#include "conclog/include/logging.hpp"

namespace Opera {

using VoidFunction = std::function<void()>;

//! \brief A queue that guarantees atomic operations
template<class T> class SynchronisedQueue {
  public:
    //! \brief Construct with an optional callback for every enqueueing
    SynchronisedQueue(VoidFunction const& callback = []{}) : _num_reserved(0), _callback(callback) { }
    //! \brief Add an element \a e to the back
    void enqueue(T const& e) { { std::lock_guard<std::mutex> lock(_mux); _queue.push(e); } _callback(); }
    //! \brief Get and remove the element in front
    T dequeue() {
        std::lock_guard<std::mutex> lock(_mux);
        OPERA_PRECONDITION(_queue.size() > 0)
        OPERA_PRECONDITION(_num_reserved > 0)
        T result = _queue.front();
        _queue.pop();
        --_num_reserved;
        return result;
    }

    //! \brief Reserve an item
    void reserve() { std::lock_guard<std::mutex> lock(_mux); ++_num_reserved; }

    //! \brief The number of reserved items
    SizeType num_reserved() const { std::lock_guard<std::mutex> lock(_mux); return _num_reserved; }

    //! \brief The size of the queue
    SizeType size() const { std::lock_guard<std::mutex> lock(_mux); return _queue.size(); }

    //! \brief Whether the queue has reserved slots
    bool can_reserve() const { std::lock_guard<std::mutex> lock(_mux); return _queue.size() > _num_reserved; }

  private:
    std::queue<T> _queue;
    SizeType _num_reserved;
    mutable std::mutex _mux;
    VoidFunction _callback;
};

}

#endif //OPERA_SYNCHRONISED_QUEUE_HPP
