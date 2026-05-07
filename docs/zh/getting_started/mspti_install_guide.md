# msPTI工具安装指南

## 安装说明

msPTI工具的安装方式包括：

- 使用CANN包安装：msPTI工具完整功能已集成在CANN包中，请参考《[CANN 快速安装](https://www.hiascend.com/cann/download)》安装昇腾NPU驱动和CANN软件（包含Toolkit和ops包）并配置环境变量。
- [使用run包安装](#使用run包安装)：msPTI工具完整功能集成在CANN包中且msPTI依赖CANN包，因此使用msPTI工具需要**先完成CANN包的安装**，若需要升级安装本工具代码仓中的最新功能，可以使用run包安装，在已安装CANN包的环境下覆盖安装msPTI包。

## 使用run包安装

如需使用最新代码的功能，可下载本仓库代码，自行编译run包并完成安装。

### 获取run包

支持两种方式获取run包：

- 方式一：从Release页面下载
- 方式二：源码编译

#### 方式一：Release页面下载

1. 请参考[msPTI Release](https://gitcode.com/Ascend/mspti/releases)下载msPTI的run包和对应数字签名文件（.sha256）。

   下载本软件即表示您同意《[华为企业业务最终用户许可协议（EULA）](https://e.huawei.com/cn/about/eula)》的条款和条件。

2. 验证run包的完整性。

   1. 在run包所在目录执行如下命令获取run包的sha256校验码。

      ```bash
      sha256sum {name}.run
      ```

      打印如下示例信息。

      ```ColdFusion
      {sha256} {name}.run
      ```

   2. 用记事本打开数字签名文件查看sha256校验码。

   3. 比对两个文件的sha256校验码是否一致。

      若两个校验码一致，则表示下载了正确的软件包；若不一致，请不要使用该软件包，需要支持与服务请在论坛求助或提交技术工单。

#### 方式二：源码编译

执行如下命令编译run包：

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
bash scripts/build.sh [{version}]
```

- 支持通过环境变量指定版本号（优先级最高）：`BUILD_VERSION`用于设置run包版本，`WHL_VERSION`用于设置whl包版本。
- 支持通过命令行参数指定版本号（优先级低于环境变量），默认版本号为`version.info`中的`Version`字段。
- run包中的arch表示系统架构，根据实际运行系统自动适配。
- 编译完成后，会在mspti/output目录下生成msPTI工具的run包，run包名称格式为`mindstudio-profiler-tools-interface_{version}_{arch}.run`。

### 安装run包

1. 增加对run包的可执行权限。

    ```shell
    chmod +x mindstudio-profiler-tools-interface_{version}_{arch}.run
    ```

2. 安装run包。

    ```shell
    ./mindstudio-profiler-tools-interface_{version}_{arch}.run --install
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

## 升级

msPTI工具升级可参照[使用run包安装](#使用run包安装)中的步骤直接安装msPTI最新的run包即可，新的run包会自动覆盖原有的run包。

## 卸载

卸载msPTI工具有如下两种方式：

方式一：通过--uninstall参数单独卸载

```bash
./mindstudio-profiler-tools-interface_{version}_{arch}.run --uninstall --install-path=<mspti_install_path>
```

> [!NOTE]
>
> 需要在run包所在路径执行该命令，其中`mspti_install_path`为该run包安装路径。

方式二：卸载CANN包

msPTI默认会安装在CANN包的安装路径下，直接卸载CANN包的时候会将msPTI一起卸载。

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
