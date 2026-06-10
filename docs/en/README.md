# msPTI Tool

## Overview

This document provides msPTI sample sets to help you understand and use the msPTI tool.

## Supported Products

>![](./figures/icon-note.gif) res/icon-note.gif) **NOTE:**
>For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

| Product Type| Supported|
| ------------ | :------: |
| Ascend 950 products| √ |
| Atlas A3 training products/Atlas A3 inference products |    √     |
| Atlas A2 training products/Atlas A2 inference products |    √     |
| Atlas 200I/500 A2 inference products |    √     |
| Atlas inference products |    ×     |
| Atlas training series products  |    ×     |

## Preparations

**Environment Preparation**

- For hardware environment requirements, see [Ascend Product Models](<>).

- For details about the software environment, see the CANN Toolkit and ops operator package installation guide of the corresponding version in the CANN Software Installation Guide (https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/softwareinst/instg/instg_quick.html?Mode=PmIns&InstallType=local&OS=openEuler&Software=cannToolKit). Install the CANN Toolkit and ops operator package and configure the CANN environment variables.
- The msPTI Python API samples depend on the PyTorch framework and torch_npu plugin. Ensure that they are installed. For details, see "Installing PyTorch" in the [Ascend Extension for PyTorch](https://gitcode.com/Ascend/pytorch/blob/v2.7.1-7.3.0/docs/zh/installation_guide/installing_PyTorch.md).

**Constraints**

The msPTI tool cannot be used together with any other performance data collection tools. Otherwise, the collected data will be lost.

## msPTI sample set

This section provides examples of using msPTI APIs for users to understand how to use msPTI APIs.

**Building and Executing Samples<a name="zh-cn_topic_0000002257378834_section6122533557"></a>**

1. After installing the CANN software, log in to the environment as the CANN running user and run the `source /${install_path}/set_env.sh` command to set environment variables. In the preceding command, `${install_path}` indicates the path for storing the CANN software installation files, for example, /usr/local/Ascend/cann.
   Example:

   ```bash
   source /usr/local/Ascend/cann/set_env.sh
   ```

2. Go to the sample directory.

   The msPTI sample code is integrated in the CANN Toolkit development kit, which is stored in $\{install\_path\}/tools/mspti/samples.

   Replace $\{install\_path\} with the path for storing the CANN software installation files. For example, if the installation is performed by the `root` user, the path is `/usr/local/Ascend/cann`.

   Example:

   ```bash
   cd ${install_path}/tools/mspti/samples/callback_domain
   ```

3. Run the sample\_run.sh script in the corresponding sample directory.

   ```bash
   bash sample_run.sh
   ```

The following table describes the samples provided currently.

- Callback API

  | Example                                            | Description                                                        |
  | ------------------------------------------------ | ------------------------------------------------------------ |
  | [callback_domain](../../samples/callback_domain) | 1. Demonstrates the callback API function. You can call the msptiEnableDomain API to perform callback operations before and after the runtime API.|
  | [callback_mstx](../../samples/callback_mstx)     | 1. Demonstrates the function of combining the callback and mstx APIs. The callback API and mstx API are used to collect operator data by performing dotting operations before and after the runtime kernel launch.<br> 2. Demonstrates the usage of userdata in the callback. You can use userdata to transparently transmit configurations or some running parameters.|

- Activity API

  | Example                                                        | Description                                                        |
  | ------------------------------------------------------------ | ------------------------------------------------------------ |
  | [mspti_activity](../../samples/mspti_activity)               | 1. Demonstrates the basic functions of the Activity API. It shows how to profile kernels and memory.<br> 2. This sample demonstrates the basic running of the Activity API, including the basic usage of the Activity API, such as activity buffer allocation and buffer consumption.|
  | [mspti_correlation](../../samples/mspti_correlation)         | 1. Demonstrates the basic functions of the Activity API. It shows how to use the **correlationId** field to correlate the API with the kernel data.<br> 2. This sample demonstrates the association between the runtime API delivery and the actual kernel execution data. After the association, the operator delivery and execution can be mapped to each other, facilitating performance bottleneck analysis.|
  | [mspti_external_correlation](../../samples/mspti_external_correlation) | 1. This sample demonstrates the msPTI external correlation function.<br>2. This sample demonstrates the usage of the msptiActivityPopExternalCorrelationId and msptiActivityPushExternalCorrelationId APIs. You can use these APIs to associate various APIs to facilitate the backtracking of the function call stack.|
  | [mspti_hccl_activity](../../samples/mspti_hccl_activity)     | 1. This sample demonstrates the basic functions of the Activity API and how to collect communication data by using the Hccl switch.|
  | [mspti_mstx_activity_domain](../../samples/mspti_mstx_activity_domain) | 1. This sample demonstrates the function of controlling the mstxDomain by using the msPTI. You can use the switch to control whether to collect the dotting data.<br> 2. You can use the msPTI switch to enable or disable the collection of dotting data in real time, reducing performance loss.|

- Python API

  | Example                                                    | Description                                                        |
  | -------------------------------------------------------- | ------------------------------------------------------------ |
  | [python_monitor](../../samples/python_monitor)           | 1. This sample demonstrates the basic usage of the Monitor, and how to obtain the time consumed by the compute operator and communication operator through KernelMonitor and HcclMonitor.|
  | [python_mstx_monitor](../../samples/python_mstx_monitor) | 1. This example demonstrates how to use MstxMonitor. You can use Mstx to collect the time consumed by operators (such as matmul).|
