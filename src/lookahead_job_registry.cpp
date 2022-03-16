/***************************************************************************
 *            lookahead_job_registry.cpp
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

#include "macros.hpp"
#include "lookahead_job_registry.hpp"

namespace Opera {

LookAheadJobTreeNode::LookAheadJobTreeNode(PriorityType const& priority) : _priority(priority), _registered(false) { }

bool LookAheadJobTreeNode::has_registered(SizeType const& depth, LookAheadJobPath const& path) const {
    if (depth == 0) return _registered;
    auto priority = path.priority(path.size() - depth);
    auto child_it = _children.find(priority);
    if (child_it == _children.end()) return false;
    auto& child = const_cast<LookAheadJobTreeNode&>(*child_it);
    return child.has_registered(depth - 1, path);
}

bool LookAheadJobTreeNode::try_register(SizeType const& depth, LookAheadJobPath const& path) {
    if (_registered) return false;
    if (depth == 0) {
        _registered = true;
        return true;
    }
    auto priority = path.priority(path.size() - depth);
    auto child_it = _children.find(priority);
    if (child_it == _children.end()) {
        auto item = _children.emplace(priority);
        child_it = item.first;
    }
    auto& child = const_cast<LookAheadJobTreeNode&>(*child_it);
    return child.try_register(depth - 1, path);
}

auto LookAheadJobTreeNode::priority() const -> PriorityType {
    return _priority;
}

bool operator<(LookAheadJobTreeNode const& first, LookAheadJobTreeNode const& second) {
    return first.priority() < second.priority();
}

LookAheadJobIdTree::LookAheadJobIdTree() : _root(LookAheadJobTreeNode(0)) { }

bool LookAheadJobIdTree::try_register(LookAheadJobPath const& path) {
    std::lock_guard<std::mutex> lock(_mux);
    return _root.try_register(path.size(), path);
}

bool LookAheadJobIdTree::has_registered(LookAheadJobPath const& path) const {
    std::lock_guard<std::mutex> lock(_mux);
    return _root.has_registered(path.size(), path);
}

LookAheadJobRegistryEntry::LookAheadJobRegistryEntry(TimestampType const& timestamp) : _timestamp(timestamp) { }

TimestampType const& LookAheadJobRegistryEntry::timestamp() const {
    return _timestamp;
}

bool LookAheadJobRegistryEntry::try_register(LookAheadJobIdentifier const& id, LookAheadJobPath const& path) {
    SharedPointer<LookAheadJobIdTree> tree = nullptr;
    {
        std::lock_guard<std::mutex> lock(_mux);
        auto it = _id_trees.find(id);
        if (it == _id_trees.end()) {
            auto res = _id_trees.insert({id,std::make_shared<LookAheadJobIdTree>()});
            it = res.first;
        }
        tree = it->second;
    }
    return tree->try_register(path);
}

bool LookAheadJobRegistryEntry::has_registered(LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const {
    SharedPointer<LookAheadJobIdTree> tree = nullptr;
    {
        std::lock_guard<std::mutex> lock(_mux);
        auto it = _id_trees.find(id);
        if (it == _id_trees.end()) return false;
        else tree = it->second;
    }
    return tree->has_registered(path);
}

bool LookAheadJobRegistry::try_register(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) {
    LookAheadJobRegistryEntry* entry = nullptr;
    {
        std::lock_guard<std::mutex> lock(_mux);
        if (_entries.empty() or _entries.back().timestamp() < timestamp) {
            _entries.emplace_back(timestamp);
            entry = &_entries.back();
        } else {
            bool found = false;
            for (auto it = _entries.rbegin(); it != _entries.rend(); ++it)
                if (it->timestamp() == timestamp) {
                    found = true;
                    entry = &*it;
                }
            OPERA_ASSERT_MSG(found,"Timestamp " << timestamp << " not found in the job registry.")
        }
    }
    return entry->try_register(id, path);
}

bool LookAheadJobRegistry::has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const {
    LookAheadJobRegistryEntry const* entry = nullptr;
    {
        std::lock_guard<std::mutex> lock(_mux);
        bool found = false;
        for (auto it = _entries.crbegin(); it != _entries.crend(); ++it)
            if (it->timestamp() == timestamp) {
                found = true;
                entry = &*it;
                break;
            }
        if (not found) return false;
    }
    return entry->has_registered(id, path);
}

}