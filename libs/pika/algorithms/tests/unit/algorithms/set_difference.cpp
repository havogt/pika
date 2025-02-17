//  Copyright (c) 2015-2020 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/parallel/algorithms/set_difference.hpp>
#include <pika/testing.hpp>

#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_set_difference1(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size()), c4(2 * c1.size());    //-V656

    pika::set_difference(iterator(std::begin(c1)), iterator(std::end(c1)),
        std::begin(c2), std::end(c2), std::begin(c3));

    std::set_difference(std::begin(c1), std::end(c1), std::begin(c2),
        std::end(c2), std::begin(c4));

    // verify values
    PIKA_TEST(std::equal(std::begin(c3), std::end(c3), std::begin(c4)));
}

template <typename ExPolicy, typename IteratorTag>
void test_set_difference1(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size()), c4(2 * c1.size());    //-V656

    pika::set_difference(policy, iterator(std::begin(c1)),
        iterator(std::end(c1)), std::begin(c2), std::end(c2), std::begin(c3));

    std::set_difference(std::begin(c1), std::end(c1), std::begin(c2),
        std::end(c2), std::begin(c4));

    // verify values
    PIKA_TEST(std::equal(std::begin(c3), std::end(c3), std::begin(c4)));
}

template <typename ExPolicy, typename IteratorTag>
void test_set_difference1_async(ExPolicy&& p, IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size()), c4(2 * c1.size());    //-V656

    pika::future<void> result = pika::set_difference(p,
        iterator(std::begin(c1)), iterator(std::end(c1)), std::begin(c2),
        std::end(c2), std::begin(c3));
    result.wait();

    std::set_difference(std::begin(c1), std::end(c1), std::begin(c2),
        std::end(c2), std::begin(c4));

    // verify values
    PIKA_TEST(std::equal(std::begin(c3), std::end(c3), std::begin(c4)));
}

template <typename IteratorTag>
void test_set_difference1()
{
    using namespace pika::execution;

    test_set_difference1(IteratorTag());

    test_set_difference1(seq, IteratorTag());
    test_set_difference1(par, IteratorTag());
    test_set_difference1(par_unseq, IteratorTag());

    test_set_difference1_async(seq(task), IteratorTag());
    test_set_difference1_async(par(task), IteratorTag());
}

