#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
import os
import re
import shutil
import time
import logging

import subprocess
import unittest
from abc import abstractmethod
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from _cfg import *

logging.basicConfig(
    level=logging.INFO, format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s'
)


class TestProfiling(unittest.TestCase):
    def __init__(self, tc, scene, mode, params, timeout):
        self.id = tc
        self.scene = scene
        self.mode = mode
        self.params = params
        self.timeout = timeout
        self.logger = logging
        self.cfg_path = ConfigPaths()
        self.start_time = 0
        self.duration_time = 0
        self.line = "-" * 30
        super(TestProfiling, self).__init__("execute")

    def initTest(self):
        self.start_time = time.time()
        self.cfg_value = ConfigValues()
        self.msprofbin_cmd = ""
        self.res_dir = os.path.join(self.cfg_path.result_path, self.id)
        self.res = 0
        self.__result = self.cfg_value.pass_res
        self.slog_stdout = "{}.log".format(os.path.join(self.res_dir, self.id))
        self.plog_path = os.path.join(self.res_dir, "plog")
        os.environ["ASCEND_PROCESS_LOG_PATH"] = self.plog_path
        self.logger.info(self.line + f" Start execute case {self.id} " + self.line)

    def write_res(self):
        if self.res != 0:
            self.__result = self.cfg_value.fail_res
        cost_time = time.time() - self.start_time
        minutes = int(cost_time // 60)
        seconds = int(cost_time % 60)
        duration_str = f"{minutes}min, {seconds}s"
        self.logger.info(self.line + f" End execute case {self.id} ({duration_str}) " + self.line)
        if not os.path.isdir(self.cfg_path.result_path):
            os.makedirs(self.cfg_path.result_path)
        with open(os.path.join(self.cfg_path.result_path, 'result.txt'), 'a+') as f:
            f.write('%s %s %s\n' % (self.id, self.__result, self.duration_time))

    def execute(self):
        self.initTest()
        self.clear_result_dir()
        self.getTestCmd()
        try:
            self.executeCmd()
            self.checkResDir()
        except Exception as e:
            self.logger.exception(e)
            self.__result = self.cfg_value.fail_res
        self.write_res()
        self.assertFalse(self.cfg_value.fail_res == self.__result)

    @abstractmethod
    def getTestCmd(self, scene=None):
        pass

    @abstractmethod
    def checkResDir(self, scene=None):
        self.__result = "pass"
        pass

    def subprocess_cmd(self, cmd):
        self.logger.info("host command: {}".format(cmd))
        try:
            result = subprocess.run(['bash', '-c', cmd], capture_output=True, text=True)
            if result.returncode != 0 and len(result.stderr) != 0:
                self.logger.error(result.stderr)
                self.res += 1
        except (Exception, TimeoutError) as err:
            self.logger.error(err)
            self.res += 1
            return err
        finally:
            pass
        return result.stdout

    def view_error_msg(self, argv_path, log_type):
        self.logger.info("start view {} log ...".format(log_type))
        if log_type == "plog":
            cmd = (
                r"grep -rn 'ERROR\] PROFILING' {0}; grep -rn '\[ERROR\] \[MSVP\]' {0}; "
                r"grep -rn 'ERROR\] Failed' {0}".format(argv_path)
            )
        else:
            cmd = r"grep -rn 'ERROR\]' {0}".format(argv_path)
        res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.logger.error(res)
            self.res += 1

    def clear_result_dir(self):
        if os.path.isdir(self.res_dir):
            shutil.rmtree(self.res_dir)
            self.logger.info("clear output ok ...")
        os.makedirs(self.res_dir)

    def executeCmd(self):
        start_time = time.time()
        output = self.subprocess_cmd(self.msprofbin_cmd)
        if output:
            self.logger.info(output)
        self.duration_time = int(time.time() - start_time)
        self.view_error_msg(self.plog_path, "plog")
        self.view_error_msg(self.slog_stdout, "screen")
