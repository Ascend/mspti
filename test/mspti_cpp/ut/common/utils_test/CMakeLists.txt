add_executable(mspti_utils_utest
    main.cpp
    utils_utest.cpp
    ${TOP_DIR}/csrc/common/utils.cpp
    ${TOP_DIR}/csrc/common/runtime_utils.cpp
    ${TOP_DIR}/csrc/common/shutdown_flag.cpp
    ${TOP_DIR}/csrc/common/function_loader.cpp
    ${UT_TOP_DIR}/stub/inject/acl_inject_stub.cpp
)

run_test(mspti_utils_utest)
