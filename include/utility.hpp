/***************************************************************************
 *            utility.hpp
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

#ifndef OPERA_UTILITY_HPP
#define OPERA_UTILITY_HPP

#include <filesystem>
#include <map>
#include <sstream>
#include "declarations.hpp"
#include "config.hpp"

using FilePath = std::filesystem::path;

namespace Opera {

class Resources {
  public:
    static FilePath path(std::string const& filename) {
        return (std::string(RESOURCES_PATH) + filename).c_str();
    }
};

template<class T> inline std::string to_string(const T& t) { std::stringstream ss; ss << t; return ss.str(); }

inline std::string format(TimestampType const& timestamp, std::string fmt = "%d/%m/%y %T") {
    time_t timestamp_t = static_cast<time_t>(timestamp/1000);
    char mbstr[100];
    #ifdef _WIN32
        struct tm _tm;
        ::localtime_s(&_tm,&timestamp_t);
        std::strftime(mbstr, sizeof(mbstr), fmt.c_str(), &_tm);
    #else
        std::strftime(mbstr, sizeof(mbstr), fmt.c_str(), std::localtime(&timestamp_t));
    #endif
    return mbstr;
}

//! \brief Soft wrapper to expose the key finding functionality
template<class K, class V> class Map : public std::map<K,V> {
  public:
    bool has_key(K const& key) const { return this->find(key) != this->end(); }
};

template<class T> std::ostream& operator<<(std::ostream& os, List<T> const& l) {
    if (l.empty()) return os << "[]";
    os << "[" << l.at(0);
    for (SizeType i=1;i<l.size();++i) os << "," << l.at(i);
    return os << "]";
}

}

#endif //OPERA_UTILITY_HPP
