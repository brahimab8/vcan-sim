cmake_minimum_required(VERSION 3.16)
project(vcan_sim LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

# -----------------------------------------------------------------------------
# can_dbc: DBC-generated signal codec (auto-generated at configure time)
# Generated from dbc/vcansim.dbc using cantools generate_c_source
# -----------------------------------------------------------------------------
set(DBC_FILE "${CMAKE_SOURCE_DIR}/dbc/vcansim.dbc")
set(DBC_OUT_DIR "${CMAKE_SOURCE_DIR}/src/dbc")
set(DBC_GENERATED_H "${DBC_OUT_DIR}/vcansim.h")
set(DBC_GENERATED_C "${DBC_OUT_DIR}/vcansim.c")
set(VCANSIM_PYTHON_EXECUTABLE "${CMAKE_SOURCE_DIR}/venv/bin/python")

add_custom_command(
    OUTPUT "${DBC_GENERATED_H}" "${DBC_GENERATED_C}"
    COMMAND "${VCANSIM_PYTHON_EXECUTABLE}" -m cantools generate_c_source
        --use-float
        --output-directory "${DBC_OUT_DIR}"
        "${DBC_FILE}"
    DEPENDS "${DBC_FILE}"
    COMMENT "Generating CAN signal codec from vcansim.dbc"
)

add_custom_target(generate_can_dbc
    DEPENDS "${DBC_GENERATED_H}" "${DBC_GENERATED_C}"
)

add_library(can_dbc STATIC "${DBC_GENERATED_C}")
set_target_properties(can_dbc PROPERTIES LINKER_LANGUAGE C)
target_include_directories(can_dbc PUBLIC "${DBC_OUT_DIR}")
add_dependencies(can_dbc generate_can_dbc)


# -----------------------------------------------------------------------------
# can_common: platform-independent core
# No OS-specific or SocketCAN dependency, fully unit-testable in isolation
# -----------------------------------------------------------------------------
add_library(can_common STATIC
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
    can_dbc
)

# -----------------------------------------------------------------------------
# can_platform: Linux platform layer (SocketCAN + timer)
# Depends on can_common interfaces
# -----------------------------------------------------------------------------
add_library(can_platform STATIC
    src/platform/linux/timer.cpp
    src/platform/linux/socketcan/driver.cpp
)
target_include_directories(can_platform PUBLIC
    src/platform/linux
    src/platform/linux/socketcan
)
target_link_libraries(can_platform PUBLIC
    can_common
)

# -----------------------------------------------------------------------------
# can_monitor: Reusable monitor/decoder library
# Decodes DBC frames and logs to CSV; shared by monitoring_app and future Qt UI
# -----------------------------------------------------------------------------
add_library(can_monitor STATIC
    src/monitor/decoder.cpp
    src/monitor/csv_logger.cpp
    src/monitor/monitor_core.cpp
    src/monitor/monitor_session.cpp
)
target_include_directories(can_monitor PUBLIC
    src/monitor
)
target_link_libraries(can_monitor PUBLIC
    can_dbc
    can_common
)

# -----------------------------------------------------------------------------
# can_ui: Qt-free UI core (CanBridge, UiController)
# -----------------------------------------------------------------------------
add_library(can_ui STATIC
    src/ui/can_bridge.cpp
    src/ui/ui_controller.cpp
)
target_include_directories(can_ui PUBLIC
    src/ui
)
target_link_libraries(can_ui PUBLIC
    can_monitor
    can_control
)

# -----------------------------------------------------------------------------
# can_control: reusable MotorControl command client
# Sends DBC-encoded MotorControl frames over ICanDriver implementations
# -----------------------------------------------------------------------------
add_library(can_control STATIC
    src/control/motor_control_client.cpp
)
target_include_directories(can_control PUBLIC
    src/control
)
target_link_libraries(can_control PUBLIC
    can_dbc
    can_common
)

# -----------------------------------------------------------------------------
# Runner executables: live ECU processes for Linux/vcan0
# -----------------------------------------------------------------------------
add_executable(motor_ecu
    apps/motor_ecu/main.cpp
)
target_include_directories(motor_ecu PRIVATE
    apps
    src/ecu
    src/platform/linux/socketcan
    src/platform/linux/sim
    src/common
)
target_link_libraries(motor_ecu PRIVATE
    can_ecu
    can_platform
)

add_executable(abs_ecu
    apps/abs_ecu/main.cpp
)
target_include_directories(abs_ecu PRIVATE
    apps
    src/ecu
    src/platform/linux/socketcan
    src/platform/linux/sim
    src/common
)
target_link_libraries(abs_ecu PRIVATE
    can_ecu
    can_platform
)

add_executable(monitoring_app
    apps/monitor/main.cpp
)
target_include_directories(monitoring_app PRIVATE
    apps
    src/platform/linux
    src/platform/linux/socketcan
    src/common
)
target_link_libraries(monitoring_app PRIVATE
    can_monitor
    can_platform
)

add_executable(motor_control
    apps/motor_control/main.cpp
)
target_include_directories(motor_control PRIVATE
    apps
    src/platform/linux
    src/platform/linux/socketcan
    src/common
)
target_link_libraries(motor_control PRIVATE
    can_control
    can_platform
)

# -----------------------------------------------------------------------------
# vcan_ui: Qt-based UI (uses can_monitor + can_control)
# Built only if Qt Widgets are available on the system.
# -----------------------------------------------------------------------------
find_package(Qt5 COMPONENTS Widgets QUIET)
if(Qt5_FOUND)
    add_executable(vcan_ui
        apps/ui/main.cpp
        apps/ui/can_worker_qt.cpp
        apps/ui/main_window.cpp
        apps/ui/can_worker_qt.h
        apps/ui/main_window.h
    )
    set_target_properties(vcan_ui PROPERTIES AUTOMOC ON)
    target_include_directories(vcan_ui PRIVATE
        apps
        src
        src/ui
        src/monitor
    )
    target_link_libraries(vcan_ui PRIVATE
        Qt5::Widgets
        can_monitor
        can_control
        can_platform
        can_ui
    )
else()
    message(STATUS "Skipping vcan_ui: Qt5 Widgets not found")
endif()

# -----------------------------------------------------------------------------
# unit_tests: GoogleTest suite
# Links against can_common and can_ecu  (no SocketCAN / OS dependency)
# -----------------------------------------------------------------------------
find_package(GTest REQUIRED)
enable_testing()

add_executable(unit_tests
    tests/unit/test_motor_ecu.cpp
    tests/unit/test_abs_ecu.cpp
    tests/unit/test_sim_engine.cpp
    tests/unit/test_csv_logger.cpp
    tests/unit/test_decoder.cpp
    tests/unit/test_monitor_core.cpp
    tests/unit/test_monitor_session.cpp
    tests/unit/test_motor_control.cpp
)
target_include_directories(unit_tests PRIVATE
    tests/mocks
    src/platform/linux/sim
)
target_link_libraries(unit_tests PRIVATE
    can_dbc
    can_common
    can_ecu
    can_monitor
    can_control
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
    tests/integration/test_motor_control_integration.cpp
    tests/integration/test_abs_integration.cpp
)
target_include_directories(integration_tests PRIVATE
    tests/mocks
    src/platform/linux/sim
    tests/integration/utils
)
target_link_libraries(integration_tests PRIVATE
    can_common
    can_ecu
    can_dbc
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
if(EXISTS "${VCANSIM_PYTHON_EXECUTABLE}")
    execute_process(
        COMMAND "${VCANSIM_PYTHON_EXECUTABLE}" -c
            "import importlib.util; print('1' if all(importlib.util.find_spec(x) for x in ('pytest', 'cantools')) else '0')"
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