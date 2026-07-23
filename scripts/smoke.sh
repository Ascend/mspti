#!/bin/bash
# This script is for smoke testing.
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
LEVEL_ERROR="ERROR"
LEVEL_WARN="WARNING"
LEVEL_INFO="INFO"
CUR_DIR=$(dirname $(readlink -f $0))
TOP_DIR=$(readlink -f "${CUR_DIR}/..")
MSPTI_SMOKE="MSPTI_SMOKE"
cann_path="/usr/local/Ascend"
conda_path=""
conda_env=""
smoke_level="l0"
output="${TOP_DIR}/mspti_st_result_dir"
model_path=""
success_list=()
failed_list=()
test_list=()
line=$(printf '=%.0s' {1..70})

show_help() {
    cat << EOF
Usage:
  bash smoke.sh [options]

Description:
  Used for executing smoke testing, supporting specification of CANN path, conda environment, smoke test type, etc.

Options:
  --cann-path=<path>        Specify the installation path of CANN (default: /usr/local/Ascend)
  --conda-path=<path>       Specify the installation path of conda
  --conda-env=<env-name>    Specify the name of the conda environment to activate
  --model-path=<path>       Specify the path of the model to be tested (required except l0)
  --smoke-level=<level>     Specify the type of smoke test (default: l0)
                              all - Execute all scripts containing "test"
                              l1 - Execute scripts containing "l1_test"
                              l0 - Execute scripts containing "l0_test"
  --case=<case1,case2>      Specify the name of the test case to execute
  --help | -h               Show this help message

Examples:
  bash smoke.sh
  bash smoke.sh --cann-path=/usr/local/Ascend
EOF
}

function print() {
    local line_no=${BASH_LINENO[0]}
    echo "[${MSPTI_SMOKE}] [$(date +"%Y-%m-%d %H:%M:%S")] [$line_no] [$1]: $2"
}

function parse_script_args() {
    while true; do
        if [ "$1" = "" ]; then
            break
        fi
        case "$1" in
        -h | --help)
            show_help
            exit 0
            ;;
        --cann-path=*)
            cann_path=${1#--cann-path=}
            shift
            continue
            ;;
        --smoke-level=*)
            smoke_level=${1#--smoke-level=}
            shift
            continue
            ;;
        --conda-path=*)
            conda_path=${1#--conda-path=}
            shift
            continue
            ;;
        --conda-env=*)
            conda_env=${1#--conda-env=}
            shift
            continue
            ;;
        --model-path=*)
            model_path=${1#--model-path=}
            shift
            continue
            ;;
        --case=*)
            case=${1#--case=}
            IFS=',' read -r -a test_list <<< "$case"
            shift
            continue
            ;;
        *)
            print $LEVEL_ERROR "Input option '$1' is invalid."
            show_help
            exit 1
            ;;
        esac
    done
}

function run_smoke() {
    cd $TOP_DIR/test/mspti_cpp/st/testcase
    rm -f "${output}/result.txt"
    if [ ${#test_list[@]} -eq 0 ]; then
        if [ "${smoke_level}" = "all" ]; then
            mapfile -t test_list < <(ls | grep "\.sh$" | grep -E "test")
        elif [ "${smoke_level}" = "l1" ]; then
            mapfile -t test_list < <(ls |grep ".sh" | grep -E "l1_test")
        elif [ "${smoke_level}" = "l0" ]; then
            mapfile -t test_list < <(ls |grep ".sh" | grep -E "l0_test")
        else
            print $LEVEL_ERROR "Invalid smoke level: ${smoke_level}"
            exit 1
        fi
    fi

    if [[ ${#test_list[@]} -gt 0 ]]; then
        print $LEVEL_INFO "There are test_list:"
        for item in "${test_list[@]}"; do
            print $LEVEL_INFO "- $item"
        done
        for item in "${test_list[@]}"; do
            print $LEVEL_INFO "${line}${line}"
            print $LEVEL_INFO "[START] $item"
            start_time=$(date +%s)
            bash $item
            exit_code=$?
            end_time=$(date +%s)
            cost_time=$((end_time - start_time))
            minutes=$((cost_time / 60))
            seconds=$((cost_time % 60))
            duration_str="${minutes}min, ${seconds}s"
            if [ $exit_code -eq 0 ]; then
                print $LEVEL_INFO "[SUCCESS] $item"
                success_list+=("$item ($duration_str)")
            else
                print $LEVEL_ERROR "[FAILED] $item"
                failed_list+=("$item ($duration_str)")
            fi
        done
    else
        print $LEVEL_ERROR "No test cases were found."
        exit 1
    fi

    print $LEVEL_INFO "${line} SUMMARY ${line}"
    if [ ${#success_list[@]} -gt 0 ]; then
        print $LEVEL_INFO "Number of successfully executed scripts: ${#success_list[@]}"
        print $LEVEL_INFO "Success list:"
        for script in "${success_list[@]}"; do
            print $LEVEL_INFO "- $script"
        done
    fi
    if [ ${#failed_list[@]} -gt 0 ]; then
        print $LEVEL_ERROR "Number of failed scripts: ${#failed_list[@]}"
        print $LEVEL_ERROR "Failed list:"
        for script in "${failed_list[@]}"; do
            print $LEVEL_ERROR "- $script"
        done
    fi
    print $LEVEL_INFO "${line}${line}"
    if [ ${#failed_list[@]} -gt 0 ]; then
        exit 1
    fi
}

function init_env() {
    if [[ "$smoke_level" != "l0" && -z "${model_path}" ]]; then
        print $LEVEL_ERROR "Please specify the path of the model to be tested"
        exit 1
    fi
    if [ -n "${conda_path}" ] && [ -n "${conda_env}" ]; then
        if ! source "${conda_path}/bin/activate"; then
            print $LEVEL_ERROR "Failed to source conda activate script"
            exit 1
        fi

        if ! conda activate "${conda_env}"; then
            print $LEVEL_ERROR "Failed to activate conda environment '${conda_env}'"
            exit 1
        fi
    elif [ -n "${conda_path}" ] || [ -n "${conda_env}" ]; then
        print $LEVEL_WARN "Both --conda-path and --conda-env must be specified together, skipping conda environment activation"
    fi
}

function install_mspti() {
    bash ${CUR_DIR}/build.sh
    if [ $? -ne 0 ]; then
        print $LEVEL_ERROR "Failed to build mspti"
        exit 1
    fi
    cd ${TOP_DIR}/output
    RUN_FILE=$(ls -1 mindstudio-profiler-tools-interface*.run 2>/dev/null | head -n 1)
    if [ -z "$RUN_FILE" ]; then
        print $LEVEL_ERROR "Can't find mspti run package"
        exit 1
    fi

    bash $RUN_FILE --install --install-path=${cann_path}/cann
    if [ $? -ne 0 ]; then
        print $LEVEL_ERROR "Failed to install mspti"
        exit 1
    fi
}

function set_env() {
    if ! source "${cann_path}/cann/set_env.sh"; then
        print $LEVEL_ERROR "Failed to source set_env.sh"
        exit 1
    fi
    export LD_PRELOAD="${cann_path}/cann/lib64/libmspti.so"
    export RESULT_DIR=${output}
    export MODEL_PATH=${model_path}
}

parse_script_args $*
init_env
install_mspti
set_env

run_smoke
