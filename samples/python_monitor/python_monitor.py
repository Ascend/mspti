#!/usr/bin/python3
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
import threading
import time
import logging
from multiprocessing import Queue

import torch

from mspti import KernelData, KernelMonitor, CommunicationData, CommunicationMonitor

# parser会被多线程调用，所以使用queue存储数据，保证多线程安全
data_queue = Queue()
logging.basicConfig(
    format='%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s', level=logging.INFO
)


def kernel_parser(data: KernelData):
    data_queue.put(data)


def communication_parser(data: CommunicationData):
    data_queue.put(data)


def consumer_func(consume_queue):
    while True:
        if not consume_queue.empty():
            data = consume_queue.get()
            if data is None:
                break
            if isinstance(data, KernelData):
                logging.info(
                    '%s, %s, %s, %s, %s, %s, %s, %s',
                    data.kind,
                    data.start,
                    data.end,
                    data.device_id,
                    data.stream_id,
                    data.correlation_id,
                    data.type,
                    data.name,
                )
            elif isinstance(data, CommunicationData):
                logging.info(
                    '%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s',
                    data.kind,
                    data.start,
                    data.end,
                    data.device_id,
                    data.stream_id,
                    data.data_type,
                    data.count,
                    data.name,
                    data.comm_name,
                    data.alg_type,
                    data.correlation_id,
                )
        else:
            time.sleep(0.1)


def init_process(backend="hccl"):
    torch.distributed.init_process_group(backend=backend, init_method='env://')


def test_monitor():
    consumer = threading.Thread(target=consumer_func, args=(data_queue,))
    consumer.start()

    # enable mspti monitor to collect activity
    kernel_monitor = KernelMonitor()
    kernel_monitor.start(kernel_parser)
    communication_monitor = CommunicationMonitor()
    communication_monitor.start(communication_parser)

    init_process()
    device = int(os.getenv('LOCAL_RANK'))
    torch.npu.set_device(device)

    width = 256

    x = torch.randn(width, width, dtype=torch.float16).npu()
    y = torch.randn(width, width, dtype=torch.float16).npu()

    result = x + y
    result = torch.matmul(x, y)
    torch.distributed.all_reduce(result)
    torch.npu.synchronize()

    # stop mspti monitor and consume activity
    kernel_monitor.stop()
    communication_monitor.stop()
    data_queue.put(None)
    consumer.join()


if __name__ == "__main__":
    test_monitor()
