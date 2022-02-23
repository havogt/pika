//  Copyright (c) 2014-2020 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/parallel/algorithms/equal.hpp>
#include <pika/testing.hpp>

#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
unsigned int seed = std::random_device{}();
std::mt19937 gen(seed);

template <typename IteratorTag>
void test_equal1(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    {
        bool result = pika::equal(
            iterator(std::begin(c1)), iterator(std::end(c1)), std::begin(c2));

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }

    {
        std::uniform_int_distribution<> dis(0, c1.size() - 1);
        ++c1[dis(gen)];    //-V104
        bool result = pika::equal(
            iterator(std::begin(c1)), iterator(std::end(c1)), std::begin(c2));

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_equal1(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    {
        bool result = pika::equal(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), std::begin(c2));

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }

    {
        std::uniform_int_distribution<> dis(0, c1.size() - 1);
        ++c1[dis(gen)];    //-V104
        bool result = pika::equal(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), std::begin(c2));

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_equal1_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    {
        pika::future<bool> result = pika::equal(p, iterator(std::begin(c1)),
            iterator(std::end(c1)), std::begin(c2));
        result.wait();

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result.get(), expected);
    }

    {
        std::uniform_int_distribution<> dis(0, c1.size() - 1);
        ++c1[dis(gen)];    //-V104

        pika::future<bool> result = pika::equal(p, iterator(std::begin(c1)),
            iterator(std::end(c1)), std::begin(c2));
        result.wait();

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result.get(), expected);
    }
}

template <typename IteratorTag>
void test_equal1()
{
    using namespace pika::execution;

    test_equal1(IteratorTag());

    test_equal1(seq, IteratorTag());
    test_equal1(par, IteratorTag());
    test_equal1(par_unseq, IteratorTag());

    test_equal1_async(seq(task), IteratorTag());
    test_equal1_async(par(task), IteratorTag());
}

void equal_test1()
{
    test_equal1<std::random_access_iterator_tag>();
    test_equal1<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_equal2(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    {
        bool result =
            pika::equal(iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), std::equal_to<std::size_t>());

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }

    {
        std::uniform_int_distribution<> dis(0, c1.size() - 1);
        ++c1[dis(gen)];    //-V104
        bool result =
            pika::equal(iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), std::equal_to<std::size_t>());

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_equal2(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    {
        bool result = pika::equal(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), std::begin(c2),
            std::equal_to<std::size_t>());

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }

    {
        std::uniform_int_distribution<> dis(0, c1.size() - 1);
        ++c1[dis(gen)];    //-V104
        bool result = pika::equal(policy, iterator(std::begin(c1)),
            iterator(std::end(c1)), std::begin(c2),
            std::equal_to<std::size_t>());

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_equal2_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    {
        pika::future<bool> result =
            pika::equal(p, iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), std::equal_to<std::size_t>());
        result.wait();

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result.get(), expected);
    }

    {
        std::uniform_int_distribution<> dis(0, c1.size() - 1);
        ++c1[dis(gen)];    //-V104

        pika::future<bool> result =
            pika::equal(p, iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), std::equal_to<std::size_t>());
        result.wait();

        bool expected =
            std::equal(std::begin(c1), std::end(c1), std::begin(c2));

        // verify values
        PIKA_TEST_EQ(result.get(), expected);
    }
}

template <typename IteratorTag>
void test_equal2()
{
    using namespace pika::execution;

    test_equal2(IteratorTag());

    test_equal2(seq, IteratorTag());
    test_equal2(par, IteratorTag());
    test_equal2(par_unseq, IteratorTag());

    test_equal2_async(seq(task), IteratorTag());
    test_equal2_async(par(task), IteratorTag());
}

void equal_test2()
{
    test_equal2<std::random_access_iterator_tag>();
    test_equal2<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_equal_exception(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    bool caught_exception = false;
    try
    {
        pika::equal(iterator(std::begin(c1)), iterator(std::end(c1)),
            std::begin(c2), [](std::size_t, std::size_t) {
                return throw std::runtime_error("test"), true;
            });

        PIKA_TEST(false);
    }
    catch (pika::exception_list const& e)
    {
        caught_exception = true;
        test::test_num_exceptions<pika::execution::sequenced_policy,
            IteratorTag>::call(pika::execution::seq, e);
    }
    catch (...)
    {
        PIKA_TEST(false);
    }

    PIKA_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_equal_exception(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    bool caught_exception = false;
    try
    {
        pika::equal(policy, iterator(std::begin(c1)), iterator(std::end(c1)),
            std::begin(c2), [](std::size_t, std::size_t) {
                return throw std::runtime_error("test"), true;
            });

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
}

template <typename ExPolicy, typename IteratorTag>
void test_equal_exception_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        pika::future<bool> f =
            pika::equal(p, iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), [](std::size_t, std::size_t) {
                    return throw std::runtime_error("test"), true;
                });
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
}

template <typename IteratorTag>
void test_equal_exception()
{
    using namespace pika::execution;

    test_equal_exception(IteratorTag());

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_equal_exception(seq, IteratorTag());
    test_equal_exception(par, IteratorTag());

    test_equal_exception_async(seq(task), IteratorTag());
    test_equal_exception_async(par(task), IteratorTag());
}

void equal_exception_test()
{
    test_equal_exception<std::random_access_iterator_tag>();
    test_equal_exception<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_equal_bad_alloc(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    bool caught_bad_alloc = false;
    try
    {
        pika::equal(policy, iterator(std::begin(c1)), iterator(std::end(c1)),
            std::begin(c2), [](std::size_t, std::size_t) {
                return throw std::bad_alloc(), true;
            });

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
}

template <typename ExPolicy, typename IteratorTag>
void test_equal_bad_alloc_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c1(10007);
    std::vector<std::size_t> c2(c1.size());

    std::size_t first_value = gen();    //-V101
    std::iota(std::begin(c1), std::end(c1), first_value);
    std::iota(std::begin(c2), std::end(c2), first_value);

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        pika::future<bool> f =
            pika::equal(p, iterator(std::begin(c1)), iterator(std::end(c1)),
                std::begin(c2), [](std::size_t, std::size_t) {
                    return throw std::bad_alloc(), true;
                });
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
}

template <typename IteratorTag>
void test_equal_bad_alloc()
{
    using namespace pika::execution;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_equal_bad_alloc(seq, IteratorTag());
    test_equal_bad_alloc(par, IteratorTag());

    test_equal_bad_alloc_async(seq(task), IteratorTag());
    test_equal_bad_alloc_async(par(task), IteratorTag());
}

void equal_bad_alloc_test()
{
    test_equal_bad_alloc<std::random_access_iterator_tag>();
    test_equal_bad_alloc<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
int pika_main(pika::program_options::variables_map& vm)
{
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    gen.seed(seed);

    equal_test1();
    equal_test2();
    equal_exception_test();
    equal_bad_alloc_test();
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
