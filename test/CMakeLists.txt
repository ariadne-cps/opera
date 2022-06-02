include(CTest)

set(UNIT_TESTS
    test_command_line_interface
    test_thread
    test_geometry
    test_interval
    test_body
    test_trace_sample_range
    test_mode
    test_state
    test_message
    test_serialisation
    test_deserialisation
    test_memory
    test_mqtt
    test_kafka
    test_body_registry
    test_barrier
    test_lookahead_job
    test_lookahead_job_registry
    test_lookahead_job_factory
    test_runtime_io
    test_runtime
)

foreach(TEST ${UNIT_TESTS})
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} opera)
    add_test(${TEST} ${TEST})
endforeach()

add_custom_target(tests)
add_dependencies(tests ${UNIT_TESTS})
