//  Copyright (c) 2017 Antoine Tran Tan
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/datastructures/tuple.hpp>

#include <type_traits>

namespace pika { namespace util {
    namespace detail {

        ///////////////////////////////////////////////////////////////////////
        template <typename Tuple>
        struct tuple_first_argument;

        template <>
        struct tuple_first_argument<pika::tuple<>>
        {
            using type = std::false_type;
        };

        template <typename Arg0, typename... Args>
        struct tuple_first_argument<pika::tuple<Arg0, Args...>>
        {
            using type = typename std::decay<Arg0>::type;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename F>
        struct function_first_argument;

        template <typename ReturnType>
        struct function_first_argument<ReturnType (*)()>
        {
            using type = std::false_type;
        };

        template <typename ReturnType, typename Arg0, typename... Args>
        struct function_first_argument<ReturnType (*)(Arg0, Args...)>
        {
            using type = typename std::decay<Arg0>::type;
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename F>
        struct lambda_first_argument;

        template <typename ClassType, typename ReturnType>
        struct lambda_first_argument<ReturnType (ClassType::*)() const>
        {
            using type = std::false_type;
        };

        template <typename ClassType, typename ReturnType, typename Arg0,
            typename... Args>
        struct lambda_first_argument<ReturnType (ClassType::*)(Arg0, Args...)
                const>
        {
            using type = typename std::decay<Arg0>::type;
        };
    }    // namespace detail

    template <typename F, typename Enable = void>
    struct first_argument
    {
    };

    // Specialization for functions
    template <typename F>
    struct first_argument<F,
        std::enable_if_t<
            std::is_function_v<typename std::remove_pointer<F>::type>>>
      : detail::function_first_argument<F>
    {
    };

    // Specialization for lambdas
    template <typename F>
    struct first_argument<F,
        std::enable_if_t<
            !std::is_function<typename std::remove_pointer<F>::type>::value>>
      : detail::lambda_first_argument<decltype(&F::operator())>
    {
    };
}}    // namespace pika::util
