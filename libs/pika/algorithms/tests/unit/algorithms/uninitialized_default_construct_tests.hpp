//  Copyright (c) 2014-2017 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/execution.hpp>
#include <pika/parallel/algorithms/uninitialized_default_construct.hpp>
#include <pika/testing.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>

#include "test_utils.hpp"

struct default_constructable
{
    default_constructable()
      : value_(42)
    {
    }
    std::int32_t value_;
};

struct value_constructable
{
    std::int32_t value_;
};

std::size_t const data_size = 10007;

////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef default_constructable* base_iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    default_constructable* p = (default_constructable*) std::malloc(
        data_size * sizeof(default_constructable));
    std::memset(
        static_cast<void*>(p), 0xcd, data_size * sizeof(default_constructable));

    pika::uninitialized_default_construct(
        std::forward<ExPolicy>(policy), iterator(p), iterator(p + data_size));

    std::size_t count = 0;
    std::for_each(p, p + data_size, [&count](default_constructable v1) {
        PIKA_TEST_EQ(v1.value_, 42);
        ++count;
    });
    PIKA_TEST_EQ(count, data_size);

    std::free(p);
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct_async(ExPolicy&& policy, IteratorTag)
{
    typedef default_constructable* base_iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    default_constructable* p = (default_constructable*) std::malloc(
        data_size * sizeof(default_constructable));
    std::memset(
        static_cast<void*>(p), 0xcd, data_size * sizeof(default_constructable));

    auto f = pika::uninitialized_default_construct(
        std::forward<ExPolicy>(policy), iterator(p), iterator(p + data_size));
    f.wait();

    std::size_t count = 0;
    std::for_each(p, p + data_size, [&count](default_constructable v1) {
        PIKA_TEST_EQ(v1.value_, 42);
        ++count;
    });
    PIKA_TEST_EQ(count, data_size);

    std::free(p);
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct2(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef value_constructable* base_iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    value_constructable* p = (value_constructable*) std::malloc(
        data_size * sizeof(value_constructable));
    std::memset(
        static_cast<void*>(p), 0xcd, data_size * sizeof(value_constructable));

    pika::uninitialized_default_construct(
        std::forward<ExPolicy>(policy), iterator(p), iterator(p + data_size));

    std::size_t count = 0;
    std::for_each(p, p + data_size, [&count](value_constructable v1) {
        PIKA_TEST_EQ(v1.value_, (std::int32_t) 0xcdcdcdcd);
        ++count;
    });
    PIKA_TEST_EQ(count, data_size);

    std::free(p);
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct_async2(ExPolicy&& policy, IteratorTag)
{
    typedef value_constructable* base_iterator;
    using iterator = test::test_iterator<base_iterator, IteratorTag>;

    value_constructable* p = (value_constructable*) std::malloc(
        data_size * sizeof(value_constructable));
    std::memset(
        static_cast<void*>(p), 0xcd, data_size * sizeof(value_constructable));

    auto f = pika::uninitialized_default_construct(
        std::forward<ExPolicy>(policy), iterator(p), iterator(p + data_size));
    f.wait();

    std::size_t count = 0;
    std::for_each(p, p + data_size, [&count](value_constructable v1) {
        PIKA_TEST_EQ(v1.value_, (std::int32_t) 0xcdcdcdcd);
        ++count;
    });
    PIKA_TEST_EQ(count, data_size);

    std::free(p);
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct_exception(
    ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using data_type = test::count_instances_v<default_constructable>;
    typedef data_type* base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    data_type* p = (data_type*) std::malloc(data_size * sizeof(data_type));
    std::memset(static_cast<void*>(p), 0xcd, data_size * sizeof(data_type));

    std::atomic<std::size_t> throw_after(std::rand() % data_size);    //-V104
    std::size_t throw_after_ = throw_after.load();

    data_type::instance_count.store(0);
    data_type::max_instance_count.store(0);

    bool caught_exception = false;
    try
    {
        pika::uninitialized_default_construct(policy,
            decorated_iterator(p,
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::runtime_error("test");
                }),
            decorated_iterator(p + data_size));
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
    PIKA_TEST_EQ(data_type::instance_count.load(), std::size_t(0));
    PIKA_TEST_LTE(throw_after_, data_type::max_instance_count.load());

    std::free(p);
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct_exception_async(
    ExPolicy policy, IteratorTag)
{
    using data_type = test::count_instances_v<default_constructable>;
    typedef data_type* base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    data_type* p = (data_type*) std::malloc(data_size * sizeof(data_type));
    std::memset(static_cast<void*>(p), 0xcd, data_size * sizeof(data_type));

    std::atomic<std::size_t> throw_after(std::rand() % data_size);    //-V104
    std::size_t throw_after_ = throw_after.load();

    data_type::instance_count.store(0);
    data_type::max_instance_count.store(0);

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        auto f = pika::uninitialized_default_construct(policy,
            decorated_iterator(p,
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::runtime_error("test");
                }),
            decorated_iterator(p + data_size));

        returned_from_algorithm = true;
        f.get();

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
    PIKA_TEST(returned_from_algorithm);
    PIKA_TEST_EQ(data_type::instance_count.load(), std::size_t(0));
    PIKA_TEST_LTE(throw_after_, data_type::max_instance_count.load());

    std::free(p);
}

//////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct_bad_alloc(
    ExPolicy policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    using data_type = test::count_instances_v<default_constructable>;
    typedef data_type* base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    data_type* p = (data_type*) std::malloc(data_size * sizeof(data_type));
    std::memset(static_cast<void*>(p), 0xcd, data_size * sizeof(data_type));

    std::atomic<std::size_t> throw_after(std::rand() % data_size);    //-V104
    std::size_t throw_after_ = throw_after.load();

    data_type::instance_count.store(0);
    data_type::max_instance_count.store(0);

    bool caught_bad_alloc = false;
    try
    {
        pika::uninitialized_default_construct(policy,
            decorated_iterator(p,
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::bad_alloc();
                }),
            decorated_iterator(p + data_size));

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
    PIKA_TEST_EQ(data_type::instance_count.load(), std::size_t(0));
    PIKA_TEST_LTE(throw_after_, data_type::max_instance_count.load());

    std::free(p);
}

template <typename ExPolicy, typename IteratorTag>
void test_uninitialized_default_construct_bad_alloc_async(
    ExPolicy policy, IteratorTag)
{
    using data_type = test::count_instances_v<default_constructable>;
    typedef data_type* base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    data_type* p = (data_type*) std::malloc(data_size * sizeof(data_type));
    std::memset(static_cast<void*>(p), 0xcd, data_size * sizeof(data_type));

    std::atomic<std::size_t> throw_after(std::rand() % data_size);    //-V104
    std::size_t throw_after_ = throw_after.load();

    data_type::instance_count.store(0);
    data_type::max_instance_count.store(0);

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        auto f = pika::uninitialized_default_construct(policy,
            decorated_iterator(p,
                [&throw_after]() {
                    if (throw_after-- == 0)
                        throw std::bad_alloc();
                }),
            decorated_iterator(p + data_size));

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
    PIKA_TEST_EQ(data_type::instance_count.load(), std::size_t(0));
    PIKA_TEST_LTE(throw_after_, data_type::max_instance_count.load());

    std::free(p);
}
