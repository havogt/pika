//  Copyright (c) 2011 Bryce Lelbach
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/barrier.hpp>
#include <pika/init.hpp>
#include <pika/modules/allocator_support.hpp>
#include <pika/modules/format.hpp>
#include <pika/runtime.hpp>
#include <pika/thread.hpp>

#include <boost/lockfree/queue.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>

template <typename T>
using queue =
    boost::lockfree::queue<T, pika::detail::aligned_allocator<std::size_t>>;

using pika::program_options::options_description;
using pika::program_options::value;
using pika::program_options::variables_map;

using pika::lcos::local::barrier;

using pika::threads::register_work;
using pika::threads::thread_init_data;

///////////////////////////////////////////////////////////////////////////////
// we use globals here to prevent the delay from being optimized away
double global_scratch = 0;
std::uint64_t num_iterations = 0;

///////////////////////////////////////////////////////////////////////////////
double delay()
{
    double d = 0.;
    for (std::uint64_t i = 0; i < num_iterations; ++i)
        d += 1 / (2. * i + 1);
    return d;
}

///////////////////////////////////////////////////////////////////////////////
void get_os_thread_num(barrier& barr, queue<std::size_t>& os_threads)
{
    global_scratch = delay();
    os_threads.push(pika::get_worker_thread_num());
    barr.wait();
}

///////////////////////////////////////////////////////////////////////////////
using result_map = std::map<std::size_t, std::size_t>;

typedef std::multimap<std::size_t, std::size_t, std::greater<std::size_t>>
    sorter;

///////////////////////////////////////////////////////////////////////////////
int pika_main(variables_map& vm)
{
    {
        num_iterations = vm["delay-iterations"].as<std::uint64_t>();

        const bool csv = vm.count("csv");

        const std::size_t pxthreads = vm["pxthreads"].as<std::size_t>();

        result_map results;

        {
            // Have the queue preallocate the nodes.
            queue<std::size_t> os_threads(pxthreads);

            barrier barr(pxthreads + 1);

            for (std::size_t j = 0; j < pxthreads; ++j)
            {
                thread_init_data data(
                    pika::threads::make_thread_function_nullary(
                        pika::util::bind(&get_os_thread_num, std::ref(barr),
                            std::ref(os_threads))),
                    "get_os_thread_num", pika::threads::thread_priority::normal,
                    pika::threads::thread_schedule_hint(0));
                register_work(data);
            }

            barr.wait();    // wait for all PX threads to enter the barrier

            std::size_t shepherd = 0;

            while (os_threads.pop(shepherd))
                ++results[shepherd];
        }

        sorter sort;

        for (result_map::value_type const& result : results)
        {
            sort.insert(sorter::value_type(result.second, result.first));
        }

        for (sorter::value_type const& result : sort)
        {
            if (csv)
                pika::util::format_to(
                    std::cout, "{1},{2}\n", result.second, result.first)
                    << std::flush;
            else
                pika::util::format_to(std::cout,
                    "OS-thread {1} ran {2} PX-threads\n", result.second,
                    result.first)
                    << std::flush;
        }
    }

    // initiate shutdown of the runtime system
    pika::finalize();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Configure application-specific options
    options_description cmdline("Usage: " PIKA_APPLICATION_STRING " [options]");

    cmdline.add_options()("pxthreads", value<std::size_t>()->default_value(128),
        "number of PX-threads to invoke")

        ("delay-iterations", value<std::uint64_t>()->default_value(65536),
            "number of iterations in the delay loop")

            ("csv", "output results as csv (format: OS-thread,PX-threads)");

    // Initialize and run pika
    pika::init_params init_args;
    init_args.desc_cmdline = cmdline;

    return pika::init(pika_main, argc, argv, init_args);
}
