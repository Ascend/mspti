# msPTI Tool Sample Guide

## Introduction

This document provides msPTI samples to help users understand and use the msPTI tool.

## Supported Products

>![](../figures/icon-note.gif) **NOTE:**
>For details about Ascend product models, see [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

| Product Type | Supported |
| ------------ | :------: |
| Atlas 350 accelerator cards | √ |
| Atlas A3 training products/Atlas A3 inference products | √ |
| Atlas A2 training products/Atlas A2 inference products | √ |
| Atlas 200I/500 A2 inference products | √ |
| Atlas inference products | × |
| Atlas training products | × |

## Preparation

**Environment Preparation**

- Hardware environment: See [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

- Software environment: Refer to the [CANN Software Installation Guide](https://www.hiascend.com/document/detail/en/canncommercial/83RC1/softwareinst/instg/instg_quick.html?Mode=PmIns&InstallType=local&OS=openEuler&Software=cannToolKit) to install the mapped CANN Toolkit and the ops package, and configure the CANN environment variables.
- The samples in the msPTI Python API section depend on the PyTorch framework and the torch_npu plugin. Ensure that they are installed. For details, see "Installing PyTorch" in [Ascend Extension for PyTorch](https://gitcode.com/Ascend/pytorch/blob/v2.7.1-26.0.0/docs/en/installation_guide/installing_PyTorch.md).

**Constraints**

The msPTI tool cannot be used simultaneously with any other performance data collection tools. Otherwise, the collected data will be lost.

## msPTI Samples

This section provides samples for using various msPTI APIs to help users understand how to use msPTI APIs.

**Sample Execution<a name="zh-cn_topic_0000002257378834_section6122533557"></a>**

1. After installing the CANN software, log in to the environment as the CANN running user and run the `source /${install_path}/set_env.sh` command to set the environment variables. `${install_path}` is the file storage path after CANN software installation, for example: `/usr/local/Ascend/cann`.
   Example:

   ```bash
   source /usr/local/Ascend/cann/set_env.sh
   ```

2. Go to the sample directory.

   The msPTI sample code is integrated in the CANN Toolkit package, with the path `$\{install\_path\}/tools/mspti/samples`.

   Replace `$\{install\_path\}` with the file storage path after CANN software installation. For example, if installed as root, the file storage path after installation is: `/usr/local/Ascend/cann`.

   Example:

   ```bash
   cd ${install_path}/tools/mspti/samples/callback_domain
   ```

3. Run `sample\_run.sh` in the corresponding sample directory.

   ```bash
   bash sample_run.sh
   ```

The following table provides introductions to the samples:

- Callback API

  | Sample                                             | Description                                                         |
  | ------------------------------------------------ | ------------------------------------------------------------ |
  | [callback_domain](../../../samples/callback_domain) | 1. Demonstrates the Callback API feature, which allows you to perform callback operations before and after runtime APIs through msptiEnableDomain. |
  | [callback_mstx](../../../samples/callback_mstx)     | 1. Demonstrates the integration of Callback and msTX APIs. Uses the Callback API and msTX marking feature to mark points before and after the Launch Kernel of the runtime, collecting operator data.<br/> 2. Demonstrates the usage of userdata in Callback, where users can transparently transmit configurations or some runtime parameters through userdata. |

- Activity API

  | Sample                                                         | Description                                                         |
  | ------------------------------------------------------------ | ------------------------------------------------------------ |
  | [mspti_activity](../../../samples/mspti_activity)               | 1. Demonstrates the basic features of the Activity API. The sample shows how to profile data such as Kernel and Memory.<br/> 2. Demonstrates the basic operation of the Activity API, describing its basic usage, including logic such as Activity Buffer memory allocation and Buffer consumption. |
  | [mspti_correlation](../../../samples/mspti_correlation)         | 1. Demonstrates the basic features of the Activity API, showing how to correlate API and Kernel data using the correlationId field.<br/> 2. Demonstrates the correlation between runtime API dispatch and actual Kernel execution data. After correlation, the dispatch and execution of operators can be matched one-to-one, facilitating performance bottleneck analysis. |
  | [mspti_external_correlation](../../../samples/mspti_external_correlation) | 1. Demonstrates the msPTI External Correlation Feature.<br/>2. Demonstrates the usage of the msptiActivityPopExternalCorrelationId and msptiActivityPushExternalCorrelationId APIs, which allow users to correlate various APIs together, facilitating backtracking of function call stacks. |
  | [mspti_hccl_activity](../../../samples/mspti_hccl_activity)     | 1. Demonstrates the basic Features of the Activity API. The sample shows how to collect communication data using the Hccl switch. |
  | [mspti_mstx_activity_domain](../../../samples/mspti_mstx_activity_domain) | 1. Demonstrates the msPTI control of the mstxDomain Feature, controlling whether marking data is collected through a switch.<br/> 2. Users can use the msPTI switch to enable or disable marking collection in real time, reducing performance overhead. |

- Python API

  | Sample                                                     | Description                                                         |
  | -------------------------------------------------------- | ------------------------------------------------------------ |
  | [python_monitor](../../../samples/python_monitor)           | 1. Demonstrates the basic usage of Monitor, obtaining the elapsed time of computation operators and communication operators through KernelMonitor and HcclMonitor. |
  | [python_mstx_monitor](../../../samples/python_mstx_monitor) | 1. Demonstrates the basic usage of MstxMonitor, where users can collect the elapsed time of corresponding operators (such as matmul) through Mstx marking. |
  