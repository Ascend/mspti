# msPTI工具

## 简介

本文通过提供msPTI样例集的方式，供用户理解使用msPTI工具。

## 产品支持情况

>![](./figures/icon-note.gif) **说明：** 
>昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

| 产品类型 | 是否支持 |
| ------------ | :------: |
| 昇腾950代际产品 | √ |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品  |    √     |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品  |    √     |
| Atlas 200I/500 A2 推理产品  |    √     |
| Atlas 推理系列产品  |    ×     |
| Atlas 训练系列产品   |    ×     |

## 使用前准备

**环境准备**

- 硬件环境请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

- 软件环境请参见《[CANN 软件安装指南](https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/softwareinst/instg/instg_quick.html?Mode=PmIns&InstallType=local&OS=openEuler&Software=cannToolKit)》安装配套版本的CANN Toolkit开发套件包和ops算子包并配置CANN环境变量。
- msPTI Python API部分的样例依赖于PyTorch框架和torch_npu插件，请确保安装。参见《[Ascend Extension for PyTorch](https://gitcode.com/Ascend/pytorch/blob/v2.7.1-7.3.0/docs/zh/installation_guide/installing_PyTorch.md)》中的“安装PyTorch”。

**约束**

msPTI工具不可与任何其他性能数据采集工具同时使用，否则会导致采集的数据丢失。

## msPTI样例集

本节提供msPTI各种接口的使用样例，供用户理解使用msPTI接口。

**构建样例执行<a name="zh-cn_topic_0000002257378834_section6122533557"></a>**

1. 安装CANN软件后，需要以CANN运行用户登录环境，执行`source /${install_path}/set_env.sh`命令设置环境变量。其中`${install_path}`为CANN软件安装后文件存储路径，例如：/usr/local/Ascend/cann。
   示例如下：

   ```bash
   source /usr/local/Ascend/cann/set_env.sh
   ```

2. 进入样例目录。

   msPTI样例代码集成在CANN Toolkit开发套件包中，路径为$\{install\_path\}/tools/mspti/samples。

   $\{install\_path\}请替换为CANN软件安装后文件存储路径。以root安装举例，则安装后文件存储路径为：/usr/local/Ascend/cann。

   示例如下：

   ```bash
   cd ${install_path}/tools/mspti/samples/callback_domain
   ```

3. 执行对应样例目录下的sample\_run.sh。

   ```bash
   bash sample_run.sh
   ```

下表为当前提供的样例介绍：

- Callback API

  | 样例                                             | 说明                                                         |
  | ------------------------------------------------ | ------------------------------------------------------------ |
  | [callback_domain](../../samples/callback_domain) | 1. 展示Callback API功能，可以通过msptiEnableDomain，在runtime API的前后执行Callback操作。 |
  | [callback_mstx](../../samples/callback_mstx)     | 1. 展示Callback与msTX接口相结合功能，使用Callback API和msTX打点功能，在runtime的Launch Kernel前后打点，采集算子数据。<br/> 2. 演示Callback中userdata用法，用户可以通过userdata透传配置或者部分运行参数。 |

- Activity API

  | 样例                                                         | 说明                                                         |
  | ------------------------------------------------------------ | ------------------------------------------------------------ |
  | [mspti_activity](../../samples/mspti_activity)               | 1. 展示Activity API接口的基本功能，样例展示如何采集Kernel和Memory等数据。<br/> 2. 演示Activity API的基本运行，讲述Activity API的基本使用，包括Activity Buffer内存分配，Buffer消费等逻辑。 |
  | [mspti_correlation](../../samples/mspti_correlation)         | 1. 展示Activity API接口的基本功能，展示如何通过correlationId字段将API和Kernel数据做关联。<br/> 2. 演示runtime API下发与Kernel实际执行数据的关联，关联后可以将算子的下发和执行一一对应，方便分析性能瓶颈。 |
  | [mspti_external_correlation](../../samples/mspti_external_correlation) | 1. 展示msPTI External Correlation功能。<br/>2. 演示msptiActivityPopExternalCorrelationId和msptiActivityPushExternalCorrelationId两接口使用方法，用户可以通过接口将各种API关联到一起，方便回溯函数的调用栈。 |
  | [mspti_hccl_activity](../../samples/mspti_hccl_activity)     | 1. 展示Activity API接口的基本功能，样例展示如何通过Hccl开关采集通信数据。 |
  | [mspti_mstx_activity_domain](../../samples/mspti_mstx_activity_domain) | 1. 展示msPTI控制mstxDomain功能，通过开关控制打点数据是否采集。<br/> 2. 用户可以通过msPTI开关实时开关采集打点，减小性能损耗。 |

- Python API

  | 样例                                                     | 说明                                                         |
  | -------------------------------------------------------- | ------------------------------------------------------------ |
  | [python_monitor](../../samples/python_monitor)           | 1.展示Monitor基本使用方式，通过KernelMonitor、HcclMonitor获取计算算子和通信算子的耗时。 |
  | [python_mstx_monitor](../../samples/python_mstx_monitor) | 1. 展示MstxMonitor基本使用方式，用户可以通过Mstx打点采集对应算子（如matmul）耗时。 |
