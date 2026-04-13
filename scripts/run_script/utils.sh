#!/bin/bash
# right constant
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
root_right=755
user_right=750

script_right=500

root_mspti_right=555
user_mspti_right=550

PATH_LENGTH=4096

MSPTI_RUN_NAME="MindStudio-Profiler-Tools-Interface"
# product constant
LIBMSPTI="libmspti.so"
SAMPLES="samples"
MSPTI_PATH="tools/mspti"
MSPTI_INCLUDE_PATH="${MSPTI_PATH}/include"
MSPTI_LIB_PATH="${MSPTI_PATH}/lib64"
MSPTI_PYTHON_PATH="${MSPTI_PATH}/python"
MSPTI_WHL="mspti-0.0.1-py3-none-any.whl"
MSPTI_HEADER=(
    mspti.h
    mspti_activity.h
    mspti_callback.h
    mspti_cbid.h
    mspti_result.h
)
UNINSTALL_SCRIPT="uninstall.sh"
MSPTI_SHARE_INFO="share/info/mspti"
UNINSTALL_SCRIPT_DIR="${MSPTI_SHARE_INFO}/script"
CANN_UNINSTALL_SCRIPT="cann_uninstall.sh"
UTILS_SCRIPT="utils.sh"
VERSION_INFO="version.info"

# log level
LEVEL_ERROR="ERROR"
LEVEL_WARNING="WARNING"
LEVEL_INFO="INFO"

function print() {
    if [ ! -f "$log_file" ]; then
        echo "[${MSPTI_RUN_NAME}] [$(date +"%Y-%m-%d %H:%M:%S")] [$1]: $2"
    else
        echo "[${MSPTI_RUN_NAME}] [$(date +"%Y-%m-%d %H:%M:%S")] [$1]: $2" | tee -a $log_file
    fi
}

function get_log_file() {
    local log_dir
    if [ "$UID" = "0" ]; then
        log_dir="/var/log/ascend_seclog"
    else
        log_dir="${HOME}/var/log/ascend_seclog"
    fi
    echo "${log_dir}/ascend_install.log"
}

function check_path() {
    local path_str=${1}
    # check the length of path
    if [ ${#path_str} -gt ${PATH_LENGTH} ]; then
        print $LEVEL_ERROR "Invalid parameter: '${path_str}' , the length exceeds ${PATH_LENGTH}."
        exit 1
    fi
    # check absolute path
    if [[ ! "${path_str}" =~ ^/.* ]]; then
        print $LEVEL_ERROR "Invalid parameter: '${path_str}' is not an absolute path."
        exit 1
    fi
    # black list
    if echo "${path_str}" | grep -Eq '/{2,}|\.{3,}'; then
        print $LEVEL_ERROR "The path '${path_str}' is invalid, cannot contain the following characters: // ...!"
        exit 1
    fi
    # white list
    if echo "${path_str}" | grep -Eq '^~?[a-zA-Z0-9./_-]*$'; then
        :
    else
        print $LEVEL_ERROR "The path '${path_str}' is invalid, only [a-z,A-Z,0-9,-,_] is support!"
        exit 1
    fi
    # check the existence of the path
    if [ ! -e "${install_path}" ]; then
        print $LEVEL_ERROR "The path '${install_path}' does not exist, please check."
        exit 1
    fi
}

function remove() {
    local target_path=${1}
    if [ ! -d "${target_path}" ] && [ ! -f "${target_path}" ]; then
        return
    fi
    local parent_dir=$(dirname ${target_path})
    local parent_right=$(stat -c '%a' ${parent_dir})
    chmod u+wx ${parent_dir}
    chmod -R u+wx ${target_path}
    rm -rf ${target_path}
    chmod ${parent_right} ${parent_dir}
}

# init log file
log_file=$(get_log_file)