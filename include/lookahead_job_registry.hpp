/***************************************************************************
 *            lookahead_job_registry.hpp
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

#ifndef OPERA_LOOKAHEAD_JOB_REGISTRY_HPP
#define OPERA_LOOKAHEAD_JOB_REGISTRY_HPP

#include "lookahead_job.hpp"

namespace Opera {

//! \brief A node for a tree describing jobs that process a given human sample
class LookAheadJobTreeNode {
    using PriorityType = LookAheadJobPath::PriorityType;
  public:
    //! \brief Construct with a \a priority
    LookAheadJobTreeNode(PriorityType const& priority);

    //! \brief Try to register the element at \a depth according to the given \a path
    //! \return Whether the node can be registered
    bool try_register(SizeType const& depth, LookAheadJobPath const& path);

    //! \brief Check if the \a path is registered, recursively to the given \a depth
    //! \details If this is the root node, then \a depth should be the size of \a path
    bool has_registered(SizeType const& depth, LookAheadJobPath const& path) const;

    //! \brief Return the priority
    PriorityType priority() const;

  private:
    PriorityType _priority;
    bool _registered;
    Set<LookAheadJobTreeNode> _children;
};

//! \brief Comparison used for set ordering
bool operator<(LookAheadJobTreeNode const& first, LookAheadJobTreeNode const& second);

//! \brief A tree for the job registry for a given job id
class LookAheadJobIdTree {
  public:
    //! \brief Create empty
    LookAheadJobIdTree();

    //! \brief Try to register the given \a path
    //! \return True if registered successfully, false if the job should not be created
    bool try_register(LookAheadJobPath const& path);

    //! \brief Check if the \a path is registered
    bool has_registered(LookAheadJobPath const& path) const;

  private:
    LookAheadJobTreeNode _root;
    std::mutex mutable _mux;
};

//! \brief An entry for the job registry
class LookAheadJobRegistryEntry {
public:
    //! \brief Create empty
    LookAheadJobRegistryEntry(TimestampType const& timestamp);

    //! \brief The timestamp
    TimestampType const& timestamp() const;

    //! \brief Try to register the given job \a id and \a path
    //! \return True if registered successfully, false if the job should not be created
    bool try_register(LookAheadJobIdentifier const& id, LookAheadJobPath const& path);

    //! \brief Check if the \a path is registered at the given \a id
    bool has_registered(LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const;

private:
    TimestampType _timestamp;
    Map<LookAheadJobIdentifier,SharedPointer<LookAheadJobIdTree>> _id_trees;
    std::mutex mutable _mux;
};

//! \brief A registry that tracks valid jobs created from a job factory
//! \details Given LookAheadJobPath information, the registry avoids to end up running
//! the same working job(s) multiple times while keeping the remaining job manipulations
//! fully distributed.
class LookAheadJobRegistry {
  public:
    //! \brief Try to register the given \a path at the given \a timestamp and job \a id
    //! \return True if registered successfully, false if the job should not be created
    bool try_register(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path);

    //! \brief Check if the \a path is registered at the given \a id and \a timestamp
    bool has_registered(TimestampType const& timestamp, LookAheadJobIdentifier const& id, LookAheadJobPath const& path) const;
  private:
    std::deque<LookAheadJobRegistryEntry> _entries;
    std::mutex mutable _mux;
};

}

#endif //OPERA_LOOKAHEAD_JOB_REGISTRY_HPP
