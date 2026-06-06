#!/bin/bash
# run_vcan_demo.sh: Orchestrate Motor and ABS ECU demo on vcan0 with live monitoring
#
# Usage: bash scripts/run_vcan_demo.sh [cli|ui]
#
# This script:
# 1. Checks for vcan kernel support (aborts if unavailable)
# 2. Sets up vcan0 interface
# 3. Starts motor_ecu and abs_ecu in background
# 4. Runs the C++ `monitoring_app` by default, or `vcan_ui` with the `ui` arg
# 5. Stops ECU processes cleanly
# 6. Collects artifacts into data/ folder
#
# Exit codes:
#   0 = success
#   1 = error (missing binary, failed setup, etc.)
#   2 = vcan not available

set -e

# Configuration
VCAN_INTERFACE="vcan0"
DEMO_DURATION_SECONDS=5  # Let ECUs run for 5 seconds
MODE="cli"

if [[ $# -gt 1 ]]; then
    echo "Usage: bash scripts/run_vcan_demo.sh [cli|ui]" >&2
    exit 1
fi

if [[ $# -eq 1 ]]; then
    MODE="$1"
fi

case "${MODE}" in
    cli|ui)
        ;;
    *)
        echo "Usage: bash scripts/run_vcan_demo.sh [cli|ui]" >&2
        exit 1
        ;;
esac

# Resolve script location and repository source directory reliably
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="${CMAKE_SOURCE_DIR:-$SCRIPT_DIR/..}"
REPO_ROOT="$(cd "${REPO_ROOT}" && pwd)"

# Determine build/binary directory: prefer CMake variable, then repo's build/, else fallback
if [[ -n "${CMAKE_BINARY_DIR:-}" && -d "${CMAKE_BINARY_DIR}" ]]; then
    BINARY_DIR="${CMAKE_BINARY_DIR}"
elif [[ -d "${REPO_ROOT}/build" ]]; then
    BINARY_DIR="${REPO_ROOT}/build"
else
    BINARY_DIR="${CMAKE_BINARY_DIR:-.}"
fi

MOTOR_BIN="${BINARY_DIR}/motor_ecu"
ABS_BIN="${BINARY_DIR}/abs_ecu"
MONITORING_APP_BIN="${BINARY_DIR}/monitoring_app"
VCAN_UI_BIN="${BINARY_DIR}/vcan_ui"
MOTOR_CONTROL_BIN="${BINARY_DIR}/motor_control"

DBC_FILE="${REPO_ROOT}/dbc/vcansim.dbc"

# Output paths
DATA_DIR="${REPO_ROOT}/data"
CSV_DIR="${DATA_DIR}/csv"
MONITOR_LOG="${DATA_DIR}/monitor.log"

# Helper function: error exit
die() {
    echo "ERROR: $*" >&2
    exit 1
}

# Helper function: cleanup on exit (stop ECU processes)
cleanup() {
    local exit_code=$?
    if [[ -n "$MOTOR_PID" ]] && kill -0 "$MOTOR_PID" 2>/dev/null; then
        echo "[cleanup] Stopping motor_ecu (PID $MOTOR_PID)..."
        kill "$MOTOR_PID" 2>/dev/null || true
    fi
    if [[ -n "$ABS_PID" ]] && kill -0 "$ABS_PID" 2>/dev/null; then
        echo "[cleanup] Stopping abs_ecu (PID $ABS_PID)..."
        kill "$ABS_PID" 2>/dev/null || true
    fi
    exit $exit_code
}
trap cleanup EXIT

# Step 0: Set up the vcan interface
"${SCRIPT_DIR}/setup_vcan.sh" "${VCAN_INTERFACE}"

# Step 1: Create output directories
mkdir -p "${CSV_DIR}"   # data/csv/ holds one file per message

# Step 2: Verify binaries exist
# -f checks if file exists
# -x checks if file exists and is executable
[[ -x "${MOTOR_BIN}" ]] || die "motor_ecu executable not found at ${MOTOR_BIN}. Run 'cmake --build ${BINARY_DIR} -j2' first."
[[ -x "${ABS_BIN}" ]] || die "abs_ecu executable not found at ${ABS_BIN}. Run 'cmake --build ${BINARY_DIR} -j2' first."
[[ -f "${DBC_FILE}" ]] || die "DBC file not found at ${DBC_FILE}"
if [[ "${MODE}" == "ui" ]]; then
    [[ -x "${VCAN_UI_BIN}" ]] || die "vcan_ui executable not found at ${VCAN_UI_BIN}. Reconfigure/build with Qt Widgets available first."
else
    [[ -x "${MONITORING_APP_BIN}" ]] || die "monitoring_app executable not found at ${MONITORING_APP_BIN}. Run 'cmake --build ${BINARY_DIR} -j2' first."
fi
[[ -x "${MOTOR_CONTROL_BIN}" ]] || die "motor_control executable not found at ${MOTOR_CONTROL_BIN}. Run 'cmake --build ${BINARY_DIR} -j2' first."

# Step 3: Start ECU processes in background
echo "[demo] Starting motor_ecu..."
"${MOTOR_BIN}" &
MOTOR_PID=$!
echo "[demo] motor_ecu started (PID $MOTOR_PID)"

echo "[demo] Starting abs_ecu..."
"${ABS_BIN}" &
ABS_PID=$!
echo "[demo] abs_ecu started (PID $ABS_PID)"

echo "[demo] Sending motor control command (motor_control)..."
"${MOTOR_CONTROL_BIN}" 3000 "${VCAN_INTERFACE}" &
MOTOR_CONTROL_PID=$!
echo "[demo] motor_control started (PID $MOTOR_CONTROL_PID)"

# Brief pause to let ECUs initialize
sleep 0.5

# Step 4: Run the chosen UI/monitor frontend
if [[ "${MODE}" == "ui" ]]; then
    echo "[demo] Launching Qt UI..."
    "${VCAN_UI_BIN}" "${VCAN_INTERFACE}"
    echo "[demo] Qt UI finished."
else
    echo "[demo] Running monitor for ${DEMO_DURATION_SECONDS}s, writing CSVs to ${CSV_DIR}/..."

    # Run C++ monitor with timeout and capture output
    timeout "${DEMO_DURATION_SECONDS}" "${MONITORING_APP_BIN}" "${VCAN_INTERFACE}" "${DBC_FILE}" >"${MONITOR_LOG}" 2>&1 || true

    echo "[demo] Monitor finished."
fi

# Step 5: Let processes finish gracefully
echo "[demo] Waiting for ECU processes to finish..."
sleep 1

# Report artifacts
echo "[demo] Artifacts in ${DATA_DIR}:"
if [[ "${MODE}" == "ui" ]]; then
    echo "[demo] UI mode does not write the CLI monitor log by default."
else
    echo "  Log: ${MONITOR_LOG}"

    CSV_COUNT=0
    # Check for CSV files and count lines for each
    # compgen -G checks for files matching the pattern; if none, it returns non-zero
    if compgen -G "${CSV_DIR}/*.csv" > /dev/null 2>&1; then
        for csv_file in "${CSV_DIR}"/*.csv; do
            line_count=$(wc -l < "${csv_file}")
            echo "  CSV: ${csv_file} (${line_count} lines)"
            (( CSV_COUNT++ )) || true
        done
        echo "[demo] ✓ ${CSV_COUNT} CSV file(s) created"
    else
        echo "[demo] ⚠ No CSV files found in ${CSV_DIR} (check monitor log)"
    fi

    if [[ -f "${MONITOR_LOG}" ]]; then
        echo "[demo] ✓ Monitor log created ($(wc -l < "${MONITOR_LOG}") lines)"
    else
        echo "[demo] ⚠ Monitor log not created"
    fi
fi

echo "[demo] Demo complete!"
exit 0
