add_executable(mspti_pm_sampling_decoder_utest
    main.cpp
    pm_sampling_decoder_utest.cpp
    ${TOP_DIR}/csrc/pm_sampling/pm_sampling_decoder.cpp
)

run_test(mspti_pm_sampling_decoder_utest)

add_executable(mspti_pm_sampling_image_api_utest
    main.cpp
    pm_sampling_image_api_utest.cpp
    pm_sampling_session_stub.cpp
    ${TOP_DIR}/csrc/pm_sampling/pm_sampling_decoder.cpp
    ${TOP_DIR}/csrc/pm_sampling/mspti_pm_sampling_api.cpp
    ${TOP_DIR}/csrc/pm_sampling/pm_sampling_metric_table.cpp
)
# Link only the result/metric APIs exercised by this isolated test; device lifecycle
# dependencies belong to the full library and real-session integration tests.
target_compile_options(mspti_pm_sampling_image_api_utest PRIVATE -ffunction-sections -fdata-sections)
target_link_options(mspti_pm_sampling_image_api_utest PRIVATE -Wl,--gc-sections)
run_test(mspti_pm_sampling_image_api_utest)
