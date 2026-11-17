# The UT stub has no PlogManager, so resolve csrc/common/plog_manager.h to production.
include_directories(BEFORE ${TOP_DIR})

add_executable(mspti_plog_manager_utest
    main.cpp
    plog_manager_utest.cpp
)

run_test(mspti_plog_manager_utest)
