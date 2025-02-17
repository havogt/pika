# Copyright (c) 2021 ETH Zurich
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

export CRAYPE_LINK_TYPE=dynamic
export APPS_ROOT="/apps/daint/SSL/pika/packages"
export GCC_VER="11.1.0"
export CXX_STD="20"
export BOOST_VER="1.78.0"
export HWLOC_VER="2.7.0"
export GCC_ROOT="${APPS_ROOT}/gcc-${GCC_VER}"
export BOOST_ROOT="${APPS_ROOT}/boost-${BOOST_VER}-gcc-${GCC_VER}-c++${CXX_STD}-debug"
export HWLOC_ROOT="${APPS_ROOT}/hwloc-${HWLOC_VER}-gcc-${GCC_VER}"
export CXXFLAGS="-nostdinc++ -I${GCC_ROOT}/include/c++/${GCC_VER} -I${GCC_ROOT}/include/c++/${GCC_VER}/x86_64-unknown-linux-gnu -I${GCC_ROOT}/include/c++/${GCC_VER}/x86_64-pc-linux-gnu -L${GCC_ROOT}/lib64 -Wl,-rpath,${GCC_ROOT}/lib64"
export LDFLAGS="-L${GCC_ROOT}/lib64"
export CXX=${GCC_ROOT}/bin/g++
export CC=${GCC_ROOT}/bin/gcc

configure_extra_options+=" -DPIKA_WITH_CXX_STANDARD=${CXX_STD}"
configure_extra_options+=" -DPIKA_WITH_MAX_CPU_COUNT=128"
configure_extra_options+=" -DPIKA_WITH_MALLOC=system"
configure_extra_options+=" -DPIKA_WITH_COMPILER_WARNINGS=ON"
configure_extra_options+=" -DPIKA_WITH_COMPILER_WARNINGS_AS_ERRORS=ON"
configure_extra_options+=" -DPIKA_WITH_SPINLOCK_DEADLOCK_DETECTION=ON"
configure_extra_options+=" -DPIKA_WITH_TESTS_HEADERS=ON"
configure_extra_options+=" -DPIKA_DATASTRUCTURES_WITH_ADAPT_STD_TUPLE=OFF"

build_extra_options+=" -j10"
