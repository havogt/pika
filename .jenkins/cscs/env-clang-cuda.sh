# Copyright (c) 2020 ETH Zurich
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

export CRAYPE_LINK_TYPE=dynamic
export APPS_ROOT="/apps/daint/SSL/pika/packages"
export CXX_STD="17"
export HWLOC_ROOT="${APPS_ROOT}/hwloc-2.0.3-gcc-8.3.0"
export BOOST_ROOT=/apps/dom/UES/jenkins/7.0.UP03/21.09/dom-gpu/software/Boost/1.78.0-CrayGNU-21.09

module load daint-gpu
module load cudatoolkit/11.0.2_3.38-8.1__g5b73779

export CXX=`which CC`
export CC=`which cc`

configure_extra_options+=" -DPIKA_WITH_CXX_STANDARD=${CXX_STD}"
configure_extra_options+=" -DPIKA_WITH_MAX_CPU_COUNT=64"
configure_extra_options+=" -DPIKA_WITH_CUDA=ON"
configure_extra_options+=" -DPIKA_WITH_MALLOC=system"
configure_extra_options+=" -DPIKA_WITH_COMPILER_WARNINGS=ON"
configure_extra_options+=" -DPIKA_WITH_COMPILER_WARNINGS_AS_ERRORS=ON"
configure_extra_options+=" -DPIKA_WITH_SPINLOCK_DEADLOCK_DETECTION=ON"
configure_extra_options+=" -DPIKA_WITH_TESTS_HEADERS=ON"
configure_extra_options+=" -DCMAKE_CUDA_COMPILER=$(which $CXX)"
configure_extra_options+=" -DCMAKE_CUDA_ARCHITECTURES=60"

# The build unit test with pika in Debug and the hello_world project in Debug
# mode hangs on this configuration. Release-Debug, Debug-Release, and
# Release-Release do not hang.
configure_extra_options+=" -DPIKA_WITH_TESTS_EXTERNAL_BUILD=OFF"

# This is a workaround for a bug in the Cray clang compiler and/or Boost. When
# compiling in device mode, Boost detects that float128 is available, but
# compilation later fails with an error message saying float128 is not
# available for the target.
#
# This sets a custom Boost user configuration, which is a concatenation of the
# clang and nvcc compiler configurations, with the exception that
# BOOST_HAS_FLOAT128 is unconditionally disabled.
configure_extra_options+=" \"-DCMAKE_CXX_FLAGS=-I${src_dir}/.jenkins/cscs/ -DBOOST_USER_CONFIG='<boost_user_config_cray_clang.hpp>'\""
configure_extra_options+=" \"-DCMAKE_CUDA_FLAGS=-I${src_dir}/.jenkins/cscs/ -DBOOST_USER_CONFIG='<boost_user_config_cray_clang.hpp>'\""
