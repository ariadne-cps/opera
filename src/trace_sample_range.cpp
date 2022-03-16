/***************************************************************************
 *            trace_sample_range.cpp
 *
 *  Copyright  2022  Luca Geretti
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

#include "macros.hpp"
#include "trace_sample_range.hpp"

namespace Opera {

TraceSampleRange::TraceSampleRange(TraceSampleIndex const& initial) : _initial(initial) {
    _upper_bounds.push_back(initial.sample);
}

TraceSampleRange& TraceSampleRange::add(SizeType const& sample_idx) {
    _upper_bounds.push_back(sample_idx);
    return *this;
}

TraceSampleRange& TraceSampleRange::increase_trace_index() {
    _upper_bounds.push_back(0);
    return *this;
}

TraceSampleRange& TraceSampleRange::update(SizeType const& sample_idx) {
    OPERA_PRECONDITION(sample_idx > _upper_bounds.back())
    _upper_bounds.back() = sample_idx;
    return *this;
}

TraceSampleIndex const& TraceSampleRange::initial() const {
    return _initial;
}

SizeType const& TraceSampleRange::upper_bound(SizeType const& idx) const {
    OPERA_PRECONDITION(idx >= _initial.trace and idx <= maximum_trace_index())
    return _upper_bounds.at(idx-_initial.trace);
}

SizeType TraceSampleRange::maximum_trace_index() const {
    OPERA_PRECONDITION(not is_empty())
    return _initial.trace+_upper_bounds.size()-1;
}

SizeType TraceSampleRange::maximum_sample_index() const {
    OPERA_PRECONDITION(not is_empty())
    return _upper_bounds.back();
}

void TraceSampleRange::scale_down_trace_of(SizeType const& amount) {
    if (maximum_trace_index() < amount) {
        _upper_bounds.clear();
        _initial = TraceSampleIndex(0, 0);
    } else {
        if (_initial.trace < amount) {
            for (SizeType i=0; i<amount-_initial.trace; ++i) _upper_bounds.pop_front();
            _initial = TraceSampleIndex(0, 0);
        } else
            _initial.trace -= amount;
    }
}

void TraceSampleRange::trim_down_trace_to(SizeType const& index_bound) {
    while (not is_empty() and maximum_trace_index() > index_bound) _upper_bounds.pop_back();
}

bool TraceSampleRange::is_empty() const {
    return _upper_bounds.empty();
}

SizeType TraceSampleRange::size() const {
    return _upper_bounds.size();
}

std::ostream& operator<<(std::ostream& os, TraceSampleRange const& r) {
    if (r.is_empty()) return os << "{}";
    else return os << "{" << r._initial.trace << "@" << r._initial.sample << "->" << r.maximum_trace_index() << "@" << r.maximum_sample_index() << "}";
}

}