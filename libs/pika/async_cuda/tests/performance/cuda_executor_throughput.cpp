//  Copyright (c) 2017-2018 John Biddiscombe
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// For compliance with the NVIDIA EULA:
// "This software contains source code provided by NVIDIA Corporation."

// This is a conversion of the NVIDIA cublas example matrixMulCUBLAS to use
// pika style data structures, executors and futures and demonstrate a simple use
// of computing a number of iteration of a matrix multiply on a stream and returning
// a future when it completes. This can be used to chain/schedule other task
// in a manner consistent with the future based API of pika.
//
// Example usage: bin/cublas_matmul --sizemult=10 --iterations=25 --pika:threads=8
// NB. The pika::threads param only controls how many parallel tasks to use for the CPU
// comparison/checks and makes no difference to the GPU execution.
//
// Note: The pika::cuda::experimental::allocator makes use of device code and if used
// this example must be compiled with nvcc instead of c++ which requires the following
// cmake setting
// set_source_files_properties(cublas_matmul.cpp
//     PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ)
// Currently, nvcc does not handle lambda functions properly and it is simpler to use
// cudaMalloc/cudaMemcpy etc, so we do not #define PIKA_CUBLAS_DEMO_WITH_ALLOCATOR

#include <pika/algorithm.hpp>
#include <pika/execution.hpp>
#include <pika/init.hpp>
#include <pika/modules/async_cuda.hpp>
#include <pika/modules/timing.hpp>
#include <pika/parallel/algorithms/copy.hpp>
#include <pika/parallel/algorithms/for_each.hpp>
#include <pika/parallel/algorithms/for_loop.hpp>
#include <pika/testing.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
//
std::mt19937 gen;

// -------------------------------------------------------------------------
// Optional Command-line multiplier for matrix sizes
struct sMatrixSize
{
    unsigned int uiWA, uiHA, uiWB, uiHB, uiWC, uiHC;
};

// -------------------------------------------------------------------------
// Run a simple test matrix multiply using CUBLAS
// -------------------------------------------------------------------------
template <typename T>
void matrixMultiply(
    sMatrixSize& matrix_size, std::size_t device, std::size_t iterations)
{
    using pika::execution::par;

    // Allocate host memory for matrices A and B
    unsigned int size_A = matrix_size.uiWA * matrix_size.uiHA;
    unsigned int size_B = matrix_size.uiWB * matrix_size.uiHB;
    unsigned int size_C = matrix_size.uiWC * matrix_size.uiHC;

    std::vector<T> h_A(size_A);
    std::vector<T> h_B(size_B);
    std::vector<T> h_C(size_C);
    std::vector<T> h_CUBLAS(size_C);

    // Fill A and B with zeroes
    auto zerofunc = [](T& x) { x = 0; };
    pika::for_each(par, h_A.begin(), h_A.end(), zerofunc);
    pika::for_each(par, h_B.begin(), h_B.end(), zerofunc);

    // create a cublas executor we'll use to futurize cuda events
    pika::cuda::experimental::cublas_executor cublas(device,
        CUBLAS_POINTER_MODE_HOST, pika::cuda::experimental::callback_mode{});
    using cublas_future =
        typename pika::cuda::experimental::cublas_executor::future_type;

    T *d_A, *d_B, *d_C;
    pika::cuda::experimental::check_cuda_error(
        cudaMalloc((void**) &d_A, size_A * sizeof(T)));
    pika::cuda::experimental::check_cuda_error(
        cudaMalloc((void**) &d_B, size_B * sizeof(T)));
    pika::cuda::experimental::check_cuda_error(
        cudaMalloc((void**) &d_C, size_C * sizeof(T)));

    // copy A to device, no future
    pika::apply(cublas, cudaMemcpyAsync, d_A, h_A.data(), size_A * sizeof(T),
        cudaMemcpyHostToDevice);

    // copy B to device on same stream as A, get a future back
    auto copy_future = pika::async(cublas, cudaMemcpyAsync, d_B, h_B.data(),
        size_B * sizeof(T), cudaMemcpyHostToDevice);

    // print out somethine when copy completes and wait for it
    copy_future
        .then([](cublas_future&&) {
            std::cout << "Async host->device copy operation completed"
                      << std::endl
                      << std::endl;
        })
        .get();

    std::cout << "Small matrix multiply tests using CUBLAS...\n\n";
    const T alpha = 1.0f;
    const T beta = 0.0f;

    auto test_function = [&](pika::cuda::experimental::cublas_executor& exec,
                             const std::string& msg, std::size_t n_iters) {
        // time many cuda kernels spawned one after each other when they complete
        pika::future<void> f;
        pika::chrono::high_resolution_timer t1;
        for (std::size_t j = 0; j < n_iters; j++)
        {
            f = pika::async(exec, cublasSgemm, CUBLAS_OP_N, CUBLAS_OP_N,
                matrix_size.uiWB, matrix_size.uiHA, matrix_size.uiWA, &alpha,
                d_B, matrix_size.uiWB, d_A, matrix_size.uiWA, &beta, d_C,
                matrix_size.uiWA);
            f.get();
        }
        double us1 = t1.elapsed_microseconds();
        std::cout << "us per iteration " << us1 / n_iters << " : " << msg
                  << std::endl
                  << std::endl;
    };

    // call our test function using a callback style executor
    pika::cuda::experimental::cublas_executor exec_callback(
        0, CUBLAS_POINTER_MODE_HOST, pika::cuda::experimental::callback_mode{});
    test_function(exec_callback, "Warmup", 100);
    test_function(exec_callback, "Callback based executor", iterations);

    // call test function with an event based one, remember to install
    // the polling handler as well
    {
        // install cuda future polling handler for this scope block
        pika::cuda::experimental::enable_user_polling poll("default");

        pika::cuda::experimental::cublas_executor exec_event(0,
            CUBLAS_POINTER_MODE_HOST, pika::cuda::experimental::event_mode{});
        test_function(exec_event, "Event polling based executor", iterations);
    }

    pika::cuda::experimental::check_cuda_error(cudaFree(d_A));
    pika::cuda::experimental::check_cuda_error(cudaFree(d_B));
    pika::cuda::experimental::check_cuda_error(cudaFree(d_C));
}

