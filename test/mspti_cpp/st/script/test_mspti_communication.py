#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
import test_base
import unittest
import logging
import re
import sys
from collections import defaultdict

logging.basicConfig(
    level=logging.INFO, format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s'
)


class MsptiCommunication(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.mspti_communication_path
        self.msprofbin_cmd += f"cd {script_path}; bash run.sh > {self.slog_stdout} 2>&1"

    def getCommActivity(self, content):
        pattern = r"\[Communication\](.*?)correlationId:\s*\d+"
        matches = re.finditer(r"\[Communication\]\s*(.*?correlationId:\s*\d+)", content, re.DOTALL)
        results = []
        for match in matches:
            data_str = match.group(1)
            kv_pairs = re.findall(r"(\w+):\s*([^,]+)", data_str)
            result = {k: v.strip() for k, v in kv_pairs}
            results.append(result)
        return results

    def getApiActivity(self, content):
        pattern = r"\[Api\](.*?)correlationId:\s*\d+"
        matches = re.finditer(r"\[Api\]\s*(.*?correlationId:\s*\d+)", content, re.DOTALL)
        results = []
        for match in matches:
            data_str = match.group(1)
            kv_pairs = re.findall(r"(\w+):\s*([^,]+)", data_str)
            result = {k: v.strip() for k, v in kv_pairs}
            results.append(result)
        return results

    def check_comm_data(self, comm_data_list):
        logging.info("check_comm_data start")
        self.assertEqual(len(comm_data_list), 20, "少了通信算子")
        for comm_data in comm_data_list:
            self.assertEqual(comm_data["name"], "hcom_allReduce_", "名字不对")
            self.assertTrue(comm_data["start"] <= comm_data["end"], "时间不对")
            self.assertEqual(int(comm_data["kind"]), 9, f"communication kind must be 9, but is {comm_data['kind']}")

    def check_api_data(self, api_data_list):
        logging.info("check_api_data start")
        for api_data in api_data_list:
            self.assertTrue(api_data["start"] <= api_data["end"], "时间不对")
            self.assertEqual(int(api_data["kind"]), 3, f"api kind must be 3, but is {api_data['kind']}")

    def check_correlation(self, comm_data_list, api_data_list):
        logging.info("check_correlation start")
        api_group = defaultdict(list)

        # 把所有 Api 分组（按 correlationId）
        for r in api_data_list:
            api_group[r['correlationId']].append(r)

        for comm in comm_data_list:
            corr_id = comm.get('correlationId')
            comm_name = comm.get('name', '').strip()
            api_candidates = api_group.get(corr_id, [])

            matched = any(api.get('name', '').strip() == comm_name for api in api_candidates)
            self.assertTrue(matched, f'Mismatch: Communication="{comm_name}, corr_id={corr_id}"')

    def checkResDir(self, scend=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            comm_data = self.getCommActivity(content)
            api_data = self.getApiActivity(content)
            self.check_comm_data(comm_data)
            self.check_api_data(api_data)
            self.check_correlation(comm_data, api_data)


if __name__ == '__main__':
    suite = unittest.TestSuite()
    timeout = 120
    suite.addTest(MsptiCommunication("test_mspti_communication", "mspti_c", "", "", timeout=timeout))
    runner = unittest.TextTestRunner(verbosity=2)
    test_result = runner.run(suite)
    if not test_result.wasSuccessful():
        sys.exit(1)
    else:
        sys.exit(0)
