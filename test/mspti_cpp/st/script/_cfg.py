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
import os
import sys
from functools import wraps


def singleton(cls):
    instances = {}

    @wraps(cls)
    def get_instance(*args, **kwargs):
        if cls not in instances:
            instances[cls] = cls(*args, **kwargs)
        return instances[cls]

    return get_instance


@singleton
class ConfigPaths:
    def __init__(self):
        ascend_home_path = os.getenv("ASCEND_HOME_PATH")
        result_path = os.getenv('RESULT_DIR')
        model_path = os.getenv('MODEL_PATH')
        if ascend_home_path is None:
            sys.exit("ASCEND_HOME_PATH is not set")
        if result_path is None:
            sys.exit("RESULT_DIR is not set")
        if model_path is None:
            sys.exit("MODEL_PATH is not set")
        self.ascend_home_path = ascend_home_path
        self.result_path = result_path
        self.mspti_base_testcase_path = os.path.join(model_path, "Mspti")
        self.mspti_sample_path = f"{ascend_home_path}/tools/mspti/samples"
        self.mspti_communication_path = os.path.join(self.mspti_base_testcase_path, "mspti_communication")


class ConfigValues:
    pass_res = "pass"
    fail_res = "fail"
