//  Copyright (c) 2017 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file pika/runtime/thread_pool_helpers.hpp

#pragma once

#include <pika/config.hpp>
#include <pika/threading_base/thread_pool_base.hpp>
#include <pika/topology/cpu_mask.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace pika { namespace resource {
    ///////////////////////////////////////////////////////////////////////////
    /// Return the number of thread pools currently managed by the
    /// \a resource_partitioner
    PIKA_EXPORT std::size_t get_num_thread_pools();

    /// Return the number of threads in all thread pools currently
    /// managed by the \a resource_partitioner
    PIKA_EXPORT std::size_t get_num_threads();

    /// Return the number of threads in the given thread pool currently
    /// managed by the \a resource_partitioner
    PIKA_EXPORT std::size_t get_num_threads(std::string const& pool_name);

    /// Return the number of threads in the given thread pool currently
    /// managed by the \a resource_partitioner
    PIKA_EXPORT std::size_t get_num_threads(std::size_t pool_index);

    /// Return the internal index of the pool given its name.
    PIKA_EXPORT std::size_t get_pool_index(std::string const& pool_name);

    /// Return the name of the pool given its internal index
    PIKA_EXPORT std::string const& get_pool_name(std::size_t pool_index);

    /// Return the name of the pool given its name
    PIKA_EXPORT threads::thread_pool_base& get_thread_pool(
        std::string const& pool_name);

    /// Return the thread pool given its internal index
    PIKA_EXPORT threads::thread_pool_base& get_thread_pool(
        std::size_t pool_index);

    /// Return true if the pool with the given name exists
    PIKA_EXPORT bool pool_exists(std::string const& pool_name);

    /// Return true if the pool with the given index exists
    PIKA_EXPORT bool pool_exists(std::size_t pool_index);
}}    // namespace pika::resource

namespace pika { namespace threads {
    ///    The function \a get_thread_count returns the number of currently
    /// known threads.
    ///
    /// \param state    [in] This specifies the thread-state for which the
    ///                 number of threads should be retrieved.
    ///
    /// \note If state == unknown this function will not only return the
    ///       number of currently existing threads, but will add the number
    ///       of registered task descriptions (which have not been
    ///       converted into threads yet).
    PIKA_EXPORT std::int64_t get_thread_count(
        thread_schedule_state state = thread_schedule_state::unknown);

    /// The function \a get_thread_count returns the number of currently
    /// known threads.
    ///
    /// \param priority [in] This specifies the thread-priority for which the
    ///                 number of threads should be retrieved.
    /// \param state    [in] This specifies the thread-state for which the
    ///                 number of threads should be retrieved.
    ///
    /// \note If state == unknown this function will not only return the
    ///       number of currently existing threads, but will add the number
    ///       of registered task descriptions (which have not been
    ///       converted into threads yet).
    PIKA_EXPORT std::int64_t get_thread_count(thread_priority priority,
        thread_schedule_state state = thread_schedule_state::unknown);

    /// The function \a get_idle_core_count returns the number of currently
    /// idling threads (cores).
    PIKA_EXPORT std::int64_t get_idle_core_count();

    /// The function \a get_idle_core_mask returns a bit-mask representing the
    /// currently idling threads (cores).
    PIKA_EXPORT detail::mask_type get_idle_core_mask();

    /// The function \a enumerate_threads will invoke the given function \a f
    /// for each thread with a matching thread state.
    ///
    /// \param f        [in] The function which should be called for each
    ///                 matching thread. Returning 'false' from this function
    ///                 will stop the enumeration process.
    /// \param state    [in] This specifies the thread-state for which the
    ///                 threads should be enumerated.
    PIKA_EXPORT bool enumerate_threads(
        util::function<bool(thread_id_type)> const& f,
        thread_schedule_state state = thread_schedule_state::unknown);
}}    // namespace pika::threads
