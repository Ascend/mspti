# 快速入门

建议按照“安装工具 > 配置环境 > 运行样例”的顺序完成快速体验：

1. 完成工具安装。

   请先参见 《[msPTI 工具安装指南](https://gitcode.com/Ascend/mspti/blob/26.0.0/docs/zh/mspti_install_guide.md)》完成msPTI工具安装。

2. 配置CANN环境变量。

   ```bash
   source ${install_path}/set_env.sh
   ```

   其中`${install_path}`需要替换为CANN的安装路径，例如`/usr/local/Ascend/cann`。

3. 进入样例目录并执行脚本。

   ```bash
   cd ${install_path}/tools/mspti/samples/callback_domain
   bash sample_run.sh
   ```

   `samples`目录下各样例的适用场景、能力说明与补充说明，请参见 《[msPTI样例集](https://gitcode.com/Ascend/mspti/blob/26.0.0/docs/zh/README.md#mspti%E6%A0%B7%E4%BE%8B%E9%9B%86)》。
