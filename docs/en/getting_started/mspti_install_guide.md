# msPTI Installation Guide

<!-- md-trans-meta sourceCommit=unknown translatedAt=2026-06-24T02:30:56.569Z pushedAt=2026-06-24T10:56:06.400Z -->

## 1. Installation Notes

This tool is integrated into CANN. If your environment does not have CANN installed, or you do not need to upgrade this tool within an existing CANN environment, it is recommended to install CANN directly. For details, see the [CANN Quick Installation](https://www.hiascend.com/en/cann/download) guide to install the Ascend NPU driver and CANN software (including Toolkit and ops packages), and configure the environment variables.

To install this tool or use the latest version, you can choose from the following three methods: [Online Installation](#21-online-installation), [Offline Installation](#22-offline-installation), [Source Installation](#23-source-installation).

## 2. Installation Methods

### 2.1 Online Installation

If your device has internet access, you can automatically download and install the tool with a single command. Visit the MindStudio [download](https://www.hiascend.com/en/developer/software/mindstudio/download) page on the Ascend Community, select the corresponding CANN version, choose online installation as the installation method, and the system will guide you through the subsequent steps.

### 2.2 Offline Installation

For devices in environments without external network access, such as enterprise intranets, first download the complete offline installation package on a machine with internet access, then transfer it to the target device for installation. Please visit the MindStudio [download](https://www.hiascend.com/en/developer/software/mindstudio/download) page on the Ascend Community, select the corresponding CANN version, choose offline installation as the installation method, and obtain the corresponding installation package and operation guide.

### 2.3 Source Installation

If you need to use the features of the latest code, you can download the code from this repository, compile the run package yourself, and complete the installation.

#### 2.3.1 Compilation and Packaging

Run the following command to compile the run package:

```bash
git clone https://gitcode.com/Ascend/mspti.git -b 26.0.0
cd mspti
bash scripts/build.sh [{version}]
```

- Supports specifying the version number via environment variables (highest priority): `BUILD_VERSION` is used to set the run package version, and `WHL_VERSION` is used to set the whl package version.
- Supports specifying the version number via command-line parameters (lower priority than environment variables). The default version number is the `Version` field in `version.info`.
- The `arch` in the run package represents the system architecture, which is automatically adapted based on the actual running system.
- After compilation is complete, the run package of the msPTI tool will be generated in the `mspti/output` directory. The run package name format is `mindstudio-profiler-tools-interface_{version}_{arch}.run`.

#### 2.3.2 Run Package Installation

1. Add executable permission to the run package.

    ```shell
    chmod +x mindstudio-profiler-tools-interface_{version}_{arch}.run
    ```

2. Install the run package.

    ```shell
    ./mindstudio-profiler-tools-interface_{version}_{arch}.run --install
    ```

    The installation command supports parameters such as `--install-path=<path>`. For details about how to use them, see [Parameter Description](#61-parameter-description).

    When you run the installation command, the --check parameter is automatically executed to verify the consistency and integrity of the software package. If the following information is displayed, the software package verification is successful.

    ```text
    Verifying archive integrity...  100%   SHA256 checksums are OK. All good.
    ```

    After the installation is complete, if the following information is displayed, the software has been successfully installed:

    ```text
    MindStudio-Profiler-Tools-Interface package install success.
    ```

## 3. Verifying the Installation

After the installation is complete, run the following command to verify whether the tool has been successfully installed:

```bash
pip show mspti
```

If the output shows no errors and displays the tool information, the installation is successful.

If `pip show mspti` prompts that the command does not exist, confirm that the current terminal is using the Python environment where `msPTI` is installed.

## 4. Uninstallation

You can uninstall it by following these steps:

1. Download the script.

   ```bash
   curl -O https://inst.obs.cn-north-4.myhuaweicloud.com/26.0.0/ms_install.py
   ```

   > [!NOTE]
   >
   > - An internet-connected environment is required for downloading. If the environment does not allow internet access or is offline, please download the script in an internet-connected environment first and then copy it to the target device.
   > - If the command execution is unresponsive or issues such as connection failure or SSL certificate errors occur, please refer to the [FAQ](https://www.hiascend.com/developer/blog/details/02176213671719317003).

2. Perform uninstallation.

   ```bash
   python ms_install.py uninstall {tools_name}
   ```

   Where `{tools_name}` is configured as the name of the tool to be uninstalled. You can query it using the `python ms_install.py help` command; the tool name is displayed under the Available Tools field in the printed information.

   Upon successful uninstallation, the following information is printed:

   ```ColdFusion
   Successfully uninstalled 1 tool ({tools_name})
   ```

## 5. Upgrade

Upgrading means "uninstall first, then install". Directly execute the installation command, and the tool will automatically uninstall the old version and guide you through the overwrite installation.

You can use the `pip show mspti` command to view the version information of the current environment, and then select the version to upgrade to. When upgrading the version, you need to pay attention to the version compatibility relationship. Please refer to the [Release Notes](https://gitcode.com/Ascend/release-management/blob/master/MindStudio/26.0.0/release_notes_en.md).

## 6. Appendix

### 6.1 Parameter Description

The installation command for the msPTI tool run package can be configured with the following parameters:

| Parameter | Optional/Required | Description |
| -------- | ------- | ----- |
| --install | Optional | Installs the software package. You can configure the --install-path parameter to specify the installation path of the software. If the --install-path parameter is not configured, the software is installed to the default path. |
| --uninstall | Optional | Uninstalls the software package. You can configure the --install-path parameter to specify the path where the software was installed. If the --install-path parameter is not configured, msPTI in the default path is uninstalled. |
| --install-path | Optional | Installation path, which must be specified to the CANN layer directory, for example, /usr/local/Ascend/cann-9.0.0. If the user does not specify an installation path, the software will be installed to the default path. The default installation paths are as follows:<br>&#8226; root user: "/usr/local/Ascend/cann".<br>&#8226; non-root user: "${HOME}/Ascend/cann", where ${HOME} is the home directory of the current user. |
| --install-for-all | Optional | During installation, allows other users to have the permissions of the installation user group. When the installation is performed with this parameter, other users are supported to use msPTI to run services. However, this parameter poses a security risk, so use it with caution. |

You can also specify other parameters when installing the run package. For details, run the `./xxx.run --help` command.
