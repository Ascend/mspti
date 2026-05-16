#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
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

import os
import re
import unittest
import logging
import sys

import test_base

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MsptiActivityCallbackCase(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        case_path = os.path.join(self.cfg_path.mspti_base_testcase_path, "activity_callback_all")
        build_path = os.path.join(case_path, "build")
        self.msprofbin_cmd = f"cmake -S {case_path} -B {build_path} -DASCEND_HOME_PATH={self.cfg_path.ascend_home_path}; \
        make -C {build_path} -j$(nproc); {build_path}/main > {self.slog_stdout} 2>&1"


class MsptiMarkMultiThreadCase(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        case_path = os.path.join(self.cfg_path.mspti_base_testcase_path, "mark_multithread")
        build_path = os.path.join(case_path, "build")
        self.thread_num = 16
        self.mark_num_per_thread = 40000
        self.msprofbin_cmd = f"cmake -S {case_path} -B {build_path} -DASCEND_HOME_PATH={self.cfg_path.ascend_home_path}; \
        make -C {build_path} -j$(nproc); {build_path}/main {self.thread_num} > {self.slog_stdout} 2>&1"

    def check_mark_num(self):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern = r"MSPTI_SMOKE_MARK_NUM\s+(\d+)"
            match = re.search(pattern, content)
            mark_num = 0
            if match:
                mark_num = int(match.group(1))
            expected_mark_num = self.thread_num * self.mark_num_per_thread
            self.assertEqual(mark_num, expected_mark_num,
                             f"mark total num error. expected {expected_mark_num}, but get {mark_num}")

    def checkResDir(self, scend=None):
        self.check_mark_num()


class MsptiPythonMonitorCase(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        self.case_path = os.path.join(self.cfg_path.mspti_base_testcase_path, "monitor_mnist")
        self.msprofbin_cmd = f"cd {self.case_path};" \
                        r"python3 mnist.py --addr='127.0.0.1' --workers 160 --lr 0.8 --print-freq 1 --dist-url 'tcp://127.0.0.1:50005' " \
                        r"--dist-backend 'hccl' --multiprocessing-distributed --world-size 1 --epochs 1 --rank 0 --device-list '4,5,6,7' --amp " \
                        f"--output {self.res_dir} > {self.slog_stdout} 2>&1"

    def check_marker_count(self):
        expected_marker_count = 7500
        marker_count = 0
        for csv_file in [path for path in os.listdir(self.res_dir) if path.endswith('csv')]:
            csv_file = os.path.join(self.res_dir, csv_file)
            cmd = "grep -r 'MSPTI_ACTIVITY_KIND_MARKER' {0} | grep 'HcclAllreduce' | wc -l".format(csv_file)
            res = self.subprocess_cmd(cmd).strip()
            if res.isdigit():
                marker_count += int(res)
        self.assertEqual(marker_count, expected_marker_count,
                         f"MarkerData num error, expected {expected_marker_count}, but get {marker_count}")

    def check_hccl_count(self):
        expected_hccl_count = 7508
        hccl_count = 0
        for csv_file in [path for path in os.listdir(self.res_dir) if path.endswith('csv')]:
            csv_file = os.path.join(self.res_dir, csv_file)
            cmd = "grep -r 'MSPTI_ACTIVITY_KIND_HCCL' {0} | wc -l".format(csv_file)
            res = self.subprocess_cmd(cmd).strip()
            if res.isdigit():
                hccl_count += int(res)
        self.assertEqual(hccl_count, expected_hccl_count,
                         f"HcclData num error, expected {expected_hccl_count}, but get {hccl_count}")

    def check_kernel_count(self):
        expected_kernel_count = 22512
        kernel_count = 0
        for csv_file in [path for path in os.listdir(self.res_dir) if path.endswith('csv')]:
            csv_file = os.path.join(self.res_dir, csv_file)
            cmd = "grep -r 'MSPTI_ACTIVITY_KIND_KERNEL' {0} | grep 'Relu' | wc -l".format(csv_file)
            res = self.subprocess_cmd(cmd).strip()
            if res.isdigit():
                kernel_count += int(res)
        self.assertEqual(kernel_count, expected_kernel_count,
                         f"KernelData num error, expected {expected_kernel_count}, but get {kernel_count}")

    def checkResDir(self, scend=None):
        self.check_marker_count()
        self.check_hccl_count()
        self.check_kernel_count()


if __name__ == '__main__':
    suite = unittest.TestSuite()
    timeout = 120
    suite.addTest(MsptiActivityCallbackCase("test_MsptiActivityCallbackCase", "", "", "", timeout=timeout))
    suite.addTest(MsptiMarkMultiThreadCase("test_MsptiMarkMultiThreadCase", "", "", "", timeout=timeout))
    suite.addTest(MsptiPythonMonitorCase("test_MsptiPythonMonitorCase", "", "", "", timeout=timeout))
    runner = unittest.TextTestRunner(verbosity=2)
    test_result = runner.run(suite)
    if not test_result.wasSuccessful():
        sys.exit(1)
    else:
        sys.exit(0)
