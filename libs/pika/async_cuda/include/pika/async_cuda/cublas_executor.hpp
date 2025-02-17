//  Copyright (c) 2020 John Biddiscombe
//  Copyright (c) 2020 Teodor Nikolov
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#if defined(PIKA_HAVE_GPU_SUPPORT)
#include <pika/async_cuda/cublas_exception.hpp>
#include <pika/async_cuda/cuda_exception.hpp>
#include <pika/async_cuda/cuda_executor.hpp>
#include <pika/async_cuda/cuda_future.hpp>
#include <pika/async_cuda/target.hpp>
#include <pika/errors/try_catch_exception_ptr.hpp>
#include <pika/execution_base/execution.hpp>
#include <pika/execution_base/traits/is_executor.hpp>
#include <pika/futures/future.hpp>

// CUDA runtime
#include <pika/async_cuda/custom_gpu_api.hpp>
// CuBLAS
#include <pika/async_cuda/custom_blas_api.hpp>
//
#include <cstddef>
#include <exception>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace pika { namespace cuda { namespace experimental {
    namespace detail {
        using print_on = debug::enable_print<false>;
        static constexpr print_on cub_debug("CUBLAS_");

        // specialization for return type of cublasStatus_t
        template <typename... Args>
        struct dispatch_helper<cublasStatus_t, Args...>
        {
            inline cublasStatus_t operator()(
                cublasStatus_t (*f)(Args...), Args... args)
            {
                cublasStatus_t err = f(args...);
                return check_cublas_error(err);
            }
        };

        struct cublas_handle
        {
            static cublasHandle_t create()
            {
                cublasHandle_t handle;
                check_cublas_error(cublasCreate(&handle));
                return handle;
            }

            // deleter for shared_ptr
            void operator()(cublasHandle_t handle) const
            {
                check_cublas_error(cublasDestroy(handle));
            }
        };
    }    // namespace detail

    // -------------------------------------------------------------------------
    // a simple cublas wrapper helper object that can be used to synchronize
    // cublas calls with an pika future.
    // -------------------------------------------------------------------------
    struct cublas_executor : cuda_executor
    {
#ifdef PIKA_HAVE_HIP
        // hipblas handle is type : void*
        using handle_ptr = std::shared_ptr<void>;
#else
        // cublas handle is type : struct cublasContext *
        using handle_ptr = std::shared_ptr<struct cublasContext>;
#endif

        // construct a cublas stream
        cublas_executor(std::size_t device,
            cublasPointerMode_t pointer_mode = CUBLAS_POINTER_MODE_HOST,
            bool event_mode = false)
          : pika::cuda::experimental::cuda_executor(device, event_mode)
          , pointer_mode_(pointer_mode)
        {
            detail::cub_debug.debug(
                debug::str<>("cublas_executor"), "event mode", event_mode);

            handle_ = handle_ptr(
                detail::cublas_handle::create(), detail::cublas_handle{});
        }

        ~cublas_executor() {}

        // -------------------------------------------------------------------------
        // OneWay Execution
        // -------------------------------------------------------------------------
        template <typename F, typename... Ts>
        decltype(auto) post(F&& f, Ts&&... ts)
        {
            return cublas_executor::apply(
                PIKA_FORWARD(F, f), PIKA_FORWARD(Ts, ts)...);
        }

        // -------------------------------------------------------------------------
        // TwoWay Execution
        // -------------------------------------------------------------------------
        template <typename F, typename... Ts>
        decltype(auto) async_execute(F&& f, Ts&&... ts)
        {
            return cublas_executor::async(
                PIKA_FORWARD(F, f), PIKA_FORWARD(Ts, ts)...);
        }

    protected:
        // This is a simple wrapper for any cublas call, pass in the same arguments
        // that you would use for a cublas call except the cublas handle which is omitted
        // as the wrapper will supply that for you
        template <typename R, typename... Params, typename... Args>
        typename std::enable_if<std::is_same<cublasStatus_t, R>::value, R>::type
        apply(R (*cublas_function)(Params...), Args&&... args)
        {
            // make sure we run on the correct device
            check_cuda_error(cudaSetDevice(device_));
            // make sure this operation takes place on our stream
            check_cublas_error(cublasSetStream(handle_.get(), stream_));
            check_cublas_error(
                cublasSetPointerMode(handle_.get(), pointer_mode_));
            // insert the cublas handle in the arg list and call the cublas function
            detail::dispatch_helper<R, Params...> helper{};
            return helper(
                cublas_function, handle_.get(), PIKA_FORWARD(Args, args)...);
        }

        // -------------------------------------------------------------------------
        // forward a cuda function through to the cuda executor base class
        // (we permit the use of a cublas executor for cuda calls)
        template <typename R, typename... Params, typename... Args>
        inline typename std::enable_if<std::is_same<cudaError_t, R>::value,
            void>::type
        apply(R (*cuda_function)(Params...), Args&&... args)
        {
            return cuda_executor::apply(
                cuda_function, PIKA_FORWARD(Args, args)...);
        }

        // -------------------------------------------------------------------------
        // launch a cuBlas function and return a future that will become ready
        // when the task completes, this allows integration of GPU kernels with
        // pika::futures and the tasking DAG.
        template <typename R, typename... Params, typename... Args>
        pika::future<typename std::enable_if<
            std::is_same<cublasStatus_t, R>::value, void>::type>
        async(R (*cublas_function)(Params...), Args&&... args)
        {
            return pika::detail::try_catch_exception_ptr(
                [&]() {
                    // make sure we run on the correct device
                    check_cuda_error(cudaSetDevice(device_));
                    // make sure this operation takes place on our stream
                    check_cublas_error(cublasSetStream(handle_.get(), stream_));
                    // insert the cublas handle in the arg list and call the
                    // cublas function
                    detail::dispatch_helper<R, Params...> helper;
                    helper(cublas_function, handle_.get(),
                        PIKA_FORWARD(Args, args)...);
                    return get_future();
                },
                [&](std::exception_ptr&& ep) {
                    return pika::make_exceptional_future<void>(PIKA_MOVE(ep));
                });
        }

        // -------------------------------------------------------------------------
        // forward a cuda function through to the cuda executor base class
        template <typename R, typename... Params, typename... Args>
        inline pika::future<typename std::enable_if<
            std::is_same<cudaError_t, R>::value, void>::type>
        async(R (*cuda_function)(Params...), Args&&... args)
        {
            return cuda_executor::async(
                cuda_function, PIKA_FORWARD(Args, args)...);
        }

        // return a copy of the cublas handle
        cublasHandle_t get_handle()
        {
            return handle_.get();
        }

    protected:
        handle_ptr handle_;
        cublasPointerMode_t pointer_mode_;
    };

}}}    // namespace pika::cuda::experimental

namespace pika { namespace parallel { namespace execution {
    /// \cond NOINTERNAL
    template <>
    struct is_one_way_executor<pika::cuda::experimental::cublas_executor>
      : std::true_type
    {
        // we support fire and forget without returning a waitable/future
    };

    template <>
    struct is_two_way_executor<pika::cuda::experimental::cublas_executor>
      : std::true_type
    {
        // we support returning a waitable/future
    };
    /// \endcond
}}}    // namespace pika::parallel::execution
#endif
