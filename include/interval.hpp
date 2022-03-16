/***************************************************************************
 *            interval.hpp
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

#ifndef OPERA_INTERVAL_HPP
#define OPERA_INTERVAL_HPP

#include "declarations.hpp"
#include "macros.hpp"

namespace Opera {

//! \brief A representation of an interval of values
template<class T> class Interval {
  public:
    //! \brief Construct from fields
    Interval(T const& lower, T const& upper) : _lower(lower), _upper(upper) { OPERA_PRECONDITION(lower<=upper) }
    //! \brief Construct singleton (explicit to avoid passing a value when an actual interval is expected)
    explicit Interval(T const& value) : Interval(value,value) { }

    //! \brief Get the lower bound
    T const& lower() const { return _lower; }
    //! \brief Get the upper bound
    T const& upper() const { return _upper; }

    //! \brief Set the lower bound
    void set_lower(T const& v) { OPERA_PRECONDITION(v<=_upper) _lower = v; }
    //! \brief Set the upper bound
    void set_upper(T const& v) { OPERA_PRECONDITION(_lower<=v) _upper = v; }

    //! \brief Addition between intervals
    friend Interval operator+(Interval const& i1, Interval const& i2) { return {i1.lower()+i2.lower(),i1.upper()+i2.upper()}; }

    //! \brief Addition of a scalar
    friend Interval operator+(Interval const& i1, T const& v) { return {i1.lower()+v,i1.upper()+v}; }

    //! \brief Subtraction of a scalar
    friend Interval operator-(Interval const& i1, T const& v) { return {i1.lower()-v,i1.upper()-v}; }

    //! \brief Equality operator
    bool operator==(Interval const& i2) const { return this->lower() == i2.lower() and this->upper() == i2.upper(); }

    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, Interval<T> const& ival) {
        if (ival.lower() == ival.upper()) return os << "[" << ival.lower() << "]";
        else return os << "[" << ival.lower() << ":" << ival.upper() <<"]";
    }

  private:
    T _lower;
    T _upper;
};

}

#endif //OPERA_INTERVAL_HPP
