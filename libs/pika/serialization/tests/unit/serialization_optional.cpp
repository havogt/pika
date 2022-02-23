//  Copyright (c) 2017 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/datastructures/optional.hpp>
#include <pika/serialization/input_archive.hpp>
#include <pika/serialization/optional.hpp>
#include <pika/serialization/output_archive.hpp>
#include <pika/serialization/serialize.hpp>

#include <pika/testing.hpp>

#include <vector>

template <typename T>
struct A
{
    A() {}

    explicit A(T t)
      : t_(t)
    {
    }
    T t_;

    A& operator=(T t)
    {
        t_ = t;
        return *this;
    }

    friend bool operator==(A a, A b)
    {
        return a.t_ == b.t_;
    }

    friend std::ostream& operator<<(std::ostream& os, A a)
    {
        os << a.t_;
        return os;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned)
    {
        ar& t_;
    }
};

int main()
{
    std::vector<char> buf;
    pika::serialization::output_archive oar(buf);
    pika::serialization::input_archive iar(buf);

    {
        pika::util::optional<int> ovar;
        pika::util::optional<int> ivar;
        oar << ovar;
        iar >> ivar;

        PIKA_TEST_EQ(ivar.has_value(), ovar.has_value());
        PIKA_TEST(ivar == ovar);
        PIKA_TEST(ivar == pika::util::nullopt);
    }

    {
        pika::util::optional<int> ovar = 42;
        pika::util::optional<int> ivar;
        oar << ovar;
        iar >> ivar;

        PIKA_TEST_EQ(ivar.has_value(), ovar.has_value());
        PIKA_TEST(ivar == ovar);
        PIKA_TEST(ivar != pika::util::nullopt);
    }

    {
        pika::util::optional<double> ovar = 2.5;
        pika::util::optional<double> ivar;
        oar << ovar;
        iar >> ivar;

        PIKA_TEST_EQ(ivar.has_value(), ovar.has_value());
        PIKA_TEST(ivar == ovar);
        PIKA_TEST(ivar != pika::util::nullopt);
    }

    {
        pika::util::optional<A<int>> ovar = A<int>{2};
        pika::util::optional<A<int>> ivar;
        oar << ovar;
        iar >> ivar;

        PIKA_TEST_EQ(ivar.has_value(), ovar.has_value());
        PIKA_TEST(ivar == ovar);
        PIKA_TEST(ivar != pika::util::nullopt);
    }

    return pika::util::report_errors();
}
