//  Copyright (c) 2007-2015 Hartmut Kaiser
//  Copyright (c)      2021 Giannis Gonidelis
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/iterator_support/iterator_range.hpp>
#include <pika/iterator_support/tests/iter_sent.hpp>
#include <pika/parallel/container_algorithms/reverse.hpp>
#include <pika/testing.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
void test_reverse_copy_sent()
{
    std::size_t const size = 100;
    std::vector<std::int16_t> c(size);
    std::vector<std::int16_t> d(size);
    std::iota(std::begin(c), std::end(c), 0);

    auto first = c.begin();
    auto last = c[49];
    PIKA_TEST(*first == 0);

    pika::ranges::reverse_copy(
        std::begin(c), sentinel<std::int16_t>{50}, std::begin(d));
    auto first_reversed = d.begin();

    PIKA_TEST(*first_reversed == last);
}

template <typename ExPolicy>
void test_reverse_copy_sent(ExPolicy policy)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    std::size_t const size = 100;
    std::vector<std::int16_t> c(size);
    std::vector<std::int16_t> d(size);
    std::iota(std::begin(c), std::end(c), 0);

    auto first = c.begin();
    auto last = c[49];
    PIKA_TEST(*first == 0);

    pika::ranges::reverse_copy(
        policy, std::begin(c), sentinel<std::int16_t>{50}, std::begin(d));
    auto first_reversed = d.begin();

    PIKA_TEST(*first_reversed == last);
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_reverse_copy(IteratorTag)
{
    typedef test::test_container<std::vector<std::size_t>, IteratorTag>
        test_vector;

    test_vector c(10007);
    std::vector<std::size_t> d1(c.size());
    std::vector<std::size_t> d2(c.size());    //-V656

    std::iota(std::begin(c.base()), std::end(c.base()), std::rand());

    pika::ranges::reverse_copy(c, std::begin(d1));

    std::reverse_copy(std::begin(c.base()), std::end(c.base()), std::begin(d2));

    std::size_t count = 0;
    PIKA_TEST(std::equal(std::begin(d1), std::end(d1), std::begin(d2),
        [&count](std::size_t v1, std::size_t v2) -> bool {
            PIKA_TEST_EQ(v1, v2);
            ++count;
            return v1 == v2;
        }));
    PIKA_TEST_EQ(count, d1.size());
}

template <typename ExPolicy, typename IteratorTag>
void test_reverse_copy(ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef test::test_container<std::vector<std::size_t>, IteratorTag>
        test_vector;

    test_vector c(10007);
    std::vector<std::size_t> d1(c.size());
    std::vector<std::size_t> d2(c.size());    //-V656

    std::iota(std::begin(c.base()), std::end(c.base()), std::rand());

    pika::ranges::reverse_copy(policy, c, std::begin(d1));

    std::reverse_copy(std::begin(c.base()), std::end(c.base()), std::begin(d2));

    std::size_t count = 0;
    PIKA_TEST(std::equal(std::begin(d1), std::end(d1), std::begin(d2),
        [&count](std::size_t v1, std::size_t v2) -> bool {
            PIKA_TEST_EQ(v1, v2);
            ++count;
            return v1 == v2;
        }));
    PIKA_TEST_EQ(count, d1.size());
}

template <typename ExPolicy, typename IteratorTag>
void test_reverse_copy_async(ExPolicy p, IteratorTag)
{
    typedef test::test_container<std::vector<std::size_t>, IteratorTag>
        test_vector;

    test_vector c(10007);
    std::vector<std::size_t> d1(c.size());
    std::vector<std::size_t> d2(c.size());    //-V656

    std::iota(std::begin(c.base()), std::end(c.base()), std::rand());

    auto f = pika::ranges::reverse_copy(p, c, std::begin(d1));
    f.wait();

    std::reverse_copy(std::begin(c.base()), std::end(c.base()), std::begin(d2));

    std::size_t count = 0;
    PIKA_TEST(std::equal(std::begin(d1), std::end(d1), std::begin(d2),
        [&count](std::size_t v1, std::size_t v2) -> bool {
            PIKA_TEST_EQ(v1, v2);
            ++count;
            return v1 == v2;
        }));
    PIKA_TEST_EQ(count, d1.size());
}

