# msPTI Tool Installation Guide

The msPTI tool can be installed in the following ways:

- CANN Package Installation: The full functionality of the msPTI tool is integrated and released with the CANN package. For details, see the [CANN Quick Installation](https://www.hiascend.com/cann/download).
- [RUN Package Installation](#run-package-installation): The full functionality of the msPTI tool is integrated in the CANN package and msPTI depends on the CANN package. You need to **install the CANN package first** before using the msPTI tool. However, you can use the .run package to upgrade and install the latest features from the tool's code repository. It overwrites and installs the msPTI package in an environment where the CANN package is already installed.

## Run Package Installation

If you need to use the features of the latest code, you can download the code from this repository, compile, package, and complete the installation yourself.

### Obtaining the Run Package

Two methods are supported for obtaining the run package:

- Method 1: Download the run package from the releases page.
- Method 2: Build the run package from source code.

#### Method 1: Download from the Releases Page

Run package release address: [msPTI releases](https://gitcode.com/Ascend/mspti/releases)

After downloading, you are advised to perform an integrity check (MD5) before installation. Example:

```shell
wget https://gitcode.com/Ascend/mspti/releases/download/<tag>/mindstudio-profiler-tools-interface_<version>_<arch>.run
md5sum mindstudio-profiler-tools-interface_<version>_<arch>.run
echo "<expected_md5> mindstudio-profiler-tools-interface_<version>_<arch>.run" | md5sum -c -
```

- `<expected_md5>` should be the MD5 value corresponding to the installation package of the same version on the release page.
- For the MD5 list of installation packages for each version, see [Release Notes](../release_notes.md).

**Handling MD5sum Checksum Mismatch**

- If `md5sum -c -` outputs `FAILED`, do not proceed with the installation.
- Delete the current file and download it again, then perform the MD5 checksum verification again.
- If the verification still fails, check whether the file name and version on the releases page are consistent, and report the issue through Issues.

#### Method 2: Compiling from Source

Run the following command to compile the run package:

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
bash scripts/build.sh [<version>]
```

After the compilation is complete, the .run package of the msPTI tool is generated in the **mspti/output** directory. The .run package name format is `mindstudio-profiler-tools-interface_<version>_<arch>.run`.

The version parameter in the preceding compilation command is the version in the software package name, indicating the version number of the .run package.

The arch in the .run package indicates the system architecture, which is automatically adapted based on the actual running system.

### Installing the Run Package

1. Add the executable permission to the .run package.

    ```shell
    chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
    ```

2. Install the .run package.

    ```shell
    ./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
    ```

    The installation command supports parameters such as `--install-path=<path>`. For details about how to use them, see [Parameter Description](#parameter-description).

    When you run the installation command, the `--check` parameter is automatically executed to verify the consistency and integrity of the software package. The following output indicates that the software package verification is successful.

    ```text
    Verifying archive integrity...  100%   SHA256 checksums are OK. All good.
    ```

    After the installation is complete, the following information indicates that the software is installed successfully:

    ```text
    MindStudio-Profiler-Tools-Interface package install success.
    ```

## Appendix

### Parameter Description

The command for installing the msPTI tool .run package can be configured with the following parameters:

| Parameter     | Optional/Mandatory | Description                                                                                                                                                                               |
| --------| -------  |----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| --install | Optional | Installs the software package. You can configure the `--install-path` parameter to specify the installation path of the software. If the `--install-path` parameter is not configured, the software is installed to the default path.                                                                                                             |
| --uninstall | Optional | Uninstalls the software package. You can configure the `--install-path` parameter to specify the path where the software was installed. If the `--install-path` parameter is not configured, mspti in the default path is uninstalled.|
| `--install-path` | Optional | Installation path, which must be specified to the CANN layer directory, for example, `/usr/local/Ascend/cann-9.0.0`. If the user does not specify an installation path, the software will be installed to the default path. The default installation paths are as follows:<br>&#8226; root user: `/usr/local/Ascend/cann`.<br>&#8226; Non-root user: `${HOME}/Ascend/cann`, where `${HOME}` is the home directory of the current user. |
| --install-for-all | Optional | During installation, allows other users to have the permissions of the installation user group. When the installation is performed with this parameter, other users are supported to use msPTI to run services. However, this parameter poses a security risk, so use it with caution.                                                                                                               |

Other parameters can also be specified for installing the run package. For details, see the `./xxx.run --help` command.
