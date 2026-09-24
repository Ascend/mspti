#!/bin/bash
# -------------------------------------------------------------------------
# This file is part of the MindStudio project.
# Copyright (c) 2025 Huawei Technologies Co.,Ltd.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------

set -e
CUR_DIR=$(dirname $(readlink -f $0))
TOP_DIR=${CUR_DIR}/..

function add_gcov_excl_line_for_mspti() {
    find ${TOP_DIR}/csrc -name "*.cpp" -type f -exec sed -i -e 's/^[[:blank:]]*MSPTI_.*;/& \/\/ LCOV_EXCL_LINE/g' -e '/^[[:blank:]]*MSPTI_.*[,"]$/,/.*;$/ s/;$/& \/\/ LCOV_EXCL_LINE/g' {} \;
}

function add_gcov_excl_line() {
    add_gcov_excl_line_for_mspti
}

function change_file_to_unix_format()
{
    find ${TOP_DIR}/csrc -type f -exec sed -i 's/\r$//' {} +
}

bash ${CUR_DIR}/download_thirdparty.sh

rm -rf ${TOP_DIR}/test/build_llt
mkdir -p ${TOP_DIR}/test/build_llt
cd ${TOP_DIR}/test/build_llt

# gcov 覆盖率统计需要给 MSPTI_* 宏加 LCOV_EXCL_LINE 注释，且需要 Unix 换行。
# 为避免污染源文件，先备份 csrc，修改后编译测试，最后恢复。
BACKUP_DIR=$(mktemp -d)
cp -a ${TOP_DIR}/csrc ${BACKUP_DIR}/csrc_bak || { echo "[execute_test_case] WARN: failed to backup csrc, aborting"; exit 1; }

# trap 必须在 csrc 被修改之前注册，否则 set -e 提前退出时无法恢复
cleanup_and_restore() {
    echo "[execute_test_case] Restoring csrc from backup..."
    rm -rf ${TOP_DIR}/csrc
    cp -a ${BACKUP_DIR}/csrc_bak ${TOP_DIR}/csrc
    rm -rf ${BACKUP_DIR}
}
trap cleanup_and_restore EXIT

change_file_to_unix_format
add_gcov_excl_line

PYTHON_ROOT=$(python3 -c 'import sysconfig; print(sysconfig.get_config_var("prefix"))')
PYTHON_EXEC=$(which python3)

# pyenv Python 的库不在默认路径，需加到 LD_LIBRARY_PATH
PYTHON_LIB_DIR=$(python3 -c 'import sysconfig; print(sysconfig.get_config_var("LIBDIR"))')
export LD_LIBRARY_PATH="${PYTHON_LIB_DIR}:${LD_LIBRARY_PATH}"

cmake ../ -DPACKAGE=ut -DBOOST_INCLUDE_DIRS=${TOP_DIR}/test/opensource/boost -DPython_ROOT_DIR=${PYTHON_ROOT} -DPython_EXECUTABLE=${PYTHON_EXEC}
make -j$(nproc)
