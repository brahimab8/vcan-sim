cmake_minimum_required(VERSION 3.16)
project(vcan_sim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# -----------------------------------------------------------------------------
# can_common: platform-independent core
# No OS-specific or SocketCAN dependency, fully unit-testable in isolation
# -----------------------------------------------------------------------------
add_library(can_common STATIC
    src/common/signal_encoder.cpp
    src/common/base_ecu.cpp
)
target_include_directories(can_common PUBLIC
    src/common
)

# -----------------------------------------------------------------------------
# can_ecu: ECU simulators
# Depends on can_common. No platform or SocketCAN dependency
# Unit-testable via MockCanDriver and MockTimer
# -----------------------------------------------------------------------------
add_library(can_ecu STATIC
    src/ecu/motor_ecu.cpp
    src/ecu/abs_ecu.cpp
)
target_include_directories(can_ecu PUBLIC
    src/ecu
)
target_link_libraries(can_ecu PUBLIC
    can_common
)

# -----------------------------------------------------------------------------
# unit_tests: GoogleTest suite
# Links against can_common and can_ecu  (no platform dependency)
# -----------------------------------------------------------------------------
find_package(GTest REQUIRED)
enable_testing()

add_executable(unit_tests
    tests/unit/test_signal_encoding.cpp
    tests/unit/test_motor_ecu.cpp
    tests/unit/test_abs_ecu.cpp
)
target_include_directories(unit_tests PRIVATE
    tests/mocks
)
target_link_libraries(unit_tests PRIVATE
    can_common
    can_ecu
    GTest::GTest
    GTest::Main
)

add_test(NAME unit_tests COMMAND unit_tests)

# -----------------------------------------------------------------------------
# integration_tests: C++ component integration (no SocketCAN/vcan0 required)
# Verifies ECU run-loop behavior with injected dependencies
# -----------------------------------------------------------------------------
add_executable(integration_tests
    tests/integration/test_motor_integration.cpp
    tests/integration/test_abs_integration.cpp
)
target_include_directories(integration_tests PRIVATE
    tests/mocks
    tests/integration/utils
)
target_link_libraries(integration_tests PRIVATE
    can_common
    can_ecu
    GTest::GTest
    GTest::Main
)

add_test(NAME integration_tests COMMAND integration_tests)

# -----------------------------------------------------------------------------
# frame_dump: emits deterministic ECU frames for Python DBC integration tests
# -----------------------------------------------------------------------------
add_executable(frame_dump
    tests/integration/utils/frame_dump_main.cpp
)
target_include_directories(frame_dump PRIVATE
    tests/mocks
)
target_link_libraries(frame_dump PRIVATE
    can_common
    can_ecu
)

# -----------------------------------------------------------------------------
# python_integration_tests: optional CTest gate for DBC decoding checks
# Uses project venv only: ${CMAKE_SOURCE_DIR}/venv/bin/python
# Also provides a single build target: python_integration
# -----------------------------------------------------------------------------
set(VCANSIM_PYTHON_EXECUTABLE "${CMAKE_SOURCE_DIR}/venv/bin/python")

if(EXISTS "${VCANSIM_PYTHON_EXECUTABLE}")
    execute_process(
        COMMAND "${VCANSIM_PYTHON_EXECUTABLE}" -c "import importlib.util; ok=importlib.util.find_spec('pytest') and importlib.util.find_spec('cantools'); print('1' if ok else '0')"
        OUTPUT_VARIABLE VCANSIM_PY_INTEGRATION_READY
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(VCANSIM_PY_INTEGRATION_READY STREQUAL "1")
        # Register Python integration test with CTest. The test will run pytest
        # and uses the built `frame_dump` binary via the VCANSIM_FRAME_DUMP_BIN env var.
        add_test(
            NAME python_integration_tests
            COMMAND "${VCANSIM_PYTHON_EXECUTABLE}" -m pytest -q ${CMAKE_SOURCE_DIR}/tests/integration/test_frames.py
        )
        set_tests_properties(python_integration_tests PROPERTIES
            ENVIRONMENT "VCANSIM_FRAME_DUMP_BIN=$<TARGET_FILE:frame_dump>"
        )
    else()
        message(STATUS "Skipping python_integration_tests: pytest and/or cantools not found in ${VCANSIM_PYTHON_EXECUTABLE}")
    endif()
else()
    message(STATUS "Skipping python_integration_tests: venv not found at ${VCANSIM_PYTHON_EXECUTABLE}")
endif()