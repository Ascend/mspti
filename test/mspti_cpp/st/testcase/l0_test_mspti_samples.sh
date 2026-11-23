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
# 两阶段调度：多卡用例串行执行，单卡用例按卡并行执行，在保证用例隔离的前提下缩短 l0 冒烟耗时。
currentDir=$(cd "$(dirname "$0")"; pwd)
testScript=${currentDir}/../script/test_mspti_samples.py

MULTI_CARD_CASES=(
    test_hccl_mspti_correlation
    test_python_monitor
    test_python_mstx_monitor
)
SINGLE_CARD_CASES=(
    test_mspti_correlation
    test_mspti_activity
    test_mspti_external_correlation
    test_mspti_mstx_activity_domain
    test_mspti_callback
    test_mspti_callback_domain
)
REQUIRED_CARD_COUNT=8
MAX_PARALLEL=6
success_list=()
failed_list=()

function record_result() {
    local case_name=$1
    local exit_code=$2
    if [ ${exit_code} -eq 0 ]; then
        success_list+=("${case_name}")
        echo "[PASS] ${case_name}"
    else
        failed_list+=("${case_name}")
        echo "[FAIL] ${case_name}"
    fi
}

# 设备池保留 ASCEND_RT_VISIBLE_DEVICES 的真实取值，未设置时回退到探测到的物理卡号，
# 避免在指定卡号的作业中把可见设备重设为 0..N-1 而越界用卡
function detect_device_pool() {
    local pool=()
    local raw_pool=()
    local item
    if [ -n "${ASCEND_RT_VISIBLE_DEVICES}" ]; then
        IFS=',' read -r -a raw_pool <<< "${ASCEND_RT_VISIBLE_DEVICES}"
    fi
    if [ ${#raw_pool[@]} -eq 0 ] && command -v npu-smi >/dev/null 2>&1; then
        local count
        count=$(npu-smi info -l 2>/dev/null | grep -c "NPU ID")
        for ((item = 0; item < count; item++)); do
            raw_pool+=("${item}")
        done
    fi
    if [ ${#raw_pool[@]} -eq 0 ]; then
        for item in /dev/davinci[0-9]*; do
            [ -e "${item}" ] || continue
            raw_pool+=("$(basename "${item}" | sed 's/^davinci//')")
        done
    fi
    for item in "${raw_pool[@]}"; do
        item=${item//[[:space:]]/}
        case "${item}" in
        '' | *[!0-9]*) continue ;;
        esac
        pool+=("${item}")
    done
    echo "${pool[@]}"
}

read -r -a device_pool <<< "$(detect_device_pool)"
card_count=${#device_pool[@]}
if [ "${card_count}" -lt "${REQUIRED_CARD_COUNT}" ]; then
    echo "[ERROR] mspti samples cases require ${REQUIRED_CARD_COUNT} NPU devices, but ${card_count} detected"
    exit 1
fi
parallel=${MAX_PARALLEL}
all_devices=$(printf '%s,' "${device_pool[@]}")
all_devices=${all_devices%,}
total_cases=$((${#MULTI_CARD_CASES[@]} + ${#SINGLE_CARD_CASES[@]}))

echo "NPU devices: ${card_count}, single-card parallelism: ${parallel}"

echo "[INFO] stage 1/2: multi-card cases (sequential)"
for case_name in "${MULTI_CARD_CASES[@]}"; do
    ASCEND_RT_VISIBLE_DEVICES=${all_devices} python3 "${testScript}" --cases="${case_name}"
    record_result "${case_name}" $?
done

echo "[INFO] stage 2/2: single-card cases (parallel)"
declare -a slot_pid
declare -a slot_case
for ((slot = 0; slot < parallel; slot++)); do
    slot_pid[slot]=""
    slot_case[slot]=""
done

FREE_SLOT=0
function acquire_slot() {
    while true; do
        for ((slot = 0; slot < parallel; slot++)); do
            if [ -z "${slot_pid[$slot]}" ]; then
                FREE_SLOT=${slot}
                return 0
            fi
            if ! kill -0 "${slot_pid[$slot]}" 2>/dev/null; then
                wait "${slot_pid[$slot]}"
                record_result "${slot_case[$slot]}" $?
                slot_pid[slot]=""
                FREE_SLOT=${slot}
                return 0
            fi
        done
        sleep 1
    done
}

for case_name in "${SINGLE_CARD_CASES[@]}"; do
    acquire_slot
    slot=${FREE_SLOT}
    slot_case[slot]=${case_name}
    ASCEND_RT_VISIBLE_DEVICES=${device_pool[$slot]} python3 "${testScript}" --cases="${case_name}" &
    slot_pid[slot]=$!
done

for ((slot = 0; slot < parallel; slot++)); do
    if [ -n "${slot_pid[$slot]}" ]; then
        wait "${slot_pid[$slot]}"
        record_result "${slot_case[$slot]}" $?
        slot_pid[slot]=""
    fi
done

echo "=============================================================="
executed_cases=$((${#success_list[@]} + ${#failed_list[@]}))
if [ ${#failed_list[@]} -gt 0 ]; then
    echo "[FAILED] ${#failed_list[@]} case(s): ${failed_list[*]}"
    exit 1
fi
if [ "${executed_cases}" -ne "${total_cases}" ]; then
    echo "[ERROR] expected ${total_cases} case(s), but ${executed_cases} executed"
    exit 1
fi
echo "[SUCCESS] all ${executed_cases} case(s) passed"
exit 0
