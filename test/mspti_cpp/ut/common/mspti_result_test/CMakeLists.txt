add_executable(mspti_result_utest
    main.cpp
    mspti_result_utest.cpp
    ${TOP_DIR}/csrc/common/mspti_result.cpp
)

run_test(mspti_result_utest)
