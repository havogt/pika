# Copyright (c) 2019 Hartmut Kaiser
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests intrusive_ptr intrusive_ptr_move ip_convertible ip_hash)

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add example executable
  pika_add_executable(
    ${test}_test INTERNAL_FLAGS
    SOURCES ${sources} ${${test}_FLAGS}
    EXCLUDE_FROM_ALL NOLIBS
    DEPENDENCIES pika
    FOLDER "Tests/Unit/Modules/Memory"
  )

  pika_add_unit_test("modules.memory" ${test} ${${test}_PARAMETERS})
  target_compile_definitions(${test}_test PRIVATE -DPIKA_MODULE_STATIC_LINKING)

endforeach()
