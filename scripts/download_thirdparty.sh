#!/bin/bash
# This script is used to download thirdpart needed by mspti.
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

OPENSOURCE_DIR=${TOP_DIR}/opensource
PLATFORM_DIR=${TOP_DIR}/platform
LLT_DIR=${TOP_DIR}/test/opensource

THIRDPARTY_LIST="${OPENSOURCE_DIR}/makeself    \\
                 ${PLATFORM_DIR}/securec       \\
                 ${LLT_DIR}/googletest         \\
                 ${LLT_DIR}/mockcpp"

if [ -n "$1" ]; then
    if [ "$1" == "force" ]; then
        echo "force delete origin opensource files"
        echo ${THIRDPARTY_LIST}
        rm -rf ${THIRDPARTY_LIST}
    fi
fi

function patch_makeself() {
    cd ${OPENSOURCE_DIR}
    git clone -b v2.5.0.x https://gitcode.com/cann-src-third-party/makeself.git
    cd ${OPENSOURCE_DIR}/makeself
    tar -zxf makeself-release-2.5.0.tar.gz
    cd makeself-release-2.5.0
    ulimit -n 8192
    patch -p1 < ../makeself-2.5.0.patch
    cd ${OPENSOURCE_DIR}/makeself
    cp -r makeself-release-2.5.0 ${OPENSOURCE_DIR}
    cd ${OPENSOURCE_DIR}
    rm -rf makeself
    mv makeself-release-2.5.0 makeself
}

function download_boost() {
    local boost_url="https://inst.obs.cn-north-4.myhuaweicloud.com/env/mirror/boost_1_81_0.tar.bz2"
    local boost_archive="boost_1_81_0.tar.bz2"
    local boost_sha256="71feeed900fbccca04a3b4f2f84a7c217186f28a940ed8b7ed4725986baf99fa"

    mkdir -p ${LLT_DIR}
    cd ${LLT_DIR}
    [ -d "boost" ] && return 0

    curl -Lfk --retry 5 --retry-delay 2 -o "${boost_archive}" "${boost_url}"
    echo "${boost_sha256}  ${boost_archive}" | sha256sum -c -
    tar -xjf "${boost_archive}"
    mv boost_1_81_0 boost
    rm -f "${boost_archive}"
}


mkdir -p ${OPENSOURCE_DIR} && cd ${OPENSOURCE_DIR}
[ ! -d "makeself" ] && patch_makeself


download_boost
mkdir -p ${LLT_DIR} && cd ${LLT_DIR}
if [ ! -d "googletest" ]; then
    git clone https://gitcode.com/GitHub_Trending/go/googletest.git -b release-1.10.0
    # 移除 googletest 内部 -Werror，避免 GCC 编译失败
    sed -i 's/set(cxx_base_flags "-Wall -Wshadow -Werror")/set(cxx_base_flags "-Wall -Wshadow")/' \
        googletest/googletest/cmake/internal_utils.cmake
    sed -i 's/set(cxx_base_flags "-Wall -Wshadow -Werror -Wconversion")/set(cxx_base_flags "-Wall -Wshadow -Wconversion")/' \
        googletest/googletest/cmake/internal_utils.cmake
fi
[ ! -d "mockcpp" ] && git clone https://gitcode.com/hhz0/mockcpp.git -b msprof

mkdir -p ${PLATFORM_DIR} && cd ${PLATFORM_DIR}
[ ! -d "securec" ] && git clone https://gitcode.com/openeuler/libboundscheck.git securec
cd ${TOP_DIR}
