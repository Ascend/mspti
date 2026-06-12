file(GLOB_RECURSE COMMON_FILE
    ${TOP_DIR}/csrc/common/*.cpp
)

file(GLOB_RECURSE REMOVE_SOURCES
    ${TOP_DIR}/csrc/common/inject/*.cpp
)

list(REMOVE_ITEM COMMON_FILE ${REMOVE_SOURCES})

set(SRC_FILE
    ${COMMON_FILE}
    ${TOP_DIR}/csrc/activity/ascend/channel/channel_reader.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/parser_manager.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/cann_hash_cache.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/device_task_calculator.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/communication_calculator.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/cann_track_cache.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/mstx_parser.cpp
    ${TOP_DIR}/csrc/activity/ascend/reporter/external_correlation_reporter.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/hccl_calculator.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/hccl_reporter.cpp
    ${TOP_DIR}/csrc/activity/activity_manager.cpp
    ${TOP_DIR}/csrc/activity/ascend/dev_task_manager.cpp
    ${TOP_DIR}/csrc/activity/ascend/dev_prof_task.cpp
    ${TOP_DIR}/csrc/activity/ascend/channel/channel_pool_manager.cpp
    ${TOP_DIR}/csrc/activity/ascend/channel/channel_pool.cpp
    ${TOP_DIR}/csrc/activity/ascend/parser/kernel_parser.cpp
    ${TOP_DIR}/csrc/activity/ascend/channel/soclog_convert.cpp
    ${TOP_DIR}/csrc/activity/ascend/channel/tsfw_convert.cpp
    ${TOP_DIR}/csrc/activity/ascend/channel/stars_common.cpp
)

add_executable(mspti_channel_utest
    ${SRC_FILE}
    main.cpp
    channel_reader_utest.cpp
    channel_pool_utest.cpp
    channel_pool_manager_utest.cpp
    convert_utest.cpp
    ${TOP_DIR}/csrc/common/inject/mstx_inject.cpp
    ${TOP_DIR}/csrc/common/function_loader.cpp
    ${UT_TOP_DIR}/stub/inject/profapi_inject_stub.cpp
    ${UT_TOP_DIR}/stub/inject/driver_inject_stub.cpp
    ${UT_TOP_DIR}/stub/inject/acl_inject_stub.cpp
    ${UT_TOP_DIR}/stub/inject/hccl_inject_stub.cpp
)

run_test(mspti_channel_utest)
