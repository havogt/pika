//  Copyright (c) 2015 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/execution.hpp>
#include <pika/init.hpp>
#include <pika/runtime/custom_exception_info.hpp>
#include <pika/task_block.hpp>
#include <pika/testing.hpp>

#include <iostream>
#include <string>
#include <vector>

using pika::execution::par;
using pika::execution::parallel_policy;
using pika::execution::parallel_policy_shim;
using pika::execution::parallel_task_policy;
using pika::execution::parallel_task_policy_shim;
using pika::execution::static_chunk_size;
using pika::execution::task;
using pika::parallel::define_task_block;
using pika::parallel::task_block;

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
void define_task_block_test1(Executor& exec)
{
    std::string s("test");

    bool parent_flag = false;
    bool task1_flag = false;
    bool task2_flag = false;
    bool task21_flag = false;
    bool task3_flag = false;

    typedef task_block<parallel_policy_shim<Executor, static_chunk_size>>
        task_block_type;

    define_task_block(par.on(exec), [&](task_block_type& trh) {
        parent_flag = true;

        trh.run([&]() {
            task1_flag = true;
            std::cout << "task1: " << s << std::endl;
        });

        trh.run([&]() {
            task2_flag = true;
            std::cout << "task2" << std::endl;

            define_task_block(par.on(exec), [&](task_block_type& trh) {
                trh.run([&]() {
                    task21_flag = true;
                    std::cout << "task2.1" << std::endl;
                });
            });
        });

        int i = 0, j = 10, k = 20;
        trh.run([=, &task3_flag]() {
            task3_flag = true;
            std::cout << "task3: " << i << " " << j << " " << k << std::endl;
        });

        std::cout << "parent" << std::endl;
    });

    PIKA_TEST(parent_flag);
    PIKA_TEST(task1_flag);
    PIKA_TEST(task2_flag);
    PIKA_TEST(task21_flag);
    PIKA_TEST(task3_flag);
}

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
void define_task_block_test2(Executor& exec)
{
    std::string s("test");

    bool parent_flag = false;
    bool task1_flag = false;
    bool task2_flag = false;
    bool task21_flag = false;
    bool task3_flag = false;

    typedef task_block<parallel_policy_shim<Executor, static_chunk_size>>
        task_block_type1;
    typedef task_block<parallel_task_policy_shim<Executor, static_chunk_size>>
        task_block_type2;

    pika::future<void> f =
        define_task_block(par(task).on(exec), [&](task_block_type2& trh) {
            parent_flag = true;

            trh.run([&]() {
                task1_flag = true;
                std::cout << "task1: " << s << std::endl;
            });

            trh.run([&]() {
                task2_flag = true;
                std::cout << "task2" << std::endl;

                define_task_block(par.on(exec), [&](task_block_type1& trh) {
                    trh.run([&]() {
                        task21_flag = true;
                        std::cout << "task2.1" << std::endl;
                    });
                });
            });

            int i = 0, j = 10, k = 20;
            trh.run([=, &task3_flag]() {
                task3_flag = true;
                std::cout << "task3: " << i << " " << j << " " << k
                          << std::endl;
            });

            std::cout << "parent" << std::endl;
        });

    f.wait();

    PIKA_TEST(parent_flag);
    PIKA_TEST(task1_flag);
    PIKA_TEST(task2_flag);
    PIKA_TEST(task21_flag);
    PIKA_TEST(task3_flag);
}

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
void define_task_block_test3(Executor& exec)
{
    std::string s("test");

    bool parent_flag = false;
    bool task1_flag = false;
    bool task2_flag = false;
    bool task21_flag = false;
    bool task3_flag = false;

    typedef task_block<parallel_policy_shim<Executor, static_chunk_size>>
        task_block_type;

    define_task_block(par.on(exec), [&](task_block_type& trh) {
        parent_flag = true;

        trh.run([&]() {
            task1_flag = true;
            std::cout << "task1: " << s << std::endl;
        });

        trh.run([&]() {
            task2_flag = true;
            std::cout << "task2" << std::endl;

            define_task_block(par.on(exec), [&](task_block_type& trh) {
                trh.run(exec, [&]() {
                    task21_flag = true;
                    std::cout << "task2.1" << std::endl;
                });
            });
        });

        int i = 0, j = 10, k = 20;
        trh.run(exec, [=, &task3_flag]() {
            task3_flag = true;
            std::cout << "task3: " << i << " " << j << " " << k << std::endl;
        });

        std::cout << "parent" << std::endl;
    });

    PIKA_TEST(parent_flag);
    PIKA_TEST(task1_flag);
    PIKA_TEST(task2_flag);
    PIKA_TEST(task21_flag);
    PIKA_TEST(task3_flag);
}

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
void define_task_block_test4(Executor& exec)
{
    std::string s("test");

    bool parent_flag = false;
    bool task1_flag = false;
    bool task2_flag = false;
    bool task21_flag = false;
    bool task3_flag = false;

    typedef task_block<parallel_policy_shim<Executor, static_chunk_size>>
        task_block_type1;
    typedef task_block<parallel_task_policy_shim<Executor, static_chunk_size>>
        task_block_type2;

    pika::future<void> f =
        define_task_block(par(task).on(exec), [&](task_block_type2& trh) {
            parent_flag = true;

            trh.run(exec, [&]() {
                task1_flag = true;
                std::cout << "task1: " << s << std::endl;
            });

            trh.run([&]() {
                task2_flag = true;
                std::cout << "task2" << std::endl;

                define_task_block(par.on(exec), [&](task_block_type1& trh) {
                    trh.run(exec, [&]() {
                        task21_flag = true;
                        std::cout << "task2.1" << std::endl;
                    });
                });
            });

            int i = 0, j = 10, k = 20;
            trh.run(exec, [=, &task3_flag]() {
                task3_flag = true;
                std::cout << "task3: " << i << " " << j << " " << k
                          << std::endl;
            });

            std::cout << "parent" << std::endl;
        });

    f.wait();

    PIKA_TEST(parent_flag);
    PIKA_TEST(task1_flag);
    PIKA_TEST(task2_flag);
    PIKA_TEST(task21_flag);
    PIKA_TEST(task3_flag);
}

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
void define_task_block_exceptions_test1(Executor& exec)
{
    typedef task_block<parallel_policy_shim<Executor, static_chunk_size>>
        task_block_type;

    try
    {
        define_task_block(par.on(exec), [](task_block_type& trh) {
            trh.run([]() {
                std::cout << "task1" << std::endl;
                throw 1;
            });

            trh.run([]() {
                std::cout << "task2" << std::endl;
                throw 2;
            });

            std::cout << "parent" << std::endl;
            throw 100;
        });

        PIKA_TEST(false);
    }
    catch (pika::parallel::exception_list const& e)
    {
        PIKA_TEST_EQ(e.size(), 3u);
    }
    catch (...)
    {
        PIKA_TEST(false);
    }
}