// -------------------------------------------------------------------------
int pika_main(pika::program_options::variables_map& vm)
{
    std::size_t device = vm["device"].as<std::size_t>();
    std::size_t iterations = vm["iterations"].as<std::size_t>();
    //
    int sizeMult = 1;

    pika::cuda::experimental::target target(device);
    std::cout << "GPU Device " << target.native_handle().get_device() << ": \""
              << target.native_handle().processor_name() << "\" "
              << "with compute capability "
              << target.native_handle().processor_family() << "\n";

    int block_size = 4;
    sMatrixSize matrix_size;
    matrix_size.uiWA = 1 * block_size * sizeMult;
    matrix_size.uiHA = 1 * block_size * sizeMult;
    matrix_size.uiWB = 1 * block_size * sizeMult;
    matrix_size.uiHB = 1 * block_size * sizeMult;
    matrix_size.uiWC = 1 * block_size * sizeMult;
    matrix_size.uiHC = 1 * block_size * sizeMult;

    printf("MatrixA(%u,%u), MatrixB(%u,%u), MatrixC(%u,%u)\n\n",
        matrix_size.uiWA, matrix_size.uiHA, matrix_size.uiWB, matrix_size.uiHB,
        matrix_size.uiWC, matrix_size.uiHC);

    matrixMultiply<float>(matrix_size, device, iterations);
    return pika::finalize();
}

// -------------------------------------------------------------------------
int main(int argc, char** argv)
{
    printf("[pika CUBLAS executor benchmark] - Starting...\n");

    using namespace pika::program_options;
    options_description cmdline("usage: " PIKA_APPLICATION_STRING " [options]");
    // clang-format off
    cmdline.add_options()
        ("device",
        pika::program_options::value<std::size_t>()->default_value(0),
        "Device to use")
        ("iterations",
        pika::program_options::value<std::size_t>()->default_value(30),
        "iterations");
    // clang-format on

    pika::init_params init_args;
    init_args.desc_cmdline = cmdline;

    auto result = pika::init(pika_main, argc, argv, init_args);
    return result || pika::util::report_errors();
}
