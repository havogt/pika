//  Copyright (c) 2021 ETH Zurich
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#include <pika/execution_base/sender.hpp>
#include <pika/functional/tag_invoke.hpp>

#include <type_traits>

namespace pika::execution::experimental {
    template <typename Scheduler>
    struct get_completion_scheduler_t final
      : pika::functional::tag<get_completion_scheduler_t<Scheduler>>
    {
    };

    template <typename Scheduler>
    PIKA_HOST_DEVICE_INLINE_CONSTEXPR_VARIABLE
        get_completion_scheduler_t<Scheduler>
            get_completion_scheduler{};

    namespace detail {
        template <bool TagInvocable, typename CPO, typename Sender>
        struct has_completion_scheduler_impl : std::false_type
        {
        };

        template <typename CPO, typename Sender>
        struct has_completion_scheduler_impl<true, CPO, Sender>
          : pika::execution::experimental::is_scheduler<pika::functional::
                    tag_invoke_result_t<get_completion_scheduler_t<CPO>,
                        std::decay_t<Sender> const&>>
        {
        };

        template <typename CPO, typename Sender>
        struct has_completion_scheduler
          : has_completion_scheduler_impl<pika::functional::is_tag_invocable_v<
                                              get_completion_scheduler_t<CPO>,
                                              std::decay_t<Sender> const&>,
                CPO, Sender>
        {
        };

        template <typename CPO, typename Sender>
        inline constexpr bool has_completion_scheduler_v =
            has_completion_scheduler<CPO, Sender>::value;

        template <bool HasCompletionScheduler, typename ReceiverCPO,
            typename Sender, typename AlgorithmCPO, typename... Ts>
        struct is_completion_scheduler_tag_invocable_impl : std::false_type
        {
        };

        template <typename ReceiverCPO, typename Sender, typename AlgorithmCPO,
            typename... Ts>
        struct is_completion_scheduler_tag_invocable_impl<true, ReceiverCPO,
            Sender, AlgorithmCPO, Ts...>
          : std::integral_constant<bool,
                pika::functional::is_tag_invocable_v<AlgorithmCPO,
                    pika::functional::tag_invoke_result_t<
                        pika::execution::experimental::
                            get_completion_scheduler_t<ReceiverCPO>,
                        Sender>,
                    Sender, Ts...>>
        {
        };

        template <typename ReceiverCPO, typename Sender, typename AlgorithmCPO,
            typename... Ts>
        struct is_completion_scheduler_tag_invocable
          : is_completion_scheduler_tag_invocable_impl<
                pika::execution::experimental::detail::
                    has_completion_scheduler_v<ReceiverCPO, Sender>,
                ReceiverCPO, Sender, AlgorithmCPO, Ts...>
        {
        };

        template <typename ReceiverCPO, typename Sender, typename AlgorithmCPO,
            typename... Ts>
        inline constexpr bool is_completion_scheduler_tag_invocable_v =
            is_completion_scheduler_tag_invocable<ReceiverCPO, Sender,
                AlgorithmCPO, Ts...>::value;

    }    // namespace detail
}    // namespace pika::execution::experimental
