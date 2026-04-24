# msPTI工具安装指南

msPTI工具的安装方式包括：

- 使用CANN包安装：msPTI工具完整功能已集成在CANN包中发布，可直接安装CANN包，具体请参见[CANN快速安装](https://www.hiascend.com/cann/download)。
- [使用run包安装](#使用run包安装)：msPTI工具完整功能集成在CANN包中且msPTI依赖CANN包，因此使用msPTI工具需要**先完成CANN包的安装**，若需要升级安装本工具代码仓中的最新功能，可以使用run包安装，在已安装CANN包的环境下覆盖安装msPTI包。

## 使用run包安装

如需使用最新代码的功能，可下载本仓库代码，自行编译run包并完成安装。

### 编译run包

执行如下命令编译run包：

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
bash scripts/build.sh [<version>]
```

- 支持通过环境变量指定版本号（优先级最高）：`BUILD_VERSION`用于设置run包版本，`WHL_VERSION`用于设置whl包版本。
- 支持通过命令行参数指定版本号（优先级低于环境变量），默认版本号为`version.info`中的`Version`字段。
- run包中的arch表示系统架构，根据实际运行系统自动适配。
- 编译完成后，会在mspti/output目录下生成msPTI工具的run包，run包名称格式为`mindstudio-profiler-tools-interface_<version>_<arch>.run`。

### 安装run包

1. 增加对run包的可执行权限。

    ```shell
    chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
    ```

2. 安装run包。

    ```shell
    ./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
    ```

    安装命令支持`--install-path=<path>`等参数，具体使用方式请参见[参数说明](#参数说明)。

    执行安装命令时，会自动执行--check参数，校验软件包的一致性和完整性，出现如下回显信息，表示软件包校验成功。

    ```text
    Verifying archive integrity...  100%   SHA256 checksums are OK. All good.
    ```

    安装完成后，若显示如下信息，则说明软件安装成功：

    ```text
    MindStudio-Profiler-Tools-Interface package install success.
    ```

## 附录

### 参数说明

msPTI工具run包的安装命令可配置如下参数：

| 参数     | 可选/必选 | 说明                                                                                                                                                                               |
| --------| -------  |----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| --install | 可选 | 安装软件包。可配置--install-path参数指定软件的安装路径；不配置--install-path参数时，则直接安装到默认路径下。                                                                                                             |
| --uninstall | 可选 | 卸载软件包。可配置--install-path参数指定软件安装时的路径；不配置--install-path参数时，则直接卸载默认路径下的mspti。|
| --install-path | 可选 | 安装路径，必须指定到CANN层目录，比如/usr/local/Ascend/cann-9.0.0。如果用户未指定安装路径，则软件会安装到默认路径下，默认安装路径如下：<br>&#8226; root用户：“/usr/local/Ascend/cann”。<br>&#8226; 非root用户：“${HOME}/Ascend/cann”，${HOME}为当前用户的家目录。 |
| --install-for-all | 可选 | 安装时，允许其他用户具有安装用户组的权限。当安装携带该参数时，支持其他用户使用msPTI运行业务，但该参数存在安全风险，请谨慎使用。                                                                                                               |

安装run包还可指定其他参数，具体可通过./xxx.run --help命令查看。