template <typename Executor>
void define_task_block_exceptions_test2(Executor& exec)
{
    typedef task_block<parallel_task_policy_shim<Executor, static_chunk_size>>
        task_block_type;

    pika::future<void> f =
        define_task_block(par(task).on(exec), [](task_block_type& trh) {
            trh.run([]() {
                std::cout << "task1" << std::endl;
                throw 1;
            });

            trh.run([]() {
                std::cout << "task2" << std::endl;
                throw 2;
            });

            std::cout << "parent" << std::endl;
            throw 100;
        });

    try
    {
        f.get();
        PIKA_TEST(false);
    }
    catch (pika::parallel::exception_list const& e)
    {
        PIKA_TEST_EQ(e.size(), 3u);
    }
    catch (...)
    {
        PIKA_TEST(false);
    }
}

template <typename Executor>
void define_task_block_exceptions_test3(Executor& exec)
{
    typedef task_block<parallel_policy_shim<Executor, static_chunk_size>>
        task_block_type;

    try
    {
        define_task_block(par.on(exec), [&](task_block_type& trh) {
            trh.run([&]() {
                PIKA_TEST(!pika::expect_exception());

                // Error: trh is not active
                trh.run([]() {
                    PIKA_TEST(false);    // should not be called
                });

                PIKA_TEST(false);

                PIKA_TEST(pika::expect_exception(false));
            });
        });

        PIKA_TEST(false);
    }
    catch (pika::exception const& e)
    {
        PIKA_TEST_EQ(int(e.get_error()), int(pika::task_block_not_active));
    }
    catch (...)
    {
        PIKA_TEST(false);
    }
}

template <typename Executor>
void define_task_block_exceptions_test4(Executor& exec)
{
    typedef task_block<parallel_task_policy_shim<Executor, static_chunk_size>>
        task_block_type;

    pika::future<void> f =
        define_task_block(par(task).on(exec), [&](task_block_type& trh) {
            trh.run([&]() {
                // Error: tr is not active
                trh.run([]() {
                    PIKA_TEST(false);    // should not be called
                });

                PIKA_TEST(false);
            });
        });

    try
    {
        f.get();
        PIKA_TEST(false);
    }
    catch (pika::exception const& e)
    {
        PIKA_TEST_EQ(int(e.get_error()), int(pika::task_block_not_active));
    }
    catch (...)
    {
        PIKA_TEST(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
template <typename Executor>
void test_executor_task_block(Executor& exec)
{
    define_task_block_test1(exec);
    define_task_block_test2(exec);
    define_task_block_test3(exec);
    define_task_block_test4(exec);

    define_task_block_exceptions_test1(exec);
    define_task_block_exceptions_test2(exec);
}

int pika_main()
{
    {
        pika::execution::sequenced_executor exec;
        test_executor_task_block(exec);
    }

    {
        pika::execution::parallel_executor exec;
        test_executor_task_block(exec);

        define_task_block_exceptions_test3(exec);
        define_task_block_exceptions_test4(exec);
    }

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
