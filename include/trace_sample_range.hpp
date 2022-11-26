/***************************************************************************
 *            sample_range.hpp
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

#ifndef OPERA_TRACE_SAMPLE_RANGE_HPP
#define OPERA_TRACE_SAMPLE_RANGE_HPP

#include <deque>
#include "declarations.hpp"

namespace Opera {

//! \brief A trace-sample index
struct TraceSampleIndex {
    TraceSampleIndex(SizeType const& t, SizeType const& s) : trace(t), sample(s) { }
    SizeType trace;
    SizeType sample;
};

//! \brief A range of trace-sample indexes
class TraceSampleRange {
  public:
    //! \brief Construct from an \a initial range
    TraceSampleRange(TraceSampleIndex const& initial);
    //! \brief Add a new element with the given \a sample_idx as upper bound
    //! \details Used for debugging purposes
    TraceSampleRange& add(SizeType const& sample_idx);
    //! \brief Increase the trace index by adding a new element with sample index 0
    TraceSampleRange& increase_trace_index();
    //! \brief Update the sample index of the end of the range with the new value
    //! \details The new value must be greater than the current one
    TraceSampleRange& update(SizeType const& sample_idx);
    //! \brief Return the initial values
    TraceSampleIndex const& initial() const;
    //! \brief Get the upper bound at the given \a index
    //! \details An out-of-range index throws an error
    SizeType const& upper_bound(SizeType const& idx) const;
    //! \brief Get the maximum trace index
    SizeType maximum_trace_index() const;
    //! \brief Get the maximum sample index
    SizeType maximum_sample_index() const;
    //! \brief Scale the elements down of a given \a amount with respect to the trace,
    //! removing resulting negative trace indexes
    void scale_down_trace_of(SizeType const& amount);
    //! \brief Cut the last elements down to a trace index higher than \a index_bound
    void trim_down_trace_to(SizeType const& index_bound);
    //! \brief The size of the range
    SizeType size() const;
    //! \brief Whether this is empty
    bool is_empty() const;

    //! \brief Print on the standard output
    friend std::ostream& operator<<(std::ostream& os, TraceSampleRange const& r);

  private:
    TraceSampleIndex _initial;
    Deque<SizeType> _upper_bounds;
};

}

#endif //OPERA_TRACE_SAMPLE_RANGE_HPP
