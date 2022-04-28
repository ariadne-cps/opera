/***************************************************************************
 *            mode.hpp
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

#ifndef OPERA_MODE_HPP
#define OPERA_MODE_HPP

#include <initializer_list>
#include <deque>
#include "declarations.hpp"
#include "utility.hpp"

namespace Opera {

using String = std::string;

//! \brief A map of string variables with values, defining a mode of operation
//! or equivalently the discrete state of a body
class Mode {
  public:
    //! \brief Construct empty
    Mode() = default;
    //! \brief Construct from a single pair
    Mode(const Pair<String,String>& pair);
    //! \brief Construct from a map directly
    Mode(const Map<String,String>& sm);
    //! \brief Construct from an initialiser list of pairs
    Mode(std::initializer_list<Pair<String,String>> const& vals);

    //! \brief Whether there are no variables defined
    bool is_empty() const;

    //! \brief The values held
    Map<String,String> const& values() const;

    //! \relates Mode \brief Equality test.
    //! Throws an error if the valuations are not identical but have no variable with distinct values.
    friend bool operator==(const Mode& loc1, const Mode& loc2);
    //! \relates Mode \brief Inequality test.
    friend bool operator!=(const Mode& loc1, const Mode& loc2);
    //! \relates Mode \brief A total order on Mode, allowing comparison of non-distinuishable valuations.
    friend bool operator<(const Mode& loc1, const Mode& loc2);

    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, Mode const& s);
  private:
    Map<String,String> _mapping;
};

struct ModeTraceEntry {
    ModeTraceEntry(Mode const& m, PositiveFloatType const& l) : mode(m), likelihood(l) { }
    Mode mode;
    PositiveFloatType likelihood;
};

//! \brief A trace of modes, enriched by the likelihood that this trace is followed
class ModeTrace {
  public:
    //! \brief Default constructor has empty trace
    ModeTrace() = default;
    //! \brief The entry in the trace at \a idx, i.e. the mode and the likelihood of the trace up to there
    ModeTraceEntry const& at(SizeType const& idx) const;

    //! \brief The starting mode in the trace
    Mode const& starting_mode() const;
    //! \brief The ending mode in the trace
    Mode const& ending_mode() const;
    //! \brief The likelihood of this trace, i.e. of its final mode
    PositiveFloatType likelihood() const;

    //! \brief The next modes with their probability, which accounts for the likelihood of this trace
    //! \details This is computed lazily
    Map<Mode,PositiveFloatType> const& next_modes() const;

    //! \brief Return the index of the first presence of the given \a mode, -1 if not found
    int forward_index(Mode const& mode) const;
    //! \brief Return the index of the last presence of the given \a mode, -1 if not found
    int backward_index(Mode const& mode) const;

    //! \brief Whether the trace contains the element
    bool contains(Mode const& mode) const;

    //! \brief Whether the trace has created at least a loop, i.e., its ending mode is already present somewhere else in the trace
    bool has_looped() const;

    //! \brief Add a mode to the head of the trace, with likelihood set to 1.0
    ModeTrace& push_front(Mode const& mode);
    //! \brief Add a \a mode with own \a likelihoood to the tail of the trace
    //! \details The \a likelihood is multiplied by the tail likelihood as obtained by likelihood()
    ModeTrace& push_back(Mode const& mode, PositiveFloatType const& likelihood = 1.0) const;

    //! \brief Reduce the trace to be between an \a initial and \a final modes
    void reduce_between(Mode const& initial, Mode const& final);

    //! \brief Reduce the trace to be between a \a lower and \a upper indices
    void reduce_between(SizeType const& lower, SizeType const& upper);

    //! \brief The number of modes
    SizeType size() const;

    //! \brief Equality operator
    bool operator==(ModeTrace const& other) const;

    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, ModeTrace const& t);

private:
    std::deque<ModeTraceEntry> _entries;
    mutable Map<Mode,PositiveFloatType> _next_modes;
};

//! \brief Create a trace which has has tail \a t1, prefixed by the modes of \a t2 from the first occurrence of the head of \a t1
//! \details The likelihood is given by the likelihood of \a t2
ModeTrace merge(ModeTrace const& t1, ModeTrace const& t2);

} //namespace Opera

#endif /* OPERA_MODE_HPP */
