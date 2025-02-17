//  Copyright (c) 2022 ETH Zurich
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This test checks that the runtime takes into account suspended threads before
// initiating full shutdown.

#include <pika/init.hpp>
#include <pika/testing.hpp>

int pika_main()
{
    return pika::finalize();
}

int main(const int argc, char** const argv)
{
    PIKA_TEST_EQ(pika::init(pika_main, argc, argv), 0);
    return pika::util::report_errors();
}