void set_difference_test1()
{
    test_set_difference1<std::random_access_iterator_tag>();
    test_set_difference1<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_set_difference2(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    auto comp = [](std::size_t l, std::size_t r) { return l > r; };

    std::sort(std::begin(c1), std::end(c1), comp);
    std::sort(std::begin(c2), std::end(c2), comp);

    std::vector<std::size_t> c3(2 * c1.size()), c4(2 * c1.size());    //-V656

    pika::set_difference(iterator(std::begin(c1)), iterator(std::end(c1)),
        std::begin(c2), std::end(c2), std::begin(c3), comp);

    std::set_difference(std::begin(c1), std::end(c1), std::begin(c2),
        std::end(c2), std::begin(c4), comp);

    // verify values
    PIKA_TEST(std::equal(std::begin(c3), std::end(c3), std::begin(c4)));
}

template <typename ExPolicy, typename IteratorTag>
void test_set_difference2(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    auto comp = [](std::size_t l, std::size_t r) { return l > r; };

    std::sort(std::begin(c1), std::end(c1), comp);
    std::sort(std::begin(c2), std::end(c2), comp);

    std::vector<std::size_t> c3(2 * c1.size()), c4(2 * c1.size());    //-V656

    pika::set_difference(policy, iterator(std::begin(c1)),
        iterator(std::end(c1)), std::begin(c2), std::end(c2), std::begin(c3),
        comp);

    std::set_difference(std::begin(c1), std::end(c1), std::begin(c2),
        std::end(c2), std::begin(c4), comp);

    // verify values
    PIKA_TEST(std::equal(std::begin(c3), std::end(c3), std::begin(c4)));
}

template <typename ExPolicy, typename IteratorTag>
void test_set_difference2_async(ExPolicy&& p, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    auto comp = [](std::size_t l, std::size_t r) { return l > r; };

    std::sort(std::begin(c1), std::end(c1), comp);
    std::sort(std::begin(c2), std::end(c2), comp);

    std::vector<std::size_t> c3(2 * c1.size()), c4(2 * c1.size());    //-V656

    pika::future<void> result = pika::set_difference(p,
        iterator(std::begin(c1)), iterator(std::end(c1)), std::begin(c2),
        std::end(c2), std::begin(c3), comp);
    result.wait();

    std::set_difference(std::begin(c1), std::end(c1), std::begin(c2),
        std::end(c2), std::begin(c4), comp);

    // verify values
    PIKA_TEST(std::equal(std::begin(c3), std::end(c3), std::begin(c4)));
}

template <typename IteratorTag>
void test_set_difference2()
{
    using namespace pika::execution;

    test_set_difference2(IteratorTag());

    test_set_difference2(seq, IteratorTag());
    test_set_difference2(par, IteratorTag());
    test_set_difference2(par_unseq, IteratorTag());

    test_set_difference2_async(seq(task), IteratorTag());
    test_set_difference2_async(par(task), IteratorTag());
}

void set_difference_test2()
{
    test_set_difference2<std::random_access_iterator_tag>();
    test_set_difference2<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_set_difference_exception(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size());

    bool caught_exception = false;
    try
    {
        pika::set_difference(decorated_iterator(std::begin(c1),
                                 []() { throw std::runtime_error("test"); }),
            decorated_iterator(std::end(c1)), std::begin(c2), std::end(c2),
            std::begin(c3));

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
void test_set_difference_exception(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size());

    bool caught_exception = false;
    try
    {
        pika::set_difference(policy,
            decorated_iterator(
                std::begin(c1), []() { throw std::runtime_error("test"); }),
            decorated_iterator(std::end(c1)), std::begin(c2), std::end(c2),
            std::begin(c3));

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
void test_set_difference_exception_async(ExPolicy&& p, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size());

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        pika::future<void> f = pika::set_difference(p,
            decorated_iterator(
                std::begin(c1), []() { throw std::runtime_error("test"); }),
            decorated_iterator(std::end(c1)), std::begin(c2), std::end(c2),
            std::begin(c3));

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
void test_set_difference_exception()
{
    using namespace pika::execution;

    test_set_difference_exception(IteratorTag());

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_set_difference_exception(seq, IteratorTag());
    test_set_difference_exception(par, IteratorTag());

    test_set_difference_exception_async(seq(task), IteratorTag());
    test_set_difference_exception_async(par(task), IteratorTag());
}

void set_difference_exception_test()
{
    test_set_difference_exception<std::random_access_iterator_tag>();
    test_set_difference_exception<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_set_difference_bad_alloc(IteratorTag)
{
    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size());

    bool caught_bad_alloc = false;
    try
    {
        pika::set_difference(decorated_iterator(std::begin(c1),
                                 []() { throw std::bad_alloc(); }),
            decorated_iterator(std::end(c1)), std::begin(c2), std::end(c2),
            std::begin(c3));

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
void test_set_difference_bad_alloc(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size());

    bool caught_bad_alloc = false;
    try
    {
        pika::set_difference(policy,
            decorated_iterator(
                std::begin(c1), []() { throw std::bad_alloc(); }),
            decorated_iterator(std::end(c1)), std::begin(c2), std::end(c2),
            std::begin(c3));

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
void test_set_difference_bad_alloc_async(ExPolicy&& p, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using base_iterator = std::vector<std::size_t>::iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c1 = test::random_fill(10007);
    std::vector<std::size_t> c2 = test::random_fill(c1.size());

    std::sort(std::begin(c1), std::end(c1));
    std::sort(std::begin(c2), std::end(c2));

    std::vector<std::size_t> c3(2 * c1.size());

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        pika::future<void> f = pika::set_difference(p,
            decorated_iterator(
                std::begin(c1), []() { throw std::bad_alloc(); }),
            decorated_iterator(std::end(c1)), std::begin(c2), std::end(c2),
            std::begin(c3));

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
void test_set_difference_bad_alloc()
{
    using namespace pika::execution;

    test_set_difference_bad_alloc(IteratorTag());

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_set_difference_bad_alloc(seq, IteratorTag());
    test_set_difference_bad_alloc(par, IteratorTag());

    test_set_difference_bad_alloc_async(seq(task), IteratorTag());
    test_set_difference_bad_alloc_async(par(task), IteratorTag());
}

void set_difference_bad_alloc_test()
{
    test_set_difference_bad_alloc<std::random_access_iterator_tag>();
    test_set_difference_bad_alloc<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    set_difference_test1();
    set_difference_test2();
    set_difference_exception_test();
    set_difference_bad_alloc_test();
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
