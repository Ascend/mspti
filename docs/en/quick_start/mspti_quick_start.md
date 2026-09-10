# msPTI Quick Start

## 1. Overview

MindStudio Profiler Tools Interface (msPTI) is a set of performance profiling APIs provided by Huawei Ascend MindStudio. You can use msPTI to build performance analysis tools for NPU applications in inference and training scenarios.

> To get a quick hands-on experience, follow the order **Install the tool → Configure the environment → Run the samples**.

---

## 2. Environment Preparation

### 2.1 Hardware Environment

- A server equipped with an Ascend NPU (see [Ascend product form description](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)).

### 2.2 Software Environment

- **CANN (including msPTI)**: You are advised to install the CANN software (Toolkit + ops packages) first, because msPTI is integrated into CANN. If CANN is already installed, you can use it directly.
  - Quick CANN installation: [Ascend Community CANN download](https://www.hiascend.com/en/cann/download)
- **Python environment**: If you use the msPTI Python API, ensure that a Python 3.10+ environment is configured.
- **PyTorch + torch_npu** (optional): The Python Monitor sample depends on the PyTorch framework and TorchNPU ([installation guide](https://gitcode.com/Ascend/pytorch/blob/master/docs/en/installation_guide/building_from_source.md)).

### 2.3 Constraints

You must not use msPTI together with other performance data collection tools. Otherwise, the collected data will be lost.

---

## 3. Tool Installation

msPTI is integrated into CANN. If CANN is already installed and you do not need to upgrade this tool, you can skip this step.

To upgrade or install the latest version separately, see the [msPTI Tool Installation Guide](../install_guide/mspti_install_guide.md).

After the installation is complete, run the following command to verify:

```bash
pip show mspti
```

If the command outputs the version information without errors, the installation is successful.

---

## 4. Quick Sample Run

### 4.1 Configuring the CANN Environment Variables

```bash
source ${install_path}/set_env.sh
```

Here, `${install_path}` is the CANN installation path, for example, `/usr/local/Ascend/cann`.

### 4.2 Running the Basic Sample

Enter the Activity API basic sample directory and run the following commands:

```bash
cd ${install_path}/tools/mspti/samples/mspti_activity
bash sample_run.sh
```

After a successful run, the output is similar to the following:

```bash
...
========== UserBufferRequest ============
result[0] is: 1.200000
result[1] is: 2.200000
result[2] is: 3.200000
result[3] is: 5.400000
result[4] is: 6.400000
result[5] is: 7.400000
result[6] is: 9.600000
result[7] is: 10.600000
========== UserBufferComplete ============
[RUNTIME_API] name: DevMalloc, start: 1775186328012443375, end: 1775186328012485525, ...
[MEMORY] operationType: ALLOCATION, memoryKind: MEMORY_DEVICE, ...
[RUNTIME_API] name: MemCopySync, start: 1775186328012545645, end: 1775186328012596965, ...
[MEMCPY] copyKind: HTOD, bytes: 32, ...
...
```

---
