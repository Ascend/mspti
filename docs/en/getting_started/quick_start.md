# msPTI Quick Start

You can follow the "Install > Configure > Run" workflow for a quick hands-on experience.

1. Complete the tool installation.
   For details, see the [msPTI Tool Installation Guide](mspti_install_guide.md) to complete the msPTI tool installation.

2. Configure CANN environment variables.

   ```bash
   source ${install_path}/set_env.sh
   ```

   Where `${install_path}` needs to be replaced with the CANN installation path, for example, `/usr/local/Ascend/cann`.

3. Go to the sample directory and run the script.

   ```bash
   cd ${install_path}/tools/mspti/samples/mspti_activity
   bash sample_run.sh
   ```

   If the following information is displayed, the execution is successful:
  
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

   For details about the applicable scenarios, capability descriptions, and supplementary notes of each sample in the `samples` directory, see [msPTI Samples](./samples_guide.md#mspti-samples).
