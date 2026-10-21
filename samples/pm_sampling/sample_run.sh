#!/usr/bin/env bash
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# Licensed under Mulan PSL v2: http://license.coscl.org.cn/MulanPSL2
set -euo pipefail

sample_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
if [[ -z "${ASCEND_HOME_PATH:-}" ]]; then
    echo "Source the installed CANN set_env.sh before running this sample."
    exit 1
fi
mspti_root=${MSPTI_ROOT:-"${ASCEND_HOME_PATH}/tools/mspti"}
if [[ ! -f "$mspti_root/include/mspti_pm_sampling.h" || ! -f "$mspti_root/lib64/libmspti.so" ]]; then
    echo "Missing installed PM Sampling header or library under: $mspti_root"
    echo "Install a package providing the PM Sampling APIs, or set MSPTI_ROOT to its tools/mspti directory."
    exit 1
fi
if [[ -n "${LD_PRELOAD:-}" ]]; then
    echo "LD_PRELOAD is set. Use a new shell without preloaded libraries to verify the installed package."
    exit 1
fi
mspti_root=$(cd -- "$mspti_root" && pwd)
build_dir=${MSPTI_SAMPLE_BUILD_DIR:-"${HOME}/.cache/mspti/pm_sampling"}
mkdir -p "$build_dir"
build_dir=$(cd -- "$build_dir" && pwd)
export LD_LIBRARY_PATH="$mspti_root/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
log="$build_dir/run_$(date +%Y%m%d_%H%M%S)_$$.log"
echo "Log: $log"
{
    echo "Header: $mspti_root/include/mspti_pm_sampling.h"
    echo "Library: $mspti_root/lib64/libmspti.so"
    sha256sum "$mspti_root/include/mspti_pm_sampling.h" "$mspti_root/lib64/libmspti.so"
    cmake -S "$sample_dir" -B "$build_dir" -DMSPTI_ROOT="$mspti_root" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$build_dir" --parallel 2
    ldd "$build_dir/pm_sampling"
    ulimit -c 0
    timeout --signal=TERM --kill-after=5s 90s "$build_dir/pm_sampling" "$@"
} 2>&1 | tee "$log"
