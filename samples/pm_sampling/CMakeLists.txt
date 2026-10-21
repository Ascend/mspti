# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# Licensed under Mulan PSL v2: http://license.coscl.org.cn/MulanPSL2
cmake_minimum_required(VERSION 3.14)
project(PM_SAMPLING LANGUAGES CXX)

if(NOT DEFINED MSPTI_ROOT)
    if(DEFINED ENV{MSPTI_ROOT})
        set(MSPTI_ROOT "$ENV{MSPTI_ROOT}")
    elseif(DEFINED ENV{ASCEND_HOME_PATH})
        set(MSPTI_ROOT "$ENV{ASCEND_HOME_PATH}/tools/mspti")
    else()
        message(FATAL_ERROR "Source the CANN set_env.sh or set MSPTI_ROOT to the installed tools/mspti directory")
    endif()
endif()
get_filename_component(MSPTI_ROOT "${MSPTI_ROOT}" ABSOLUTE)
set(MSPTI_HEADER "${MSPTI_ROOT}/include/mspti_pm_sampling.h")
set(MSPTI_LIBRARY "${MSPTI_ROOT}/lib64/libmspti.so")
if(NOT EXISTS "${MSPTI_HEADER}" OR NOT EXISTS "${MSPTI_LIBRARY}")
    message(FATAL_ERROR "Missing installed header or library under ${MSPTI_ROOT}")
endif()
message(STATUS "MSPTI header: ${MSPTI_HEADER}")
message(STATUS "MSPTI library: ${MSPTI_LIBRARY}")

find_package(Threads REQUIRED)
add_executable(pm_sampling pm_sampling.cpp)
set_target_properties(pm_sampling PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_include_directories(pm_sampling PRIVATE "${MSPTI_ROOT}/include")
target_link_libraries(pm_sampling PRIVATE "${MSPTI_LIBRARY}" Threads::Threads)
set_target_properties(pm_sampling PROPERTIES BUILD_RPATH "${MSPTI_ROOT}/lib64")
target_compile_options(pm_sampling PRIVATE
    -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-all -ftrapv
    -Wall -Wextra -Werror=uninitialized -Werror=return-type)
target_link_options(pm_sampling PRIVATE -Wl,-z,relro,-z,now,-z,noexecstack)
