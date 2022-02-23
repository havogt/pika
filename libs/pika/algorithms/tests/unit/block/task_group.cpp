//  Copyright (c) 2021 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/parallel/task_group.hpp>
#include <pika/testing.hpp>

#include <cstddef>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
int fib(int n)
{
    if (n < 2)
    {
        return n;
    }

    int x = 0, y = 0;

    pika::execution::experimental::task_group g;
    g.run([&x, n] { x = fib(n - 1); });    // spawn a task
    g.run([&y, n] { y = fib(n - 2); });    // spawn another task
    g.wait();                              // wait for both tasks to complete

    return x + y;
}

void task_group_test1()
{
    PIKA_TEST_EQ(fib(22), 17711);
}

///////////////////////////////////////////////////////////////////////////////
int fib1(int n)
{
    if (n < 2)
    {
        return n;
    }

    int x = 0, y = 0;

    pika::execution::experimental::task_group g;
    g.run([&x, n] { x = fib1(n - 1); });    // spawn a task
    g.wait();                               // wait for both tasks to complete

    // reuse the task group
    g.run([&y, n] { y = fib1(n - 2); });    // spawn another task
    g.wait();                               // wait for both tasks to complete

    return x + y;
}

void task_group_test1_reuse()
{
    PIKA_TEST_EQ(fib(22), 17711);
}

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
int fib(Executor&& exec, int n)
{
    if (n < 2)
    {
        return n;
    }

    int x = 0, y = 0;

    pika::execution::experimental::task_group g;
    g.run(
        exec, [&](int n) { x = fib(exec, n); }, n - 1);
    g.run(
        exec, [&](int n) { y = fib(exec, n); }, n - 2);
    g.wait();    // wait for both tasks to complete

    return x + y;
}

void task_group_test2()
{
    PIKA_TEST_EQ(fib(pika::execution::parallel_executor{}, 22), 17711);
}

///////////////////////////////////////////////////////////////////////////////
void task_group_test3()
{
    bool throws_exception = true;
    bool caught_exception = false;
    try
    {
        pika::execution::experimental::task_group g;
        g.run([] { throw std::runtime_error("test1"); });
        g.run([] { throw std::runtime_error("test2"); });
        throws_exception = false;

        g.wait();    // rethrows after waiting for all tasks to finish
        PIKA_TEST(false);
    }
    catch (pika::exception_list const& l)
    {
        caught_exception = true;
        PIKA_TEST_EQ(l.size(), std::size_t(2));
    }
    catch (...)
    {
        PIKA_TEST(false);
    }
    PIKA_TEST(!throws_exception);
    PIKA_TEST(caught_exception);
}

///////////////////////////////////////////////////////////////////////////////
int pika_main()
{
    task_group_test1();
    task_group_test1_reuse();
    task_group_test2();
    task_group_test3();

    return pika::finalize();
}

int main(int argc, char* argv[])
{
    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"pika.os_threads=all"};

    // Initialize and run pika
    pika::init_params init_args;
    init_args.cfg = cfg;
    PIKA_TEST_EQ_MSG(pika::init(pika_main, argc, argv, init_args), 0,
        "pika main exited with non-zero status");

    return pika::util::report_errors();
}
