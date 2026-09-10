# msPTI FAQ

## Product Overview

### Q1: What Is msPTI?

MindStudio Profiler Tools Interface (msPTI) is a set of profiling APIs provided by Huawei Ascend MindStudio. You can use msPTI to build performance analysis tools for NPU applications in inference and training scenarios. For details, see [msPTI Quick Start](../quick_start/mspti_quick_start.md).

### Q2: What Core Capabilities Does msPTI Provide?

msPTI provides the following core capabilities:

- **Tracing**: Collects the execution timestamps and metadata of CANN applications, including CANN API calls, Kernel execution, memory copies, communication operations, and user-defined markers, helping you locate execution bottlenecks.
- **Profiling**: Collects the NPU performance metrics of a single Kernel or a group of Kernels, supporting computation and communication analysis.

### Q3: Which Product Models Does msPTI Support?

| Product Type | Supported |
| --- | :---: |
| Ascend 950 products | √ |
| Atlas A3 training products/Atlas A3 inference products | √ |
| Atlas A2 training products/Atlas A2 inference products | √ |
| Atlas 200I/500 A2 inference products | √ |
| Atlas inference products | × |
| Atlas training products | × |

### Q4: Does msPTI Support Windows Environments?

No. msPTI depends on the Linux operating system and Ascend NPU hardware, and currently supports only the Linux environment.

---

## Installation and Uninstallation

### Q5: How Do I Install msPTI?

msPTI is integrated into CANN. If CANN is installed and you do not need to upgrade this tool, you can use it directly. To install or upgrade msPTI separately, use the following methods:

| Installation Method | Applicable Scenario |
| --- | --- |
| Online installation | The device has internet access |
| Offline installation | An environment without external network access, such as an enterprise intranet |
| Source installation | You need to use the latest development code |

For detailed installation steps, see the [msPTI Tool Installation Guide](../install_guide/mspti_install_guide.md).

### Q6: How Do I Verify That the Installation Is Successful?

Run the following command:

```bash
pip show mspti
```

If the output displays the version information without errors, the installation is successful.

### Q7: How Do I Uninstall msPTI?

Perform the following steps:

```bash
curl -O https://inst.obs.cn-north-4.myhuaweicloud.com/26.1.0/ms_install.py
python ms_install.py uninstall {tools_name}
```

You can query the value of `{tools_name}` by running the `python ms_install.py help` command. If the environment does not have network access, download `ms_install.py` on a device that has network access and copy it to the target device.

### Q8: How Do I Upgrade msPTI?

