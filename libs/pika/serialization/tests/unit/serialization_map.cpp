//  Copyright (c) 2014 Thomas Heller
//  Copyright (c) 2015 Anton Bikineev
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/serialization/input_archive.hpp>
#include <pika/serialization/map.hpp>
#include <pika/serialization/output_archive.hpp>
#include <pika/serialization/serialize.hpp>
#include <pika/serialization/vector.hpp>

#include <pika/testing.hpp>

#include <algorithm>
#include <iterator>
#include <map>
#include <numeric>
#include <utility>
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
    bool operator==(const A& a) const
    {
        return t_ == a.t_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned)
    {
        ar& t_;
    }
};

template <class T>
std::ostream& operator<<(std::ostream& os, const A<T>& a)
{
    return os << a.t_;
}

template <typename T>
void test(T min, T max)
{
    {
        std::vector<char> buffer;
        pika::serialization::output_archive oarchive(buffer);
        std::map<T, A<T>> os;
        for (T c = min; c < max; ++c)
        {
            os.insert(std::make_pair(c, A<T>(c)));
        }
        oarchive << os;
        pika::serialization::input_archive iarchive(buffer);
        std::map<T, A<T>> is;
        iarchive >> is;
        PIKA_TEST_EQ(os.size(), is.size());
        for (const auto& v : os)
        {
            PIKA_TEST_EQ(os[v.first], is[v.first]);
        }
    }
}

template <typename T>
void test_fp(T min, T max)
{
    {
        std::vector<char> buffer;
        pika::serialization::output_archive oarchive(buffer);
        std::map<T, A<T>> os;
        for (T c = min; c < max; c += static_cast<T>(0.5))
        {
            os.insert(std::make_pair(c, A<T>(c)));
        }
        oarchive << os;
        pika::serialization::input_archive iarchive(buffer);
        std::map<T, A<T>> is;
        iarchive >> is;
        PIKA_TEST_EQ(os.size(), is.size());
        for (const auto& v : os)
        {
            PIKA_TEST_EQ(os[v.first], is[v.first]);
        }
    }
}

// prohibited, but for adl
namespace std {
    std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec)
    {
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(os, " "));
        return os;
    }
}    // namespace std

void test_vector_as_value()
{
    std::vector<char> buffer;
    pika::serialization::output_archive oarchive(buffer);
    std::map<size_t, std::vector<int>> os;
    for (int k = 0; k < 10; ++k)
    {
        std::vector<int> vec(10);
        std::iota(vec.begin(), vec.end(), k);
        os.insert(std::make_pair(k, vec));
    }
    oarchive << os;
    pika::serialization::input_archive iarchive(buffer);
    std::map<size_t, std::vector<int>> is;
    iarchive >> is;
    PIKA_TEST_EQ(os.size(), is.size());
    for (const auto& v : os)
    {
        PIKA_TEST_EQ(os[v.first], is[v.first]);
    }
}

int main()
{
    test<char>(
        (std::numeric_limits<char>::min)(), (std::numeric_limits<char>::max)());
    test<int>((std::numeric_limits<int>::min)(),
        (std::numeric_limits<int>::min)() + 100);
    test<int>((std::numeric_limits<int>::max)() - 100,
        (std::numeric_limits<int>::max)());
    test<int>(-100, 100);
    test<unsigned>((std::numeric_limits<unsigned>::min)(),
        (std::numeric_limits<unsigned>::min)() + 100);
    test<unsigned>((std::numeric_limits<unsigned>::max)() - 100,
        (std::numeric_limits<unsigned>::max)());
    test<long>((std::numeric_limits<long>::min)(),
        (std::numeric_limits<long>::min)() + 100);
    test<long>((std::numeric_limits<long>::max)() - 100,
        (std::numeric_limits<long>::max)());
    test<long>(-100, 100);
    test<unsigned long>((std::numeric_limits<unsigned long>::min)(),
        (std::numeric_limits<unsigned long>::min)() + 100);
    test<unsigned long>((std::numeric_limits<unsigned long>::max)() - 100,
        (std::numeric_limits<unsigned long>::max)());
    test_fp<float>((std::numeric_limits<float>::min)(),
        (std::numeric_limits<float>::min)() + 100);
    test_fp<float>(-100, 100);
    test<double>((std::numeric_limits<double>::min)(),
        (std::numeric_limits<double>::min)() + 100);
    test<double>(-100, 100);

    test_vector_as_value();

    return pika::util::report_errors();
}
