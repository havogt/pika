# Copyright (c) 2014-2020 Hartmut Kaiser
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    count_3646
    for_each_annotated_function
    for_loop_2281
    minimal_findend
    reduce_3641
    scan_different_inits
    scan_non_commutative
    scan_shortlength
    search_zerolength
    set_operations_3442
    stable_merge_2964
    static_chunker_2282
    transform_inclusive_scan_4786
    transform_inclusive_scan_4787
    ranges_facilities
)

if(PIKA_WITH_DATAPAR_VC OR PIKA_WITH_CXX20_EXPERIMENTAL_SIMD)
  list(APPEND tests for_each_datapar)
endif()

foreach(test ${tests})
  set(sources ${test}.cpp)

  set(${test}_PARAMETERS THREADS 4)

  source_group("Source Files" FILES ${sources})

  # add example executable
  pika_add_executable(
    ${test}_test INTERNAL_FLAGS
    SOURCES ${sources}
    EXCLUDE_FROM_ALL ${${test}_FLAGS}
    FOLDER "Tests/Regressions/Modules/Algorithms/"
  )

  target_link_libraries(
    ${test}_test PRIVATE pika_iterator_support_test_utilities
  )

  pika_add_regression_test("modules.algorithms" ${test} ${${test}_PARAMETERS})
endforeach()
