//  Copyright (c) 2017-2018 Taeguk Kwon
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/iterator_support/tests/iter_sent.hpp>
#include <pika/parallel/container_algorithms/remove.hpp>
#include <pika/testing.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "test_utils.hpp"

unsigned int seed = std::random_device{}();
std::mt19937 g(seed);

////////////////////////////////////////////////////////////////////////////
struct user_defined_type
{
    user_defined_type() = default;
    user_defined_type(int rand_no)
      : val(rand_no)
      , name(name_list[std::rand() % name_list.size()])
    {
    }

    bool operator==(int rand_no) const
    {
        return this->val == rand_no;
    }

    bool operator==(user_defined_type const& t) const
    {
        return this->name == t.name && this->val == t.val;
    }

    static const std::vector<std::string> name_list;

    int val;
    std::string name;
};

const std::vector<std::string> user_defined_type::name_list{
    "ABB", "ABC", "ACB", "BASE", "CAA", "CAAA", "CAAB"};

struct random_fill
{
    random_fill(int rand_base, int range)
      : gen(std::rand())
      , dist(rand_base - range / 2, rand_base + range / 2)
    {
    }

    int operator()()
    {
        return dist(gen);
    }

    std::mt19937 gen;
    std::uniform_int_distribution<> dist;
};

////////////////////////////////////////////////////////////////////////////
// test case for iterator - sentinel_value
void test_remove_sent()
{
    using pika::get;

    std::size_t const size = 100;
    std::vector<std::int16_t> c(size);
    std::iota(std::begin(c), std::end(c), 0);
    c[99] = 42;    //both c[99] and c[42] equal 42

    int value = 42;

    auto pre_result = std::count(std::begin(c), std::end(c), value);
    pika::ranges::remove(std::begin(c), sentinel<std::int16_t>{50}, value);
    auto post_result = std::count(std::begin(c), std::end(c), value);

    PIKA_TEST(pre_result == 2 && post_result == 1);
}

template <typename ExPolicy>
void test_remove_sent(ExPolicy policy)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using pika::get;

    std::size_t const size = 100;
    std::vector<std::int16_t> c(size);
    std::iota(std::begin(c), std::end(c), 0);
    c[99] = 42;    //both c[99] and c[42] equal 42

    int value = 42;

    auto pre_result = std::count(std::begin(c), std::end(c), value);
    pika::ranges::remove(
        policy, std::begin(c), sentinel<std::int16_t>{50}, value);
    auto post_result = std::count(std::begin(c), std::end(c), value);

    PIKA_TEST(pre_result == 2 && post_result == 1);
}

template <typename DataType>
void test_remove(DataType)
{
    using pika::get;

    std::size_t const size = 10007;
    std::vector<DataType> c(size), d;
    std::generate(std::begin(c), std::end(c), random_fill(0, 6));
    d = c;

    auto value = DataType(0);

    auto result = pika::ranges::remove(c, value);
    auto solution = std::remove(std::begin(d), std::end(d), value);

    bool equality =
        test::equal(std::begin(c), std::begin(result), std::begin(d), solution);

    PIKA_TEST(equality);
}

template <typename ExPolicy, typename DataType>
void test_remove(ExPolicy policy, DataType)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using pika::get;

    std::size_t const size = 10007;
    std::vector<DataType> c(size), d;
    std::generate(std::begin(c), std::end(c), random_fill(0, 6));
    d = c;

    auto value = DataType(0);

    auto result = pika::ranges::remove(policy, c, value);
    auto solution = std::remove(std::begin(d), std::end(d), value);

    bool equality =
        test::equal(std::begin(c), std::begin(result), std::begin(d), solution);

    PIKA_TEST(equality);
}

template <typename ExPolicy, typename DataType>
void test_remove_async(ExPolicy policy, DataType)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using pika::get;

    std::size_t const size = 10007;
    std::vector<DataType> c(size), d;
    std::generate(std::begin(c), std::end(c), random_fill(0, 6));
    d = c;

    auto value = DataType(0);

    auto f = pika::ranges::remove(policy, c, value);
    auto result = f.get();
    auto solution = std::remove(std::begin(d), std::end(d), value);

    bool equality =
        test::equal(std::begin(c), std::begin(result), std::begin(d), solution);

    PIKA_TEST(equality);
}

template <typename ExPolicy, typename DataType>
void test_remove_proj(
    ExPolicy policy, DataType, int rand_base, bool test_for_remove_if = false)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    std::size_t const size = 10007;
    std::vector<DataType> c(size);
    std::generate(
        std::begin(c), std::end(c), random_fill(rand_base, size / 10));

    // Test projection.
    {
        DataType value(rand_base);

        if (test_for_remove_if)
        {
            auto result = pika::ranges::remove_if(
                policy, std::begin(c), std::end(c),
                [&value](DataType const& a) -> bool { return a == value; },
                [&value](DataType const&) -> DataType& {
                    // This is the projection.
                    return value;
                });
            auto dist = std::distance(std::begin(c), std::begin(result));
            PIKA_TEST_EQ(dist, 0);
        }
        else
        {
            auto result = pika::ranges::remove(policy, std::begin(c),
                std::end(c), value, [&value](DataType const&) -> DataType& {
                    // This is the projection.
                    return value;
                });
            auto dist = std::distance(std::begin(c), std::begin(result));
            PIKA_TEST_EQ(dist, 0);
        }
    }
}

template <typename DataType>
void test_remove()
{
    using namespace pika::execution;

    test_remove(DataType());
    test_remove(seq, DataType());
    test_remove(par, DataType());
    test_remove(par_unseq, DataType());

    test_remove_async(seq(task), DataType());
    test_remove_async(par(task), DataType());

    test_remove_sent();
    test_remove_sent(par);
    test_remove_sent(par_unseq);

    // test projection
    int rand_base = g();
    test_remove_proj(seq, DataType(), rand_base);
    test_remove_proj(seq, DataType(), rand_base, true);
    test_remove_proj(par, DataType(), rand_base);
    test_remove_proj(par, DataType(), rand_base, true);
    test_remove_proj(par_unseq, DataType(), rand_base);
    test_remove_proj(par_unseq, DataType(), rand_base, true);
}

void test_remove()
{
    test_remove<int>();
    test_remove<user_defined_type>();
}

int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    test_remove();
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
