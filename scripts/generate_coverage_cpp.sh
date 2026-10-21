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

set -euo pipefail
CUR_DIR=$(dirname "$(readlink -f "$0")")
TOP_DIR=$(readlink -f "${CUR_DIR}/..")
BUILD_DIR=${TOP_DIR}/test/build_llt
COV_DIR=${BUILD_DIR}/output/cpp_coverage

LCOV_RC="--ignore-errors inconsistent,mismatch,negative,empty,unused,unsupported
--rc lcov_branch_coverage=1
--rc geninfo_no_exception_branch=1"
GENHTML_RC="--ignore-errors inconsistent,corrupt --branch-coverage"

if [[ ! -d "$BUILD_DIR" ]]; then
    echo "Missing UT build directory; run scripts/execute_test_case.sh first." >&2
    exit 1
fi
mapfile -d '' -t notes < <(find "$BUILD_DIR" -type f -name '*.gcno' -print0)
if (( ${#notes[@]} == 0 )); then
    echo "No coverage notes found; build UT with coverage enabled first." >&2
    exit 1
fi
if [[ -z $(find "$BUILD_DIR" -type f -name '*.gcda' -print -quit) ]]; then
    echo "No coverage counters found; execute UT before generating the report." >&2
    exit 1
fi
mkdir -p "$COV_DIR"
# Discover compiled targets from coverage notes, including targets without an
# _utest suffix. Do not depend on a manually maintained list or object layout.
printf '%s\n' "${notes[@]}" | sed -n 's@/CMakeFiles/\([^/]*\)\.dir/.*@/CMakeFiles/\1.dir@p' |
    sort -u > "$COV_DIR/coverage_targets.txt"
echo "Coverage target directories:"
cat "$COV_DIR/coverage_targets.txt"

# The initial trace keeps compiled but unexecuted code in the denominator.
# Capture the complete build tree so PM Sampling and future UT targets are
# included automatically. Restrict the report to product sources, excluding
# test doubles, test cases and third-party code by source path.
# Set GCOV_TOOL when the UT compiler differs from the default gcov version.
lcov -c -i -d "$BUILD_DIR" --include "$TOP_DIR/csrc/*" --include "$TOP_DIR/mspti/csrc/*" \
    --gcov-tool "${GCOV_TOOL:-gcov}" -o "$COV_DIR/initial.info" $LCOV_RC
lcov -c -d "$BUILD_DIR" --include "$TOP_DIR/csrc/*" --include "$TOP_DIR/mspti/csrc/*" \
    --gcov-tool "${GCOV_TOOL:-gcov}" -o "$COV_DIR/executed.info" $LCOV_RC
lcov -a "$COV_DIR/initial.info" -a "$COV_DIR/executed.info" -o "$COV_DIR/ut_report.info" $LCOV_RC
# An empty report must not be mistaken for a successful coverage measurement.
if ! grep -q '^SF:' "$COV_DIR/ut_report.info"; then
    echo "No product source coverage found in the UT build." >&2
    exit 1
fi
genhtml "$COV_DIR/ut_report.info" -o "$COV_DIR/result" $GENHTML_RC
echo "report: ${COV_DIR}"

if [[ "${1:-}" == "diff" ]];then
  targetBranch=${targetBranch:-master}
  lcov_cobertura "$COV_DIR/ut_report.info" -o "$COV_DIR/coverage.xml"
  diff-cover "$COV_DIR/coverage.xml" --html-report "$COV_DIR/result/ut_incremental_coverage_report.html" --compare-branch="origin/${targetBranch}" --fail-under=80
fi
