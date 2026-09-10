# msPTI Installation Guide

## 1. Installation Description

This tool is integrated into CANN. If CANN is installed and you do not need to update this tool, you can use it directly without performing the installation described in this document.

If CANN is not installed in your environment, install the Ascend NPU driver and CANN software (including the Toolkit and ops packages) by following the [CANN Quick Installation](https://www.hiascend.com/en/cann/download) guide, and configure the environment variables.

If you need to upgrade this tool separately or use the latest version, you can install it in any of the following ways: [online installation](#21-online-installation), [offline installation](#22-offline-installation), or [source code installation](#23-source-code-installation).

## 2. Installation Methods

### 2.1 Online Installation

If your device has Internet access, you can automatically download and install the tool with a single command. See the MindStudio [Download](https://www.hiascend.com/en/developer/software/mindstudio/download) page on the Ascend community, select the corresponding CANN version, choose "Online Installation" as the installation mode, and the system will guide you through the rest of the process.

### 2.2 Offline Installation

For devices in an environment without Internet access, such as an enterprise intranet, download the complete offline installation package on a machine with Internet access first, and then transfer it to the target device for installation. See the MindStudio [Download](https://www.hiascend.com/en/developer/software/mindstudio/download) page on the Ascend community, select the corresponding CANN version, choose "Offline Installation" as the installation mode, and obtain the corresponding installation package and operation guide.

### 2.3 Source Code Installation

To use the features of the latest code, download the code of this repository, compile the .run package by yourself, and complete the installation.

#### 2.3.1 Building and Packaging

Run the following command to compile the .run package:

```bash
git clone https://gitcode.com/Ascend/mspti.git -b master
cd mspti
bash scripts/build.sh [{version}]
```

- You can specify the version number through environment variables (with the highest priority): `BUILD_VERSION` sets the version of the .run package, and `WHL_VERSION` sets the version of the whl package.
- You can also specify the version number through CLI parameters (with lower priority than environment variables). The default version number is the `Version` field in `version.info`.
- `arch` in the .run package indicates the system architecture, which is automatically determined based on the host system.
- After the compilation is complete, the .run package of the msPTI tool is generated in the mspti/output directory. The .run package is named in the format of `mindstudio-profiler-tools-interface_{version}_{arch}.run`.

#### 2.3.2 Installing the .run Package

1. Grant the execute permission on the .run package.

    ```shell
    chmod +x mindstudio-profiler-tools-interface_{version}_{arch}.run
    ```

2. Install the .run package.

    ```shell
    ./mindstudio-profiler-tools-interface_{version}_{arch}.run --install
    ```

    The installation command supports parameters such as `--install-path=<path>`. For details about how to use these parameters, see [Parameters](#61-parameters).

    When the installation command is executed, the `--check` option is automatically executed to check the consistency and integrity of the software package. If the following information is displayed, the software package is verified successfully.

    ```text
    Verifying archive integrity...  100%   SHA256 checksums are OK. All good.
    ```

    If the following information is displayed, the software is successfully installed:

    ```text
    MindStudio-Profiler-Tools-Interface package install success.
    ```

## 3. Installation Verification

After the installation is complete, run the following command to verify that the tool is installed successfully:

```bash
pip show mspti
```

If the command output contains no errors and displays the tool information, the tool is installed successfully.

If `pip show mspti` reports that the command does not exist, check that the current terminal uses the Python environment where `msPTI` is installed.

## 4. Uninstallation

You can uninstall the tool by performing the following steps:

1. Download the script.

   ```bash
   curl -O https://inst.obs.cn-north-4.myhuaweicloud.com/26.1.0/ms_install.py
   ```

   > [!NOTE]
   >
   > - A network connection is required for the download. If the environment does not allow network access or is offline, download the script in a networked environment first, and then copy it to the target device.
   > - If the command does not respond or issues such as connection failures or SSL certificate errors occur, see [FAQ](https://www.hiascend.com/developer/blog/details/02176213671719317003).

2. Uninstall the tool.

   ```bash
   python ms_install.py uninstall {tools_name}
   ```

   In the command, `{tools_name}` is the name of the tool to uninstall. You can query the tool name by running the `python ms_install.py help` command. The tool name is displayed under the Available Tools field in the output.

   If the uninstallation is successful, the following information is displayed:

   ```ColdFusion
   Successfully uninstalled 1 tool ({tools_name})
   ```

## 5. Upgrade

An upgrade is performed by uninstalling first and then installing. If you run the installation command directly, the tool automatically uninstalls the old version and guides you through the overlay installation.

You can run the `pip show mspti` command to view the version information of the current environment, and then select the version to upgrade to. When upgrading the version, pay attention to the version compatibility. For details, see the [Release Notes](https://gitcode.com/Ascend/release-management/blob/master/MindStudio/master/release_notes_en.md).

## 6. Appendixes

### 6.1 Parameters

The following parameters can be configured for the installation command of the msPTI tool .run package:

| Parameter    | Mandatory (Yes/No)| Description                                                                                                                                                                              |
| --------| -------  |----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| --install | Optional| Installs the software package. You can use the `--install-path` option to specify the installation path of the software. If the `--install-path` option is not specified, the software is installed in the default path.                                                                                                            |
| --uninstall | Optional| Uninstalls the software package. You can specify the installation path of the software by setting the `--install-path` parameter. If the --install-path parameter is not set, mspti in the default path is directly uninstalled.|
| --install-path | Optional| Installation path, which must be specified to the CANN directory, for example, `/usr/local/Ascend/cann-9.0.0`. If the installation path is not specified, the software is installed in the default path. The default installation paths are as follows:<br>&#8226; For the root user: `/usr/local/Ascend/cann`.<br>&#8226; For a non-root user: `${HOME}/Ascend/cann`, where `${HOME}` indicates the home directory of the current user. |
| --install-for-all | Optional| Allows other users to have the permission of the installation user group during installation. When this parameter is used during installation, other users can use msPTI to run services. However, this parameter has security risks. Exercise caution when using this parameter.                                                                                                              |

You can also specify other options when installing the .run package. For details, run the `./xxx.run --help` command.
