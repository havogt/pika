//  Copyright (c) 2014 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/parallel/algorithms/uninitialized_fill.hpp>
#include <pika/testing.hpp>

#include <atomic>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_uninitialized_fill(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    pika::uninitialized_fill(
        iterator(std::begin(c)), iterator(std::end(c)), 10);

    // verify values
    std::size_t count = 0;
    std::for_each(std::begin(c), std::end(c), [&count](std::size_t v) -> void {
        PIKA_TEST_EQ(v, std::size_t(10));
        ++count;
    });
    PIKA_TEST_EQ(count, c.size());
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_fill(ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    pika::uninitialized_fill(
        policy, iterator(std::begin(c)), iterator(std::end(c)), 10);

    // verify values
    std::size_t count = 0;
    std::for_each(std::begin(c), std::end(c), [&count](std::size_t v) -> void {
        PIKA_TEST_EQ(v, std::size_t(10));
        ++count;
    });
    PIKA_TEST_EQ(count, c.size());
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_fill_async(ExPolicy p, IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    pika::future<void> f = pika::uninitialized_fill(
        p, iterator(std::begin(c)), iterator(std::end(c)), 10);
    f.wait();

    std::size_t count = 0;
    std::for_each(std::begin(c), std::end(c), [&count](std::size_t v) -> void {
        PIKA_TEST_EQ(v, std::size_t(10));
        ++count;
    });
    PIKA_TEST_EQ(count, c.size());
}

template <typename IteratorTag>
void test_uninitialized_fill()
{
    using namespace pika::execution;
    test_uninitialized_fill(IteratorTag());
    test_uninitialized_fill(seq, IteratorTag());
    test_uninitialized_fill(par, IteratorTag());
    test_uninitialized_fill(par_unseq, IteratorTag());

    test_uninitialized_fill_async(seq(task), IteratorTag());
    test_uninitialized_fill_async(par(task), IteratorTag());
}

void uninitialized_fill_test()
{
    test_uninitialized_fill<std::random_access_iterator_tag>();
    test_uninitialized_fill<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_fill_exception(ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<test::count_instances>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<test::count_instances> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    std::atomic<std::size_t> throw_after(std::rand() % c.size());    //-V104
    test::count_instances::instance_count.store(0);

    bool caught_exception = false;
    try
    {
        pika::uninitialized_fill(policy,
            decorated_iterator(std::begin(c),
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::runtime_error("test");
                }),
            decorated_iterator(std::end(c)), 10);

        PIKA_TEST(false);
    }
    catch (pika::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
    }
    catch (...)
    {
        PIKA_TEST(false);
    }

    PIKA_TEST(caught_exception);
    PIKA_TEST_EQ(test::count_instances::instance_count.load(), std::size_t(0));
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_fill_exception_async(ExPolicy p, IteratorTag)
{
    using base_iterator = std::vector<test::count_instances>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<test::count_instances> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    std::atomic<std::size_t> throw_after(std::rand() % c.size());    //-V104
    test::count_instances::instance_count.store(0);

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        pika::future<void> f = pika::uninitialized_fill(p,
            decorated_iterator(std::begin(c),
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::runtime_error("test");
                }),
            decorated_iterator(std::end(c)), 10);

        returned_from_algorithm = true;
        f.get();

        PIKA_TEST(false);
    }
    catch (pika::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
    }
    catch (...)
    {
        PIKA_TEST(false);
    }

    PIKA_TEST(caught_exception);
    PIKA_TEST(returned_from_algorithm);
    PIKA_TEST_EQ(test::count_instances::instance_count.load(), std::size_t(0));
}

template <typename IteratorTag>
void test_uninitialized_fill_exception()
{
    using namespace pika::execution;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_uninitialized_fill_exception(seq, IteratorTag());
    test_uninitialized_fill_exception(par, IteratorTag());

    test_uninitialized_fill_exception_async(seq(task), IteratorTag());
    test_uninitialized_fill_exception_async(par(task), IteratorTag());
}

void uninitialized_fill_exception_test()
{
    test_uninitialized_fill_exception<std::random_access_iterator_tag>();
    test_uninitialized_fill_exception<std::forward_iterator_tag>();
}

//////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_fill_bad_alloc(ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<test::count_instances>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<test::count_instances> c(100007);
    std::iota(std::begin(c), std::end(c), std::rand());

    std::atomic<std::size_t> throw_after(std::rand() % c.size());    //-V104
    test::count_instances::instance_count.store(0);

    bool caught_bad_alloc = false;
    try
    {
        pika::uninitialized_fill(policy,
            decorated_iterator(std::begin(c),
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::bad_alloc();
                }),
            decorated_iterator(std::end(c)), 10);

        PIKA_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        PIKA_TEST(false);
    }

    PIKA_TEST(caught_bad_alloc);
    PIKA_TEST_EQ(test::count_instances::instance_count.load(), std::size_t(0));
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_fill_bad_alloc_async(ExPolicy p, IteratorTag)
{
    using base_iterator = std::vector<test::count_instances>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<test::count_instances> c(10007);
    std::iota(std::begin(c), std::end(c), std::rand());

    std::atomic<std::size_t> throw_after(std::rand() % c.size());    //-V104
    test::count_instances::instance_count.store(0);

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        pika::future<void> f = pika::uninitialized_fill(p,
            decorated_iterator(std::begin(c),
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::bad_alloc();
                }),
            decorated_iterator(std::end(c)), 10);

        returned_from_algorithm = true;
        f.get();

        PIKA_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        PIKA_TEST(false);
    }

    PIKA_TEST(caught_bad_alloc);
    PIKA_TEST(returned_from_algorithm);
    PIKA_TEST_EQ(test::count_instances::instance_count.load(), std::size_t(0));
}

template <typename IteratorTag>
void test_uninitialized_fill_bad_alloc()
{
    using namespace pika::execution;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_uninitialized_fill_bad_alloc(seq, IteratorTag());
    test_uninitialized_fill_bad_alloc(par, IteratorTag());

    test_uninitialized_fill_bad_alloc_async(seq(task), IteratorTag());
    test_uninitialized_fill_bad_alloc_async(par(task), IteratorTag());
}

void uninitialized_fill_bad_alloc_test()
{
    test_uninitialized_fill_bad_alloc<std::random_access_iterator_tag>();
    test_uninitialized_fill_bad_alloc<std::forward_iterator_tag>();
}

int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    uninitialized_fill_test();
    uninitialized_fill_exception_test();
    uninitialized_fill_bad_alloc_test();
    return pika::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace pika::program_options;
    options_description desc_commandline(
        "Usage: " PIKA_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run");

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"pika.os_threads=all"};

    // Initialize and run pika
    pika::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    init_args.cfg = cfg;

    PIKA_TEST_EQ_MSG(pika::init(pika_main, argc, argv, init_args), 0,
        "pika main exited with non-zero status");

    return pika::util::report_errors();
}
