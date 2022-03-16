/***************************************************************************
 *            handle.hpp
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

#ifndef OPERA_HANDLE_HPP
#define OPERA_HANDLE_HPP

#include <memory>

namespace Opera {

//! \brief A handle for interface implementations
template<class I> class Handle {
  protected:
    mutable std::shared_ptr<I> _ptr;
  public:
    ~Handle() { }
    explicit Handle(I* p) : _ptr(p) { }
    Handle(std::shared_ptr<I> p) : _ptr(p) { }
    template<class T> Handle(const T& t)
        : _ptr(new T(t)) { }
    Handle(const Handle<I>& h) = default;
    Handle(Handle<I>&& h) = default;
    Handle<I>& operator=(const Handle<I>& h) = default;
    Handle<I>& operator=(Handle<I>&& h) = default;
    operator I const& () const { return *_ptr; }
    const I* ptr() const { return _ptr.operator->(); }
};

} // namespace Opera

#endif // OPERA_HANDLE_HPP
