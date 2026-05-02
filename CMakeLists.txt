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