# ============================================================================
# Tell cmake about subdirectories to look into
# ============================================================================
add_subdirectory(gtest-1.7.0)

# ============================================================================
# Look for includes in gtest's include dir and in simuscale's src dir
# ============================================================================
include_directories(${gtest_SOURCE_DIR}/include)


# ============================================================================
# Make STDC MACROS available (for fixed width integers)
# ============================================================================
add_definitions(-D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS)


# ============================================================================
# Use C++11
# ============================================================================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


# ============================================================================
# Add tests
# ============================================================================
# Libraries to link unit tests against
set(test_libs gtest_main simuscale-core)

# List unit tests
set(TESTS test_param_loader Cell_SyncClock_test test_Cell)

# Create a runner for each unit test
foreach (TEST IN LISTS TESTS)
  set(TEST_RUNNER  run_${TEST})
  set(TEST_RUNNERS ${TEST_RUNNERS} ${TEST_RUNNER})
  add_executable(${TEST} ${TEST}.cpp ${PLUGIN_SOURCES})
  target_include_directories(${TEST} PRIVATE ${PROJECT_SOURCE_DIR}/src)
  target_link_libraries(${TEST} ${test_libs})
  add_custom_target(${TEST_RUNNER}
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST}
    DEPENDS ${TEST})
endforeach(TEST)

# Add the specifics of some tests
set(file_to_copy "data/param.in")
add_custom_command(
  TARGET test_param_loader
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/${file_to_copy}"
    "${CMAKE_CURRENT_BINARY_DIR}/${file_to_copy}"
  COMMENT "Copying test parameter file"
)

# Create meta-targets for all tests, unit tests and integration tests
add_custom_target(check DEPENDS utest itest)
add_custom_target(utest DEPENDS ${TEST_RUNNERS})
add_custom_target(itest ${CMAKE_CTEST_COMMAND} DEPENDS simuscale)

# Enable CTest and dive into integration tests
enable_testing()
add_subdirectory(integration_tests)