Upgrading means uninstalling the old version first and then installing the new one. Run the installation command of the new version directly. The tool automatically uninstalls the old version and guides you through the overwrite installation. Before upgrading, check the current version by running `pip show mspti` and pay attention to version compatibility. See [Release Notes](https://gitcode.com/Ascend/mspti/releases).

### Q9: What Parameters Are Supported When Installing the Run Package?

| Parameter | Description |
| --- | --- |
| `--install` | Installs the software package. You can use it with `--install-path` to specify the path. |
| `--uninstall` | Uninstalls the software package. |
| `--install-path=<path>` | Specifies the installation path, which must point to the CANN layer directory. |
| `--install-for-all` | Allows other users to have the permissions of the installation user group. This poses a security risk. Therefore, use it with caution. |

### Q10: What If the MD5 Checksum Does Not Match?

If the output of `md5sum -c -` shows `FAILED`, do not continue the installation. Delete the current file, download it again, and run the MD5 checksum again. If it still fails, check that the file name and version match those on the releases page, and report the issue through [Issues](https://gitcode.com/Ascend/mspti/issues).

---

## Environment and Configuration

### Q11: What Are the Prerequisites for Using msPTI?

- **Hardware**: a server equipped with Ascend NPUs
- **Software**: CANN Toolkit and the ops operator package (version ≥ 8.5.0)
- **Python version**: Python 3.10 or later if you use the Python API.

### Q12: Can msPTI Be Used Together with Other Performance Collection Tools?

**No**. Do not use msPTI together with other performance data collection tools. Otherwise, the collected data will be lost.

---

## Usage Questions

### Q13: How Do I Get Started with msPTI Quickly?

You are advised to perform the following steps in order:

1. Install CANN and configure environment variables.
2. Enter the sample directory: `cd ${install_path}/tools/mspti/samples/mspti_activity`
3. Run the sample script: `bash sample_run.sh`

For detailed instructions, see [msPTI Quick Start](../quick_start/mspti_quick_start.md).

### Q14: Which APIs Does msPTI Provide?

msPTI provides the following types of APIs:

| API Type | Description |
| --- | --- |
| **C Activity API** | Collects activity data such as Kernel, Memory, HCCL, and Marker for Tracing and Profiling. |
| **C Callback API** | Subscribes to Runtime/HCCL callbacks and runs custom logic before and after API calls. |
| **Python API** | Provides high-level interfaces such as `KernelMonitor`, `HcclMonitor`, `MstxMonitor`, and `CommunicationMonitor`. |

For detailed interface definitions, see [C API Reference](../api_reference/c_api/README.md) and [Python API Reference](../api_reference/python_api/README.md).

### Q15: What Samples Are Available?

| Sample | Interface Type | Applicable Scenario |
| --- | --- | --- |
| `callback_domain` | Callback API | Runtime API callback interception and pre- and post-processing |
| `callback_mstx` | Callback API + MSTX | Launch Kernel marker collection and context pass-through |
| `mspti_activity` | Activity API | Basic activity data collection and buffer processing |
| `mspti_correlation` | Activity API | Correlation analysis between API dispatch and Kernel execution |
| `mspti_external_correlation` | Activity API | Correlation analysis of cross-layer call chains |
| `mspti_hccl_activity` | Activity API | HCCL communication behavior collection and analysis |
| `mspti_mstx_activity_domain` | Activity API + MSTX | Domain-level marker collection control |
| `python_monitor` | Python API | Python computation and communication time collection |
| `python_mstx_monitor` | Python API + MSTX | Python custom marker analysis |

### Q16: What Additional Installation Do the Python Samples Require?

The Python samples (`python_monitor` and `python_mstx_monitor`) additionally depend on the PyTorch framework and the TorchNPU plugin. See [TorchNPU Software Installation](https://gitcode.com/Ascend/pytorch/tree/master/docs/en/installation_guide/building_from_source.md).

### Q17: What Is the Activity Buffer and How Do I Manage It?

The Activity Buffer is a memory buffer that msPTI uses to cache Activity Record data. You need to register the `RequestFunc` and `CompleteFunc` callback functions:

1. msPTI requests an empty buffer through `RequestFunc`.
2. When the buffer is full, msPTI returns the full buffer through the `CompleteFunc` callback.
3. You iterate over the records through `msptiActivityGetNextRecord`.
4. After consuming the records, you return the empty buffer to msPTI through `RequestFunc`.

### Q18: What Is the Purpose of `correlationId`?

The `correlationId` is used to associate CANN API calls with the Activity records they trigger, such as Kernel execution and memory operations. Through this field, you can establish a one-to-one correspondence between API dispatch and actual hardware execution, making it easier to analyze performance bottlenecks.

### Q19: What Is the Purpose of External Correlation IDs?

External correlation IDs implement a push/pop mechanism through `msptiActivityPushExternalCorrelationId` and `msptiActivityPopExternalCorrelationId`, allowing API calls at different levels to be associated with each other, making it easier to trace the complete function call stack.

### Q20: How Do I Control the Collection Scope of Markers?

Use `msptiActivityEnableMarkerDomain`/`msptiActivityDisableMarkerDomain` (C API) or `MstxMonitor.enable_domain`/`MstxMonitor.disable_domain` (Python API) to dynamically start or stop marker collection by domain, thereby controlling the collection scope and performance overhead.

---

## Common Errors and Troubleshooting

### Q21: What If `pip show mspti` Reports That the Command Does Not Exist?

Verify that the current terminal uses the Python environment in which msPTI is installed. You can confirm the Python path by running the `which python` command, and then switch to the correct virtual environment.

### Q22: What If a Sample Fails to Run Because `set_env.sh` Cannot Be Found?

Verify that `${install_path}` is replaced with the actual CANN installation path, for example, `/usr/local/Ascend/cann`. If you are not sure about the installation path, find it by running `find / -name "set_env.sh" 2>/dev/null`.

### Q23: What If `ms_install.py` Download Fails or an SSL Certificate Error Occurs?

See [FAQ](https://www.hiascend.com/developer/blog/details/02176213671719317003). If the environment does not allow network access, download the script on a device that has network access first, and then copy it to the target device.

### Q24: What If the CMake Version Is Too Low When Compiling the Source Code?

msPTI requires CMake 3.14 or later. Upgrade CMake and try again.

### Q25: What If No Data or Incomplete Data Is Collected When Running Samples?

1. Verify that msPTI is correctly installed: `pip show mspti`
2. Verify that the CANN environment variables are correctly configured: `source ${install_path}/set_env.sh`
3. Verify that no other performance collection tools are running at the same time.
4. Verify that the Ascend product you use is on the supported products list.
5. Check that `msptiActivityEnable` is correctly called to enable the Activity Kind of the corresponding type.
