#!/bin/bash
# This script is used to build libmspti.so, mspti_C.so
# -------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This file is part of the MindStudio project.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#    http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------

set -e
CUR_DIR=$(dirname $(readlink -f $0))
TOP_DIR=${CUR_DIR}/..
BUILD_TYPE="Release"
VERSION=""

print_help() {
    cat << EOF
Usage:
  bash build.sh [options]

Description:
  Used for building mindstudio-profiler-tools-interface_<version>_<arch>.run

Options:
  --help | -h    Show this help message
  Debug          Build with Debug mode, only the first (and only) parameter is valid
  <version>      Build with specified version, only the first (and only) parameter is valid

Examples:
  bash build.sh
  bash build.sh Debug
  bash build.sh v1.2.3
EOF
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    print_help
    exit 0
elif [ $# -gt 1 ]; then
    cat << EOF
[ERROR] Invalid parameters! (only one parameter allowed)
EOF
    print_help
    exit 1
fi

if [ $# -eq 1 ] && [ "$1" = "Debug" ]; then
    BUILD_TYPE="Debug"
elif [ $# -eq 1 ] && [ "$1" != "Debug" ]; then
    VERSION=$1
fi

bash ${CUR_DIR}/download_thirdparty.sh

rm -rf  ${TOP_DIR}/build
cmake -S ${TOP_DIR} -B ${TOP_DIR}/build -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${TOP_DIR}/prefix -DSECUREC_LIB_DIR=${TOP_DIR}/prefix/securec_shared
cd ${TOP_DIR}/build; make -j$(nproc); make install

bash ${TOP_DIR}/scripts/make_run.sh ${VERSION}
