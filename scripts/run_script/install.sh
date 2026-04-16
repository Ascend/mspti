#!/bin/bash
# install constant
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
install_path=${1}
install_for_all_flag=${3}
pylocal=y

function get_right() {
    if [ "$install_for_all_flag" = 1 ] || [ "$UID" = "0" ]; then
        mspti_right=${root_mspti_right}
        right=${root_right}
    else
        mspti_right=${user_mspti_right}
        right=${user_right}
    fi
}

function install_whl_package() {
    local _pylocal=$1
    local _package=$2
    local _pythonlocalpath=$3

    print ${LEVEL_INFO} "Start to install ${_package}."
    if [ ! -f "${_package}" ]; then
        print ${LEVEL_ERROR} "The ${_package} does not exist."
        return 1
    fi
    if [ "-${_pylocal}" = "-y" ]; then
        pip3 install --upgrade --no-deps --force-reinstall "${_package}" -t "${_pythonlocalpath}" > /dev/null 2>&1
    else
        if [ "$(id -u)" -ne 0 ]; then
            pip3 install --upgrade --no-deps --force-reinstall "${_package}" --user > /dev/null 2>&1
        else
            pip3 install --upgrade --no-deps --force-reinstall "${_package}" > /dev/null 2>&1
        fi
    fi
    if [ $? -ne 0 ]; then
        print ${LEVEL_ERROR} "Install ${_package} failed."
        return 1
    fi
    print ${LEVEL_INFO} "Install ${_package} success."
    return 0
}

function implement_install() {
    create_directory ${install_path}/${MSPTI_PATH} ${right}
    create_directory ${install_path}/${MSPTI_INCLUDE_PATH} ${right}
    create_directory ${install_path}/${MSPTI_LIB_PATH} ${right}
    create_directory ${install_path}/${MSPTI_PYTHON_PATH} ${right}
    create_directory ${install_path}/share/info/mspti ${right}
    create_directory ${install_path}/${UNINSTALL_SCRIPT_DIR} ${right}
    for header in "${MSPTI_HEADER[@]}"; do
        copy_file ${header} ${install_path}/${MSPTI_INCLUDE_PATH}/${header} ${mspti_right}
    done
    copy_file ${LIBMSPTI} ${install_path}/${MSPTI_LIB_PATH}/${LIBMSPTI} ${mspti_right}
    copy_file ${SAMPLES} ${install_path}/${MSPTI_PATH}/${SAMPLES} ${right}
    copy_file ${UNINSTALL_SCRIPT} ${install_path}/${UNINSTALL_SCRIPT_DIR}/${UNINSTALL_SCRIPT} ${right}
    copy_file ${UTILS_SCRIPT} ${install_path}/${UNINSTALL_SCRIPT_DIR}/${UTILS_SCRIPT} ${right}
    copy_file ${VERSION_INFO} ${install_path}/${MSPTI_SHARE_INFO}/${VERSION_INFO} ${right}
    # install new whl
    remove ${install_path}/python/site-packages/mspti-*.dist-info
    remove ${install_path}/${MSPTI_PYTHON_PATH}/${MSPTI_WHL}
    mspti_whl_name=$(echo ${MSPTI_WHL})
    mspti_whl="${install_path}/${MSPTI_PYTHON_PATH}/${mspti_whl_name}"
    copy_file "${mspti_whl_name}" "${mspti_whl}" "${right}"
    install_whl_package "${pylocal}" "${mspti_whl}" "${install_path}/python/site-packages"
}

function copy_file() {
    local filename=${1}
    local target_file=$(readlink -f ${2})
    local _right=${3}
    if [ ! -f "${filename}" ] && [ ! -d "${filename}" ]; then
        print ${LEVEL_ERROR} "${filename} does not exist."
        return 1
    fi
    local parent_dir=$(dirname ${target_file})
    local parent_right=$(stat -c '%a' "${parent_dir}")
    chmod u+w "${parent_dir}"
    if [ -f "${target_file}" ] || [ -d "${target_file}" ]; then
        chmod -R u+w "${target_file}"
        rm -rf "${target_file}"
    fi
    cp -r "${filename}" "${target_file}"
    chmod -R "${_right}" "${target_file}"
    chmod "${parent_right}" "${parent_dir}"
    print ${LEVEL_INFO} "${filename} copy success."
    return 0
}

function create_directory() {
    local _dir=${1}
    local _right=${2}
    if [ ! -d "${_dir}" ]; then
        local parent_dir=$(dirname ${_dir})
        if [ ! -d "${parent_dir}" ]; then
            print ${LEVEL_ERROR} "parent directory '${parent_dir}' does not exist"
            exit 1
        fi
        local parent_right=$(stat -c '%a' ${parent_dir})
        chmod u+w ${parent_dir}
        mkdir -p ${_dir}
        chmod ${_right} ${_dir}
        chmod ${parent_right} ${parent_dir}
    fi
}

function register_uninstall() {
    local target_line='uninstall_package "share/info/mspti/script"'
    if [ ! -f "${install_path}/${UNINSTALL_SCRIPT_DIR}/${UNINSTALL_SCRIPT}" ]; then
        print ${LEVEL_ERROR} "No such file: ${install_path}/${UNINSTALL_SCRIPT_DIR}/${UNINSTALL_SCRIPT}"
    fi
    if [ ! -x "${install_path}/${UNINSTALL_SCRIPT_DIR}/${UNINSTALL_SCRIPT}" ]; then
        print ${LEVEL_ERROR} "The file ${install_path}/${UNINSTALL_SCRIPT_DIR}/${UNINSTALL_SCRIPT} is not executable."
        return 1
    fi
    if [ ! -f "${install_path}/${CANN_UNINSTALL_SCRIPT}" ]; then
        print ${LEVEL_ERROR} "Failed to register uninstall script, no such file: ${install_path}/${CANN_UNINSTALL_SCRIPT}"
        return 1
    fi
    if grep -qxF "${target_line}" "${install_path}/${CANN_UNINSTALL_SCRIPT}"; then
        return 0
    fi
    local script_right=$(stat -c '%a' "${install_path}/${CANN_UNINSTALL_SCRIPT}")
    chmod u+w "${install_path}/${CANN_UNINSTALL_SCRIPT}"
    sed -i "/^exit /i uninstall_package \"share\/info\/mspti\/script\"" "${install_path}/${CANN_UNINSTALL_SCRIPT}"
    chmod ${script_right} "${install_path}/${CANN_UNINSTALL_SCRIPT}"
}

source utils.sh

right=${user_right}
get_right
implement_install
if [ $? -eq 0 ]; then
    register_uninstall
fi
