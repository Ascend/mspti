add_library(profapi SHARED
    ./stub_so.cpp
)

set_target_properties(profapi
    PROPERTIES
    OUTPUT_NAME profapi
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib64
)

add_executable(function_loader_utest
    main.cpp
    function_loader_utest.cpp
    ${TOP_DIR}/csrc/common/utils.cpp
    ${TOP_DIR}/csrc/common/shutdown_flag.cpp
    ${TOP_DIR}/csrc/common/function_loader.cpp
    ${UT_TOP_DIR}/stub/inject/acl_inject_stub.cpp
)

add_dependencies(function_loader_utest profapi)

run_test(function_loader_utest)
