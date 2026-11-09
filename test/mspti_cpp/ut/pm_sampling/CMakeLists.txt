file(GLOB_RECURSE SRC_FILE
    ${TOP_DIR}/csrc/activity/*.cpp
    ${TOP_DIR}/csrc/pm_sampling/*.cpp
)

add_executable(pm_sampling_utest
    ${SRC_FILE}
    main.cpp
    pm_sampling_contract_utest.cpp
    pm_sampling_object_utest.cpp
    pm_sampling_manager_utest.cpp
    pm_sampling_parser_utest.cpp
    pm_sampling_decoder_utest.cpp
    pm_sampling_image_api_utest.cpp
    pm_sampling_exception_injection.cpp
    pm_sampling_metric_table_utest.cpp
    pm_sampling_dcmi_utest.cpp
    ${TOP_DIR}/csrc/common/context_manager.cpp
    ${TOP_DIR}/csrc/common/utils.cpp
    ${TOP_DIR}/csrc/common/runtime_utils.cpp
    ${TOP_DIR}/csrc/common/thread_pool.cpp
    ${TOP_DIR}/csrc/common/function_loader.cpp
    ${TOP_DIR}/csrc/common/inject/plog_inject.cpp
    ${UT_TOP_DIR}/stub/inject/driver_inject_stub.cpp
    ${UT_TOP_DIR}/stub/inject/profapi_inject_stub.cpp
    ${UT_TOP_DIR}/stub/inject/acl_inject_stub.cpp
)

run_test(pm_sampling_utest)

# Link-time wrappers keep compiler-generated unwind information intact on ARM64.
# These symbols are Decoder::GetSize and Decoder::CheckPlan under the Linux C++ ABI.
target_link_options(pm_sampling_utest PRIVATE
    "-Wl,--wrap=_ZN5Mspti10PmSampling17PmSamplingDecoder7GetSizeEjjmRm"
    "-Wl,--wrap=_ZN5Mspti10PmSampling17PmSamplingDecoder9CheckPlanEPKhmRKSt6vectorIjSaIjEEjm"
)
