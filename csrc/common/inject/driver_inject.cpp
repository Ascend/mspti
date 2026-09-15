/* -------------------------------------------------------------------------
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This file is part of the MindStudio project.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *    http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
#include "csrc/common/inject/driver_inject.h"

#include <pthread.h>

#include <functional>

#include "csrc/common/function_loader.h"
#include "csrc/common/utils.h"

namespace
{
enum DriverFunctionIndex
{
    FUNC_PROF_DRV_GET_CHANNELS,
    FUNC_DRV_GET_DEV_IDS,
    FUNC_DRV_GET_DEV_NUM,
    FUNC_PROF_DRV_START,
    FUNC_PROF_STOP,
    FUNC_PROF_CHANNEL_READ,
    FUNC_PROF_CHANNEL_POLL,
    FUNC_HAL_GET_DEVICE_INFO,
    FUNC_HAL_GET_API_VERSION,
    FUNC_HAL_PROF_DATA_FLUSH,
    FUNC_HAL_ESCHED_ATTACH_DEVICE,
    FUNC_HAL_ESCHED_DETTACH_DEVICE,
    FUNC_HAL_ESCHED_CREATE_GRP_EX,
    FUNC_HAL_ESCHED_SUBSCRIBE_EVENT,
    FUNC_HAL_ESCHED_WAIT_EVENT,
    FUNC_HAL_ESCHED_QUERY_INFO,
    FUNC_HAL_QUERY_DEVPID,
    FUNC_DRIVER_COUNT
};

pthread_once_t g_once = PTHREAD_ONCE_INIT;
void *g_driverFuncArray[FUNC_DRIVER_COUNT];

void LoadDriverFunction()
{
    g_driverFuncArray[FUNC_PROF_DRV_GET_CHANNELS] =
        Mspti::Common::RegisterFunction("libascend_hal", "prof_drv_get_channels");
    g_driverFuncArray[FUNC_DRV_GET_DEV_IDS] = Mspti::Common::RegisterFunction("libascend_hal", "drvGetDevIDs");
    g_driverFuncArray[FUNC_DRV_GET_DEV_NUM] = Mspti::Common::RegisterFunction("libascend_hal", "drvGetDevNum");
    g_driverFuncArray[FUNC_PROF_DRV_START] = Mspti::Common::RegisterFunction("libascend_hal", "prof_drv_start");
    g_driverFuncArray[FUNC_PROF_STOP] = Mspti::Common::RegisterFunction("libascend_hal", "prof_stop");
    g_driverFuncArray[FUNC_PROF_CHANNEL_READ] = Mspti::Common::RegisterFunction("libascend_hal", "prof_channel_read");
    g_driverFuncArray[FUNC_PROF_CHANNEL_POLL] = Mspti::Common::RegisterFunction("libascend_hal", "prof_channel_poll");
    g_driverFuncArray[FUNC_HAL_GET_DEVICE_INFO] = Mspti::Common::RegisterFunction("libascend_hal", "halGetDeviceInfo");
    g_driverFuncArray[FUNC_HAL_GET_API_VERSION] = Mspti::Common::RegisterFunction("libascend_hal", "halGetAPIVersion");
    g_driverFuncArray[FUNC_HAL_PROF_DATA_FLUSH] = Mspti::Common::RegisterFunction("libascend_hal", "halProfDataFlush");
    g_driverFuncArray[FUNC_HAL_ESCHED_ATTACH_DEVICE] =
        Mspti::Common::RegisterFunction("libascend_hal", "halEschedAttachDevice");
    g_driverFuncArray[FUNC_HAL_ESCHED_DETTACH_DEVICE] =
        Mspti::Common::RegisterFunction("libascend_hal", "halEschedDettachDevice");
    g_driverFuncArray[FUNC_HAL_ESCHED_CREATE_GRP_EX] =
        Mspti::Common::RegisterFunction("libascend_hal", "halEschedCreateGrpEx");
    g_driverFuncArray[FUNC_HAL_ESCHED_SUBSCRIBE_EVENT] =
        Mspti::Common::RegisterFunction("libascend_hal", "halEschedSubscribeEvent");
    g_driverFuncArray[FUNC_HAL_ESCHED_WAIT_EVENT] =
        Mspti::Common::RegisterFunction("libascend_hal", "halEschedWaitEvent");
    g_driverFuncArray[FUNC_HAL_ESCHED_QUERY_INFO] =
        Mspti::Common::RegisterFunction("libascend_hal", "halEschedQueryInfo");
    g_driverFuncArray[FUNC_HAL_QUERY_DEVPID] = Mspti::Common::RegisterFunction("libascend_hal", "halQueryDevpid");
}
}  // namespace

int ProfDrvGetChannels(unsigned int deviceId, ChannelListT *channelList)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_PROF_DRV_GET_CHANNELS];
    using ProfDrvGetChannelsFunc = std::function<decltype(ProfDrvGetChannels)>;
    ProfDrvGetChannelsFunc func = Mspti::Common::ReinterpretConvert<decltype(&ProfDrvGetChannels)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "prof_drv_get_channels", func);
    }
    THROW_FUNC_NOTFOUND(func, "prof_drv_get_channels", "libascend_hal.so");
    return func(deviceId, channelList);
}

DrvError DrvGetDevIDs(uint32_t *devices, uint32_t len)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_DRV_GET_DEV_IDS];
    using DrvGetDevIDsFunc = std::function<decltype(DrvGetDevIDs)>;
    DrvGetDevIDsFunc func = Mspti::Common::ReinterpretConvert<decltype(&DrvGetDevIDs)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "drvGetDevIDs", func);
    }
    THROW_FUNC_NOTFOUND(func, "drvGetDevIDs", "libascend_hal.so");
    return func(devices, len);
}

DrvError DrvGetDevNum(uint32_t *count)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_DRV_GET_DEV_NUM];
    using DrvGetDevNumFunc = std::function<decltype(DrvGetDevNum)>;
    DrvGetDevNumFunc func = Mspti::Common::ReinterpretConvert<decltype(&DrvGetDevNum)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "drvGetDevNum", func);
    }
    THROW_FUNC_NOTFOUND(func, "drvGetDevNum", "libascend_hal.so");
    return func(count);
}

int ProfDrvStart(unsigned int deviceId, unsigned int channelId, struct ProfStartPara *startPara)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_PROF_DRV_START];
    using ProfDrvStartFunc = std::function<decltype(ProfDrvStart)>;
    ProfDrvStartFunc func = Mspti::Common::ReinterpretConvert<decltype(&ProfDrvStart)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "prof_drv_start", func);
    }
    THROW_FUNC_NOTFOUND(func, "prof_drv_start", "libascend_hal.so");
    return func(deviceId, channelId, startPara);
}

int ProfStop(unsigned int deviceId, unsigned int channelId)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_PROF_STOP];
    using ProfStopFunc = std::function<decltype(ProfStop)>;
    ProfStopFunc func = Mspti::Common::ReinterpretConvert<decltype(&ProfStop)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "prof_stop", func);
    }
    THROW_FUNC_NOTFOUND(func, "prof_stop", "libascend_hal.so");
    return func(deviceId, channelId);
}

int ProfChannelRead(unsigned int deviceId, unsigned int channelId, char *outBuf, unsigned int bufSize)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_PROF_CHANNEL_READ];
    using ProfChannelReadFunc = std::function<decltype(ProfChannelRead)>;
    ProfChannelReadFunc func = Mspti::Common::ReinterpretConvert<decltype(&ProfChannelRead)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "prof_channel_read", func);
    }
    THROW_FUNC_NOTFOUND(func, "prof_channel_read", "libascend_hal.so");
    return func(deviceId, channelId, outBuf, bufSize);
}

int ProfChannelPoll(struct ProfPollInfo *outBuf, int num, int timeout)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_PROF_CHANNEL_POLL];
    using ProfChannelPollFunc = std::function<decltype(ProfChannelPoll)>;
    ProfChannelPollFunc func = Mspti::Common::ReinterpretConvert<decltype(&ProfChannelPoll)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "prof_channel_poll", func);
    }
    THROW_FUNC_NOTFOUND(func, "prof_channel_poll", "libascend_hal.so");
    return func(outBuf, num, timeout);
}

DrvError HalGetDeviceInfo(uint32_t deviceId, int32_t moduleType, int32_t infoType, int64_t *value)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_GET_DEVICE_INFO];
    using HalGetDeviceInfoFunc = std::function<decltype(HalGetDeviceInfo)>;
    HalGetDeviceInfoFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalGetDeviceInfo)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halGetDeviceInfo", func);
    }
    THROW_FUNC_NOTFOUND(func, "halGetDeviceInfo", "libascend_hal.so");
    return func(deviceId, moduleType, infoType, value);
}

DrvError halGetAPIVersion(int32_t *apiVersion)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_GET_API_VERSION];
    using halGetAPIVersionFunc = std::function<decltype(halGetAPIVersion)>;
    halGetAPIVersionFunc func = Mspti::Common::ReinterpretConvert<decltype(&halGetAPIVersion)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", __FUNCTION__, func);
    }
    THROW_FUNC_NOTFOUND(func, __FUNCTION__, "libascend_hal.so");
    return func(apiVersion);
}

int HalProfDataFlush(unsigned int device_id, unsigned int channel_id, unsigned int *data_len)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_PROF_DATA_FLUSH];
    using HalProfDataFlushFunc = std::function<decltype(HalProfDataFlush)>;
    HalProfDataFlushFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalProfDataFlush)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halProfDataFlush", func);
    }
    THROW_FUNC_NOTFOUND(func, "halProfDataFlush", "libascend_hal.so");
    return func(device_id, channel_id, data_len);
}

int HalEschedAttachDevice(unsigned int devId)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_ESCHED_ATTACH_DEVICE];
    using HalEschedAttachDeviceFunc = std::function<decltype(HalEschedAttachDevice)>;
    HalEschedAttachDeviceFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalEschedAttachDevice)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halEschedAttachDevice", func);
    }
    THROW_FUNC_NOTFOUND(func, "halEschedAttachDevice", "libascend_hal.so");
    return func(devId);
}

int HalEschedDettachDevice(unsigned int devId)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_ESCHED_DETTACH_DEVICE];
    using HalEschedDettachDeviceFunc = std::function<decltype(HalEschedDettachDevice)>;
    HalEschedDettachDeviceFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalEschedDettachDevice)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halEschedDettachDevice", func);
    }
    THROW_FUNC_NOTFOUND(func, "halEschedDettachDevice", "libascend_hal.so");
    return func(devId);
}

int HalEschedCreateGrpEx(unsigned int devId, MsptiEschedGrpParaT *grpPara, unsigned int *grpId)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_ESCHED_CREATE_GRP_EX];
    using HalEschedCreateGrpExFunc = std::function<decltype(HalEschedCreateGrpEx)>;
    HalEschedCreateGrpExFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalEschedCreateGrpEx)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halEschedCreateGrpEx", func);
    }
    THROW_FUNC_NOTFOUND(func, "halEschedCreateGrpEx", "libascend_hal.so");
    return func(devId, grpPara, grpId);
}

int HalEschedSubscribeEvent(unsigned int devId, unsigned int grpId, unsigned int threadId,
                            unsigned long long eventBitmap)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_ESCHED_SUBSCRIBE_EVENT];
    using HalEschedSubscribeEventFunc = std::function<decltype(HalEschedSubscribeEvent)>;
    HalEschedSubscribeEventFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalEschedSubscribeEvent)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halEschedSubscribeEvent", func);
    }
    THROW_FUNC_NOTFOUND(func, "halEschedSubscribeEvent", "libascend_hal.so");
    return func(devId, grpId, threadId, eventBitmap);
}

int HalEschedWaitEvent(unsigned int devId, unsigned int grpId, unsigned int threadId, int timeout,
                       MsptiEventInfoT *event)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_ESCHED_WAIT_EVENT];
    using HalEschedWaitEventFunc = std::function<decltype(HalEschedWaitEvent)>;
    HalEschedWaitEventFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalEschedWaitEvent)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halEschedWaitEvent", func);
    }
    THROW_FUNC_NOTFOUND(func, "halEschedWaitEvent", "libascend_hal.so");
    return func(devId, grpId, threadId, timeout, event);
}

int HalEschedQueryInfo(unsigned int devId, int type, MsptiEschedInputInfoT *inPut, MsptiEschedOutputInfoT *outPut)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_ESCHED_QUERY_INFO];
    using HalEschedQueryInfoFunc = std::function<decltype(HalEschedQueryInfo)>;
    HalEschedQueryInfoFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalEschedQueryInfo)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halEschedQueryInfo", func);
    }
    THROW_FUNC_NOTFOUND(func, "halEschedQueryInfo", "libascend_hal.so");
    return func(devId, type, inPut, outPut);
}

int HalQueryDevpid(MsptiHalQueryDevpidInfoT info, int *devPid)
{
    pthread_once(&g_once, LoadDriverFunction);
    void *voidFunc = g_driverFuncArray[FUNC_HAL_QUERY_DEVPID];
    using HalQueryDevpidFunc = std::function<decltype(HalQueryDevpid)>;
    HalQueryDevpidFunc func = Mspti::Common::ReinterpretConvert<decltype(&HalQueryDevpid)>(voidFunc);
    if (func == nullptr)
    {
        Mspti::Common::GetFunction("libascend_hal", "halQueryDevpid", func);
    }
    THROW_FUNC_NOTFOUND(func, "halQueryDevpid", "libascend_hal.so");
    return func(info, devPid);
}
