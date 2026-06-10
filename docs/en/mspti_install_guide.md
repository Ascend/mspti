# msPTI Installation Guide

## Installation Description

This document describes how to install the msPTI tool.

## Prepare for installation.

- For hardware environment requirements, see [Ascend Product Models](<>).

- For details about the software environment, see the CANN Toolkit and OPS operator package installation guide of the corresponding version in the CANN Software Installation Guide (https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/softwareinst/instg/instg_quick.html?Mode=PmIns&InstallType=local&OS=openEuler&Software=cannToolKit), and configure the CANN environment variables.

## Installing the .run Package

You can obtain the.runfile in either of the following ways:
- Method 1: Download the.runfile from the releases page.
- Method 2: Build the.runfile from the source code.

### Method 1: Downloading from the releases page
The software installation package is released at:
- [msPTI releases](https://gitcode.com/Ascend/mspti/releases)

After the download, you are advised to verify the integrity (MD5) of the package before installing it. Example:
```shell
wget https://gitcode.com/Ascend/mspti/releases/download/<tag>/mindstudio-profiler-tools-interface_<version>_<arch>.run
md5sum mindstudio-profiler-tools-interface_<version>_<arch>.run
echo "<expected_md5> mindstudio-profiler-tools-interface_<version>_<arch>.run" | md5sum -c -
```
**NOTE**
- For `<expected_md5>`, use the MD5 value corresponding to the installation package of the same version on the release page.
- For details about the MD5 list of installation packages of each version, see the Release Notes (./release_notes.md).

**Suggestions for MD5 checksum inconsistency:**
- If the `FAILED` is displayed in the `md5sum -c -` output, do not continue the installation.
- Delete the current file, download it again, and perform the MD5 verification again.
- If the verification still fails, check whether the file name and version are consistent on the releases page and report the problem using Issues.

### Method 2: Compiling the Source Code

Run the following command to compile the.runfile:

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
bash scripts/build.sh [<version>]
```

After the compilation is complete, the.runfile of the msPTI tool is generated in the mspti/output directory. The.runfile is named in the format of `mindstudio-profiler-tools-interface_<version>_<arch>.run`.
The version parameter in the preceding compilation command indicates the version number of the.runfile, which is the same as that in the software package name.
`arch` in the .run package indicates the system architecture, which is automatically determined based on the host system.

### Installation Procedure

1. Grant the execute permission on the.runfile.

    ```shell
    chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
    ```

2. Install the .run package.

    ```shell
    ./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
    ```

    The installation command supports parameters such as `--install-path=<path>`. For details, see the parameter description in the following section.

    When the installation command is executed, the `--check` option is automatically executed to check the consistency and integrity of the software package. If the following information is displayed, the software package is verified successfully.

    ```text
    Verifying archive integrity...  100%   SHA256 checksums are OK. All good.
    ```

    If the following information is displayed, the software is successfully installed:

    ```text
    MindStudio-Profiler-Tools-Interface package install success.
    ```

## Appendixes

### Parameters

The following parameters can be configured for the installation command of the msPTI tool runfile:

| Parameter    | Mandatory (Yes/No)| Description                                                                                                                                                                              |
| --------| -------  |----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| --install | Optional| Installs the software package. You can use the `--install-path` option to specify the installation path of the software. If the `--install-path` option is not specified, the software is installed in the default path.                                                                                                            |
| --uninstall | Optional| Uninstalls the software package. You can specify the installation path of the software by setting the --install-path parameter. If the --install-path parameter is not set, the mspti in the default path is directly uninstalled.|
| --install-path | Optional| Installation path, which must be specified to the CANN directory, for example, /usr/local/Ascend/cann-9.0.0. If the installation path is not specified, the software is installed in the default path. The default installation paths are as follows:<br> - For the root user: /usr/local/Ascend/cann.<br>- For a non-root user: \${HOME}/Ascend/cann, where ${HOME} indicates the home directory of the current user.|
| --install-for-all | Optional| Allows other users to have the permission of the installation user group during installation. When this parameter is used during installation, other users can use msPTI to run services. However, this parameter has security risks. Exercise caution when using this parameter.                                                                                                              |

You can also specify other options when installing the .run package. For details, run the `./xxx.run --help` command.
