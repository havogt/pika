//  Copyright (c) 2020 ETH Zurich
//  Copyright (c) 2013 Hartmut Kaiser
//  Copyright (c) 2017 Denis Blank
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define PIKA_NO_VERSION_CHECK

#include <pika/assert.hpp>
#include <pika/modules/format.hpp>
#include <pika/modules/util.hpp>
#include <pika/testing.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

namespace pika { namespace util {
    namespace detail {

        std::atomic<std::size_t> fixture::sanity_failures_(0);
        std::atomic<std::size_t> fixture::test_failures_(0);

        void fixture::increment(counter_type c)
        {
            switch (c)
            {
            case counter_sanity:
                ++sanity_failures_;
                return;
            case counter_test:
                ++test_failures_;
                return;
            default:
                break;
            }
            PIKA_ASSERT(false);
        }

        std::size_t fixture::get(counter_type c) const
        {
            switch (c)
            {
            case counter_sanity:
                return sanity_failures_;
            case counter_test:
                return test_failures_;
            default:
                break;
            }
            PIKA_ASSERT(false);
            return std::size_t(-1);
        }

        fixture global_fixture{std::cerr};

    }    // namespace detail

    ////////////////////////////////////////////////////////////////////////////
    int report_errors()
    {
        return report_errors(std::cerr);
    }

    int report_errors(std::ostream& stream)
    {
        std::size_t sanity = detail::global_fixture.get(counter_sanity),
                    test = detail::global_fixture.get(counter_test);
        if (sanity == 0 && test == 0)
            return 0;

        else
        {
            pika::util::ios_flags_saver ifs(stream);
            stream << sanity << " sanity check"    //-V128
                   << ((sanity == 1) ? " and " : "s and ") << test << " test"
                   << ((test == 1) ? " failed." : "s failed.") << std::endl;
            return 1;
        }
    }

    void print_cdash_timing(const char* name, double time)
    {
        // use format followed by single cout for better multi-threaded output
        std::string temp =
            pika::util::format("<DartMeasurement name=\"{}\" "
                               "type=\"numeric/double\">{}</DartMeasurement>",
                name, time);
        std::cout << temp << std::endl;
    }

    void print_cdash_timing(const char* name, std::uint64_t time)
    {
        print_cdash_timing(name, time / 1e9);
    }

}}    // namespace pika::util
