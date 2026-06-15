# msPTI快速入门

建议按照“安装工具 > 配置环境 > 运行样例”的顺序完成快速体验：

1. 完成工具安装。
   请先参见 《[msPTI 工具安装指南](../getting_started/mspti_install_guide.md)》完成msPTI工具安装。

2. 配置CANN环境变量。

   ```bash
   source ${install_path}/set_env.sh
   ```

   其中`${install_path}`需要替换为CANN的安装路径，例如`/usr/local/Ascend/cann`。

3. 进入样例目录并执行脚本。

   ```bash
   cd ${install_path}/tools/mspti/samples/mspti_activity
   bash sample_run.sh
   ```

   看到如下信息，则表示运行成功：

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
   [RUNTIME_API] name: DevMalloc, start: 1775186328012443375, end: 1775186328012485525, processId: 3141177, threadId: 3141177, correlationId: 1
   [MEMORY] operationType: ALLOCATION, memoryKind: MEMORY_DEVICE, correlationId: 1, start: 1775186328012398215, end: 1775186328012526835, address: 20619064340480, bytes:32, processId: 3141177, deviceId: 0, streamId: 4294967295
   [RUNTIME_API] name: MemCopySync, start: 1775186328012545645, end: 1775186328012596965, processId: 3141177, threadId: 3141177, correlationId: 2
   [MEMCPY] copyKind: HTOD, bytes: 32, start: 1775186328012540625, end: 1775186328012599655, deviceId: 0, streamId: 4294967295, correlationId: 2, isAsync: 0
   ...
   ```

   `samples`目录下各样例的适用场景、能力说明与补充说明，请参见 《[msPTI样例集](../../../samples/README.md)》。
