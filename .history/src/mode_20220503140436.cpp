/***************************************************************************
 *            mode.cpp
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

#include "mode.hpp"
#include "macros.hpp"

namespace Opera {

Mode::Mode(const Map<String,String>& sm) : _mapping(sm) { }

Mode::Mode(Pair<String,String> const& pair) : Mode({pair}) { }

Mode::Mode(std::initializer_list<Pair<String,String>> const& vals) {
    for (auto const& v : vals) {
        _mapping.insert(std::make_pair(v.first,v.second));
    }
}

bool Mode::is_empty() const {
    return _mapping.empty();
}

Map<String,String> const& Mode::values() const {
    return _mapping;
}

bool operator==(const Mode& q1, const Mode& q2) {
    bool identical=true;
    const Map<String,String>& q1sm=q1._mapping;
    const Map<String,String>& q2sm=q2._mapping;
    auto q1iter=q1sm.begin();
    auto q2iter=q2sm.begin();

    while(q1iter!=q1sm.end() && q2iter!=q2sm.end()) {
        if(q1iter->first==q2iter->first) {
            if(q1iter->second != q2iter->second) { return false; }
            ++q1iter; ++q2iter;
        } else if(q1iter->first<q2iter->first) {
            identical=false; ++q1iter;
        } else {
            identical=false; ++q2iter;
        }
    }
    if(q1iter!=q1sm.end() || q2iter!=q2sm.end()) { identical=false; }
    OPERA_ASSERT_MSG(identical,"Modes "<<q1<<" and "<<q2<<" have different key values, hence they are not comparable.")
    return true;
}

bool operator!=(const Mode& q1, const Mode& q2) {
    return !(q1==q2);
}

bool operator<(const Mode& q1, const Mode& q2) {
    const Map<String,String>& q1sm=q1._mapping;
    const Map<String,String>& q2sm=q2._mapping;
    auto q1iter=q1sm.begin();
    auto q2iter=q2sm.begin();

    while(q1iter!=q1sm.end() && q2iter!=q2sm.end()) {
        if(q1iter->first!=q2iter->first) {
            return (q1iter->first<q2iter->first);
        } else if(q1iter->second != q2iter->second) {
            return q1iter->second < q2iter->second;
        } else {
            ++q1iter; ++q2iter;
        }
    }
    return q2iter!=q2sm.end();
}

std::ostream& operator<<(std::ostream& os, Mode const& s) {
    if (s._mapping.empty()) return os << "{}";
    auto it=s._mapping.begin();
    os << "{" << it->first << "|" << it->second;
    while (++it != s._mapping.end())
        os << "," << it->first << "|" << it->second;
    return os << "}";
}

ModeTrace& ModeTrace::push_front(Mode const& mode) {
    _entries.push_front({mode,1.0});
    _next_modes.clear();
    return *this;
}

ModeTrace& ModeTrace::push_back(Mode const& mode, PositiveFloatType const& l){
    _entries.push_back({mode,likelihood()*l});
    _next_modes.clear();
    return *this;
}

int ModeTrace::forward_index(Mode const& mode) const {
    for (SizeType i=0; i<_entries.size(); ++i)
        if (_entries.at(i).mode == mode) return static_cast<int>(i);
    return -1;
}

int ModeTrace::backward_index(Mode const& mode) const {
    SizeType top = _entries.size()-1;
    for (auto it = _entries.crbegin(); it != _entries.crend(); ++it)
        if (it->mode == mode) return static_cast<int>(top);
        else top--;
    return -1;
}

void ModeTrace::reduce_between(Mode const& initial, Mode const& final) {
    OPERA_ASSERT_MSG(_entries.size()>0, "Cannot reduce an empty mode trace")

    int bottom = forward_index(initial);
    OPERA_ASSERT_MSG(bottom != -1, "Initial mode " << initial << " not found in the mode trace "  << *this)

    int top = backward_index(final);
    OPERA_ASSERT_MSG(top != -1, "Final mode " << final << " not found in the mode trace")

    reduce_between(static_cast<SizeType>(bottom),static_cast<SizeType>(top));
}

void ModeTrace::reduce_between(SizeType const& lower, SizeType const& upper) {
    OPERA_ASSERT_MSG(_entries.size()>0, "Cannot reduce an empty mode trace")
    OPERA_ASSERT_MSG(lower<=upper, "The reduction bounds are inconsistent: " << lower << " vs " << upper)

    std::deque<ModeTraceEntry> new_entries;
    for (SizeType i=lower; i<=upper; ++i) new_entries.push_back(_entries.at(i));
    _entries = new_entries;
}

bool ModeTrace::contains(Mode const& mode) const {
    for (auto const& e : _entries) if (e.mode == mode) return true;
    return false;
}

bool ModeTrace::has_looped() const {
    if (_entries.size() <= 1) return false;
    auto const& mode_to_find = ending_mode();
    for (SizeType i=0; i<_entries.size()-1; ++i)
        if (_entries.at(i).mode == mode_to_find) return true;
    return false;
}

SizeType ModeTrace::size() const {
    return _entries.size();
}

ModeTraceEntry const& ModeTrace::at(SizeType const& idx) const {
    OPERA_PRECONDITION(idx < _entries.size())
    return _entries.at(idx);
}

//#~#v
ModeTrace ModeTrace::clone(ModeTrace const& mode_trace){
    result = ModeTrace();
    for (ModeTraceEntry entry : this){
        result.
    }
}
//#~#^

Mode const& ModeTrace::starting_mode() const {
    return _entries.at(0).mode;
}

Mode const& ModeTrace::ending_mode() const {
    return _entries.at(_entries.size()-1).mode;
}

PositiveFloatType ModeTrace::likelihood() const {
    if (_entries.size() == 0) return 1.0;
    return _entries.at(_entries.size()-1).likelihood;
}

bool ModeTrace::operator==(ModeTrace const& other) const {
    if (this->size() != other.size()) return false;
    for (SizeType i=0; i<this->size(); ++i)
        if (this->_entries.at(i).mode != other._entries.at(i).mode or this->_entries.at(i).likelihood != other._entries.at(i).likelihood)
            return false;
    return true;
}

std::ostream& operator<<(std::ostream& os, ModeTrace const& t) {
    List<Pair<Mode,PositiveFloatType>> entries;
    os << "{";
    if (t.size() > 0) {
        os << t.at(0).mode << "@" << t.at(0).likelihood;
        for (SizeType i=1; i<t.size(); ++i) os << "," << t.at(i).mode << "@" << t.at(i).likelihood;
    }
    return os << "}";
}

struct RobotDiscreteTraceBacktracking {
    RobotDiscreteTraceBacktracking(SizeType const& index_, ModeTrace const& trace_, Mode const& _next_mode, bool const& _is_valid)
            : index(index_), trace(trace_), next_mode(_next_mode), is_valid(_is_valid) { }
    SizeType index;
    ModeTrace trace;
    Mode next_mode;
    bool is_valid;
};

Map<Mode,PositiveFloatType> const& ModeTrace::next_modes() const {
    if (_next_modes.empty()) {
        List<RobotDiscreteTraceBacktracking> tracking;
        for (SizeType i=0; i<_entries.size()-1; ++i)
            if (_entries.at(i).mode == _entries.back().mode)
                tracking.push_back(RobotDiscreteTraceBacktracking(i, ModeTrace().push_back(_entries.at(i).mode,_entries.at(i).likelihood), _entries.at(i + 1).mode, true));
        SizeType maximum_trace_size = 0;
        SizeType num_having_maximum_trace_size = 0;
        SizeType num_valid = tracking.size();
        while (num_valid > 0) {
            maximum_trace_size++;
            num_having_maximum_trace_size = num_valid;
            for (auto& st : tracking) {
                if (st.is_valid) {
                    if (st.index > 0) {
                        st.index--;
                        if (_entries.at(st.index).mode == _entries.at(_entries.size() - 1 - maximum_trace_size).mode) {
                            st.trace.push_front(_entries.at(st.index).mode);
                        } else {
                            num_valid--;
                            st.is_valid = false;
                        }
                    } else {
                        num_valid--;
                        st.is_valid = false;
                    }
                }
            }
        }
        for (auto t : tracking) {
            if (t.trace.size() == maximum_trace_size) {
                if (_next_modes.has_key(t.next_mode))
                    _next_modes.at(t.next_mode) += 1.0;
                else
                    _next_modes.insert(std::make_pair(t.next_mode,1.0));
            }
        }
        for (auto& l : _next_modes) {
            l.second /= num_having_maximum_trace_size;
        }
    }
    return _next_modes;
}

ModeTrace merge(ModeTrace const& t1, ModeTrace const& t2) {
    ModeTrace result = t2;
    SizeType i=t1.size();
    if (t1.at(i-1).mode == t2.at(0).mode) --i;
    while (i>0) result.push_front(t1.at(--i).mode);
    return result;
}

} //namespace Opera
