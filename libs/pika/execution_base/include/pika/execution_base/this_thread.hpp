//  Copyright (c) 2019 Thomas Heller
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#include <pika/execution_base/agent_base.hpp>
#include <pika/execution_base/agent_ref.hpp>
#include <pika/timing/high_resolution_timer.hpp>
#include <pika/timing/steady_clock.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace pika { namespace execution_base {
    namespace detail {
        PIKA_EXPORT agent_base& get_default_agent();
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace this_thread {
        namespace detail {

            struct agent_storage;
            PIKA_EXPORT agent_storage* get_agent_storage();
        }    // namespace detail

        struct PIKA_EXPORT reset_agent
        {
            reset_agent(detail::agent_storage*, agent_base& impl);
            reset_agent(agent_base& impl);
            ~reset_agent();

            detail::agent_storage* storage_;
            agent_base* old_;
        };

        PIKA_EXPORT pika::execution_base::agent_ref agent();

        PIKA_EXPORT void yield(
            char const* desc = "pika::execution_base::this_thread::yield");
        PIKA_EXPORT void yield_k(std::size_t k,
            char const* desc = "pika::execution_base::this_thread::yield_k");
        PIKA_EXPORT void suspend(
            char const* desc = "pika::execution_base::this_thread::suspend");

        template <typename Rep, typename Period>
        void sleep_for(std::chrono::duration<Rep, Period> const& sleep_duration,
            char const* desc = "pika::execution_base::this_thread::sleep_for")
        {
            agent().sleep_for(sleep_duration, desc);
        }

        template <class Clock, class Duration>
        void sleep_until(
            std::chrono::time_point<Clock, Duration> const& sleep_time,
            char const* desc = "pika::execution_base::this_thread::sleep_for")
        {
            agent().sleep_until(sleep_time, desc);
        }
    }    // namespace this_thread
}}       // namespace pika::execution_base

namespace pika { namespace util {
    template <typename Predicate>
    void yield_while(Predicate&& predicate, const char* thread_name = nullptr,
        bool allow_timed_suspension = true)
    {
        if (allow_timed_suspension)
        {
            for (std::size_t k = 0; predicate(); ++k)
            {
                pika::execution_base::this_thread::yield_k(k, thread_name);
            }
        }
        else
        {
            for (std::size_t k = 0; predicate(); ++k)
            {
                pika::execution_base::this_thread::yield_k(k % 16, thread_name);
            }
        }
    }

    namespace detail {
        // yield_while_count yields until the predicate returns true
        // required_count times consecutively. This function is used in cases
        // where there is a small false positive rate and repeatedly calling the
        // predicate reduces the rate of false positives overall.
        //
        // Note: This is mostly a hack used to work around the raciness of
        // termination detection for thread pools and the runtime and can be
        // replaced if and when a better solution appears.
        template <typename Predicate>
        void yield_while_count(Predicate&& predicate,
            std::size_t required_count, const char* thread_name = nullptr,
            bool allow_timed_suspension = true)
        {
            std::size_t count = 0;
            if (allow_timed_suspension)
            {
                for (std::size_t k = 0;; ++k)
                {
                    if (!predicate())
                    {
                        if (++count > required_count)
                        {
                            return;
                        }
                    }
                    else
                    {
                        count = 0;
                        pika::execution_base::this_thread::yield_k(
                            k, thread_name);
                    }
                }
            }
            else
            {
                for (std::size_t k = 0;; ++k)
                {
                    if (!predicate())
                    {
                        if (++count > required_count)
                        {
                            return;
                        }
                    }
                    else
                    {
                        count = 0;
                        pika::execution_base::this_thread::yield_k(
                            k % 16, thread_name);
                    }
                }
            }
        }

        // yield_while_count_timeout is similar to yield_while_count, with the
        // addition of a timeout parameter. If the timeout is exceeded, waiting
        // is stopped and the function returns false. If the predicate is
        // successfully waited for the function returns true.
        template <typename Predicate>
        PIKA_NODISCARD bool yield_while_count_timeout(Predicate&& predicate,
            std::size_t required_count, std::chrono::duration<double> timeout,
            const char* thread_name = nullptr,
            bool allow_timed_suspension = true)
        {
            // Seconds represented using a double
            using duration_type = std::chrono::duration<double>;

            bool use_timeout = timeout >= duration_type(0.0);

            std::size_t count = 0;
            pika::chrono::high_resolution_timer t;

            if (allow_timed_suspension)
            {
                for (std::size_t k = 0;; ++k)
                {
                    if (use_timeout && duration_type(t.elapsed()) > timeout)
                    {
                        return false;
                    }

                    if (!predicate())
                    {
                        if (++count > required_count)
                        {
                            return true;
                        }
                    }
                    else
                    {
                        count = 0;
                        pika::execution_base::this_thread::yield_k(
                            k, thread_name);
                    }
                }
            }
            else
            {
                for (std::size_t k = 0;; ++k)
                {
                    if (use_timeout && duration_type(t.elapsed()) > timeout)
                    {
                        return false;
                    }

                    if (!predicate())
                    {
                        if (++count > required_count)
                        {
                            return true;
                        }
                    }
                    else
                    {
                        count = 0;
                        pika::execution_base::this_thread::yield_k(
                            k % 16, thread_name);
                    }
                }
            }
        }
    }    // namespace detail
}}       // namespace pika::util
