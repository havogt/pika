# Copyright (c) 2007-2017 Hartmut Kaiser
# Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    future
    future_ref
    future_then
    promise_allocator
    use_allocator
    make_future
    make_ready_future
    shared_future
)

if(PIKA_WITH_CXX20_COROUTINES)
  set(tests ${tests} await)
  set(await_PARAMETERS THREADS 4)
endif()

set(future_PARAMETERS THREADS 4)
set(future_then_PARAMETERS THREADS 4)

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  set(folder_name "Tests/Unit/Modules/Futures")

  pika_add_executable(
    ${test}_test INTERNAL_FLAGS
    SOURCES ${sources} ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER ${folder_name}
  )

  pika_add_unit_test("modules.futures" ${test} ${${test}_PARAMETERS})
endforeach()