template <typename IteratorTag>
void test_reverse_copy()
{
    using namespace pika::execution;
    test_reverse_copy(IteratorTag());
    test_reverse_copy(seq, IteratorTag());
    test_reverse_copy(par, IteratorTag());
    test_reverse_copy(par_unseq, IteratorTag());

    test_reverse_copy_sent();
    test_reverse_copy_sent(seq);
    test_reverse_copy_sent(par);
    test_reverse_copy_sent(par_unseq);

    test_reverse_copy_async(seq(task), IteratorTag());
    test_reverse_copy_async(par(task), IteratorTag());
}

void reverse_copy_test()
{
    test_reverse_copy<std::random_access_iterator_tag>();
    test_reverse_copy<std::bidirectional_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_reverse_copy_exception(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::vector<std::size_t> d(c.size());
    std::iota(std::begin(c), std::end(c), std::rand());

    bool caught_exception = false;
    try
    {
        pika::ranges::reverse_copy(
            pika::util::make_iterator_range(decorated_iterator(std::begin(c)),
                decorated_iterator(
                    std::end(c), []() { throw std::runtime_error("test"); })),
            std::begin(d));
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
void test_reverse_copy_exception(ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::vector<std::size_t> d(c.size());
    std::iota(std::begin(c), std::end(c), std::rand());

    bool caught_exception = false;
    try
    {
        pika::ranges::reverse_copy(policy,
            pika::util::make_iterator_range(decorated_iterator(std::begin(c)),
                decorated_iterator(
                    std::end(c), []() { throw std::runtime_error("test"); })),
            std::begin(d));
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
void test_reverse_copy_exception_async(ExPolicy p, IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::vector<std::size_t> d(c.size());
    std::iota(std::begin(c), std::end(c), std::rand());

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        auto f = pika::ranges::reverse_copy(p,
            pika::util::make_iterator_range(decorated_iterator(std::begin(c)),
                decorated_iterator(
                    std::end(c), []() { throw std::runtime_error("test"); })),
            std::begin(d));
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
void test_reverse_copy_exception()
{
    using namespace pika::execution;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_reverse_copy_exception(IteratorTag());
    test_reverse_copy_exception(seq, IteratorTag());
    test_reverse_copy_exception(par, IteratorTag());

    test_reverse_copy_exception_async(seq(task), IteratorTag());
    test_reverse_copy_exception_async(par(task), IteratorTag());
}

void reverse_copy_exception_test()
{
    test_reverse_copy_exception<std::random_access_iterator_tag>();
    test_reverse_copy_exception<std::bidirectional_iterator_tag>();
}

//////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_reverse_copy_bad_alloc(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::vector<std::size_t> d(c.size());
    std::iota(std::begin(c), std::end(c), std::rand());

    bool caught_bad_alloc = false;
    try
    {
        pika::ranges::reverse_copy(
            pika::util::make_iterator_range(decorated_iterator(std::begin(c)),
                decorated_iterator(
                    std::end(c), []() { throw std::bad_alloc(); })),
            std::begin(d));
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
void test_reverse_copy_bad_alloc(ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::vector<std::size_t> d(c.size());
    std::iota(std::begin(c), std::end(c), std::rand());

    bool caught_bad_alloc = false;
    try
    {
        pika::ranges::reverse_copy(policy,
            pika::util::make_iterator_range(decorated_iterator(std::begin(c)),
                decorated_iterator(
                    std::end(c), []() { throw std::bad_alloc(); })),
            std::begin(d));
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
void test_reverse_copy_bad_alloc_async(ExPolicy p, IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::vector<std::size_t> d(c.size());
    std::iota(std::begin(c), std::end(c), std::rand());

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        auto f = pika::ranges::reverse_copy(p,
            pika::util::make_iterator_range(decorated_iterator(std::begin(c)),
                decorated_iterator(
                    std::end(c), []() { throw std::bad_alloc(); })),
            std::begin(d));
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
void test_reverse_copy_bad_alloc()
{
    using namespace pika::execution;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_reverse_copy_bad_alloc(IteratorTag());
    test_reverse_copy_bad_alloc(seq, IteratorTag());
    test_reverse_copy_bad_alloc(par, IteratorTag());

    test_reverse_copy_bad_alloc_async(seq(task), IteratorTag());
    test_reverse_copy_bad_alloc_async(par(task), IteratorTag());
}

void reverse_copy_bad_alloc_test()
{
    test_reverse_copy_bad_alloc<std::random_access_iterator_tag>();
    test_reverse_copy_bad_alloc<std::bidirectional_iterator_tag>();
}

int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    reverse_copy_test();
    reverse_copy_exception_test();
    reverse_copy_bad_alloc_test();
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
