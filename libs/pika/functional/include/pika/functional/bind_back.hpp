//  Copyright (c) 2011-2012 Thomas Heller
//  Copyright (c) 2013-2019 Agustin Berge
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#include <pika/datastructures/member_pack.hpp>
#include <pika/functional/invoke.hpp>
#include <pika/functional/invoke_result.hpp>
#include <pika/functional/one_shot.hpp>
#include <pika/functional/traits/get_function_address.hpp>
#include <pika/functional/traits/get_function_annotation.hpp>
#include <pika/type_support/decay.hpp>
#include <pika/type_support/pack.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace pika { namespace util {
    namespace detail {
        template <typename F, typename Ts, typename... Us>
        struct invoke_bound_back_result;

        template <typename F, typename... Ts, typename... Us>
        struct invoke_bound_back_result<F, util::pack<Ts...>, Us...>
          : util::invoke_result<F, Us..., Ts...>
        {
        };

        ///////////////////////////////////////////////////////////////////////
        template <typename F, typename Is, typename... Ts>
        class bound_back;

        template <typename F, std::size_t... Is, typename... Ts>
        class bound_back<F, index_pack<Is...>, Ts...>
        {
        public:
            template <typename F_, typename... Ts_,
                typename = typename std::enable_if<
                    std::is_constructible<F, F_>::value>::type>
            constexpr explicit bound_back(F_&& f, Ts_&&... vs)
              : _f(PIKA_FORWARD(F_, f))
              , _args(std::piecewise_construct, PIKA_FORWARD(Ts_, vs)...)
            {
            }

#if !defined(__NVCC__) && !defined(__CUDACC__)
            bound_back(bound_back const&) = default;
            bound_back(bound_back&&) = default;
#else
            PIKA_NVCC_PRAGMA_HD_WARNING_DISABLE
            constexpr PIKA_HOST_DEVICE bound_back(bound_back const& other)
              : _f(other._f)
              , _args(other._args)
            {
            }

            PIKA_NVCC_PRAGMA_HD_WARNING_DISABLE
            constexpr PIKA_HOST_DEVICE bound_back(bound_back&& other)
              : _f(PIKA_MOVE(other._f))
              , _args(PIKA_MOVE(other._args))
            {
            }
#endif

            bound_back& operator=(bound_back const&) = delete;

            PIKA_NVCC_PRAGMA_HD_WARNING_DISABLE
            template <typename... Us>
            constexpr PIKA_HOST_DEVICE typename invoke_bound_back_result<F&,
                util::pack<Ts&...>, Us&&...>::type
            operator()(Us&&... vs) &
            {
                return PIKA_INVOKE(
                    _f, PIKA_FORWARD(Us, vs)..., _args.template get<Is>()...);
            }

            PIKA_NVCC_PRAGMA_HD_WARNING_DISABLE
            template <typename... Us>
            constexpr PIKA_HOST_DEVICE
                typename invoke_bound_back_result<F const&,
                    util::pack<Ts const&...>, Us&&...>::type
                operator()(Us&&... vs) const&
            {
                return PIKA_INVOKE(
                    _f, PIKA_FORWARD(Us, vs)..., _args.template get<Is>()...);
            }

            PIKA_NVCC_PRAGMA_HD_WARNING_DISABLE
            template <typename... Us>
            constexpr PIKA_HOST_DEVICE typename invoke_bound_back_result<F&&,
                util::pack<Ts&&...>, Us&&...>::type
            operator()(Us&&... vs) &&
            {
                return PIKA_INVOKE(PIKA_MOVE(_f), PIKA_FORWARD(Us, vs)...,
                    PIKA_MOVE(_args).template get<Is>()...);
            }

            PIKA_NVCC_PRAGMA_HD_WARNING_DISABLE
            template <typename... Us>
            constexpr PIKA_HOST_DEVICE
                typename invoke_bound_back_result<F const&&,
                    util::pack<Ts const&&...>, Us&&...>::type
                operator()(Us&&... vs) const&&
            {
                return PIKA_INVOKE(PIKA_MOVE(_f), PIKA_FORWARD(Us, vs)...,
                    PIKA_MOVE(_args).template get<Is>()...);
            }

            constexpr std::size_t get_function_address() const
            {
                return traits::get_function_address<F>::call(_f);
            }

            constexpr char const* get_function_annotation() const
            {
#if defined(PIKA_HAVE_THREAD_DESCRIPTION)
                return traits::get_function_annotation<F>::call(_f);
#else
                return nullptr;
#endif
            }

#if PIKA_HAVE_ITTNOTIFY != 0 && !defined(PIKA_HAVE_APEX)
            util::itt::string_handle get_function_annotation_itt() const
            {
#if defined(PIKA_HAVE_THREAD_DESCRIPTION)
                return traits::get_function_annotation_itt<F>::call(_f);
#else
                static util::itt::string_handle sh("bound_back");
                return sh;
#endif
            }
#endif

        private:
            F _f;
            util::member_pack_for<Ts...> _args;
        };
    }    // namespace detail

    template <typename F, typename... Ts>
    constexpr detail::bound_back<typename std::decay<F>::type,
        typename util::make_index_pack<sizeof...(Ts)>::type,
        typename util::decay_unwrap<Ts>::type...>
    bind_back(F&& f, Ts&&... vs)
    {
        typedef detail::bound_back<typename std::decay<F>::type,
            typename util::make_index_pack<sizeof...(Ts)>::type,
            typename util::decay_unwrap<Ts>::type...>
            result_type;

        return result_type(PIKA_FORWARD(F, f), PIKA_FORWARD(Ts, vs)...);
    }

    // nullary functions do not need to be bound again
    template <typename F>
    constexpr typename std::decay<F>::type bind_back(F&& f)
    {
        return PIKA_FORWARD(F, f);
    }
}}    // namespace pika::util

///////////////////////////////////////////////////////////////////////////////
namespace pika { namespace traits {
    ///////////////////////////////////////////////////////////////////////////
#if defined(PIKA_HAVE_THREAD_DESCRIPTION)
    template <typename F, typename... Ts>
    struct get_function_address<util::detail::bound_back<F, Ts...>>
    {
        static constexpr std::size_t call(
            util::detail::bound_back<F, Ts...> const& f) noexcept
        {
            return f.get_function_address();
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename... Ts>
    struct get_function_annotation<util::detail::bound_back<F, Ts...>>
    {
        static constexpr char const* call(
            util::detail::bound_back<F, Ts...> const& f) noexcept
        {
            return f.get_function_annotation();
        }
    };

#if PIKA_HAVE_ITTNOTIFY != 0 && !defined(PIKA_HAVE_APEX)
    template <typename F, typename... Ts>
    struct get_function_annotation_itt<util::detail::bound_back<F, Ts...>>
    {
        static util::itt::string_handle call(
            util::detail::bound_back<F, Ts...> const& f) noexcept
        {
            return f.get_function_annotation_itt();
        }
    };
#endif
#endif
}}    // namespace pika::traits
