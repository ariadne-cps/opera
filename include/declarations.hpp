/***************************************************************************
 *            declarations.hpp
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

#ifndef OPERA_DECLARATIONS_HPP
#define OPERA_DECLARATIONS_HPP

#include <cstring>
#include <numeric>
#include <vector>
#include <set>
#include <deque>
#include <memory>

namespace Opera {

typedef double FloatType;
typedef FloatType PositiveFloatType;
template<class T> using List = std::vector<T>;
template<class T1, class T2> using Pair = std::pair<T1,T2>;
template<class T> using Set = std::set<T>;
template<class T> using Deque = std::deque<T>;

using SizeType = size_t;
template<class T> using SharedPointer = std::shared_ptr<T>;
using TimestampType = uint64_t; // Expressed in milliseconds

static const FloatType infinity = std::numeric_limits<FloatType>::infinity();

}

#endif //OPERA_DECLARATIONS_HPP
