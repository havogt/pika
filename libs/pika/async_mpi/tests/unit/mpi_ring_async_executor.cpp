//  Copyright (c) 2019 John Biddiscombe
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/execution.hpp>
#include <pika/future.hpp>
#include <pika/init.hpp>
#include <pika/mpi.hpp>
#include <pika/program_options.hpp>
#include <pika/testing.hpp>

#include <array>
#include <atomic>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <mpi.h>

// This test send a message from rank 0 to rank 1 and from R1->R2 in a ring
// until the last rank which sends it back to R0.and completes an iteration
//
// For benchmarking the test has been extended to allow many iterations
// but unfortunately, if we prepost too many receives, MPI has problems
// so we do 1000 iterations per main loop and another loop around that.

using pika::program_options::options_description;
using pika::program_options::value;
using pika::program_options::variables_map;

static bool output = true;

void msg_recv(int rank, int size, int /*to*/, int from, int token, unsigned tag)
{
    // to reduce string corruption on stdout from multiple threads
    // writing simultaneously, we use a stringstream as a buffer
    if (output)
    {
        std::ostringstream temp;
        temp << "Rank " << std::setfill(' ') << std::setw(3) << rank << " of "
             << std::setfill(' ') << std::setw(3) << size << " Recv token "
             << std::setfill(' ') << std::setw(3) << token << " from rank "
             << std::setfill(' ') << std::setw(3) << from << " tag "
             << std::setfill(' ') << std::setw(3) << tag;
        std::cout << temp.str() << std::endl;
    }
}

void msg_send(int rank, int size, int to, int /*from*/, int token, unsigned tag)
{
    if (output)
    {
        std::ostringstream temp;
        temp << "Rank " << std::setfill(' ') << std::setw(3) << rank << " of "
             << std::setfill(' ') << std::setw(3) << size << " Sent token "
             << std::setfill(' ') << std::setw(3) << token << " to   rank "
             << std::setfill(' ') << std::setw(3) << to << " tag "
             << std::setfill(' ') << std::setw(3) << tag;
        std::cout << temp.str() << std::endl;
    }
}

// this is called on an pika thread after the runtime starts up
int pika_main(pika::program_options::variables_map& vm)
{
    int rank, size;
    //
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // if comm size < 2 this test should fail
    // it needs to run on N>2 ranks to be useful
    PIKA_TEST_MSG(size > 1, "This test requires N>1 mpi ranks");

    const std::uint64_t iterations = vm["iterations"].as<std::uint64_t>();
    //
    output = vm.count("output") != 0;

    if (rank == 0 && output)
    {
        std::cout << "Rank " << std::setfill(' ') << std::setw(3) << rank
                  << " of " << std::setfill(' ') << std::setw(3) << size
                  << std::endl;
    }

    {
        // this needs to scope all uses of pika::mpi::experimental::executor
        pika::mpi::experimental::enable_user_polling enable_polling;

        // Ring send/recv around N ranks
        // Rank 0      : Send then Recv
        // Rank 1->N-1 : Recv then Send

        pika::mpi::experimental::executor exec(MPI_COMM_WORLD);

        // mpi chokes if we put too many messages into the system at once
        // we will use a limiting executor with N 'in flight' at once
        pika::execution::experimental::limiting_executor<
            pika::mpi::experimental::executor>
            limexec(exec, 32, 64, true);

        std::vector<int> tokens(iterations, -1);

        pika::chrono::high_resolution_timer t;

        std::atomic<std::uint64_t> counter(iterations);
        for (std::uint64_t i = 0; (i != iterations); ++i)
        {
            tokens[i] = (rank == 0) ? 1 : -1;
            int rank_from = (size + rank - 1) % size;
            int rank_to = (rank + 1) % size;

            // all ranks pre-post a receive
            pika::future<int> f_recv = pika::async(
                limexec, MPI_Irecv, &tokens[i], 1, MPI_INT, rank_from, i);

            // when the recv completes,
            f_recv.then([=, &exec, &tokens, &counter](auto&&) {
                msg_recv(rank, size, rank_to, rank_from, tokens[i], i);
                if (rank > 0)
                {
                    // send the incremented token to the next rank
                    ++tokens[i];
                    pika::future<int> f_send = pika::async(
                        exec, MPI_Isend, &tokens[i], 1, MPI_INT, rank_to, i);
                    // when the send completes
                    f_send.then([=, &tokens, &counter](auto&&) {
                        msg_send(rank, size, rank_to, rank_from, tokens[i], i);
                        // ranks > 0 are done when they have sent their token
                        --counter;
                    });
                }
                else
                {
                    // rank 0 is done when it receives its token
                    --counter;
                }
            });

            // rank 0 starts the process with a send
            if (rank == 0)
            {
                auto f_send = pika::async(
                    limexec, MPI_Isend, &tokens[i], 1, MPI_INT, rank_to, i);
                f_send.then([=, &tokens](auto&&) {
                    msg_send(rank, size, rank_to, rank_from, tokens[i], i);
                });
            }
        }

        std::cout << "Reached end of test " << counter << std::endl;
        // Our simple counter should reach zero when all send/recv pairs are done
        pika::mpi::experimental::wait([&]() { return counter != 0; });

        if (rank == 0)
        {
            std::cout << "time " << t.elapsed() << std::endl;
        }

        // let the user polling go out of scope
    }
    return pika::finalize();
}

// the normal int main function that is called at startup and runs on an OS
// thread the user must call pika::init to start the pika runtime which
// will execute pika_main on an pika thread
int main(int argc, char* argv[])
{
    // if this test is run with distributed runtime, we need to make sure
    // that all ranks run their main function
    std::vector<std::string> cfg = {"pika.run_pika_main!=1"};

    // Init MPI
    int provided = MPI_THREAD_MULTIPLE;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE)
    {
        std::cout << "Provided MPI is not : MPI_THREAD_MULTIPLE " << provided
                  << std::endl;
    }

    // Configure application-specific options.
    options_description cmdline("usage: " PIKA_APPLICATION_STRING " [options]");

    // clang-format off
    cmdline.add_options()(
        "iterations",
        value<std::uint64_t>()->default_value(5000),
        "number of iterations to test")

        ("output", "display messages during test");
    // clang-format on

    // Initialize and run pika.
    pika::init_params init_args;
    init_args.desc_cmdline = cmdline;
    init_args.cfg = cfg;

    auto result = pika::init(pika_main, argc, argv, init_args);

    // Finalize MPI
    MPI_Finalize();

    return result || pika::util::report_errors();
}
