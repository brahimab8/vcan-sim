#!/bin/bash
# run_vcan_demo.sh: Orchestrate Motor and ABS ECU demo on vcan0 with live monitoring
# 
# Usage: bash scripts/run_vcan_demo.sh
#
# This script:
# 1. Checks for vcan kernel support (aborts if unavailable)
# 2. Sets up vcan0 interface
# 3. Starts motor_ecu and abs_ecu in background
# 4. Runs can_monitor.py to collect decoded frames into CSV and log
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

DBC_FILE="${REPO_ROOT}/dbc/vcansim.dbc"
PYTHON_VENV="${REPO_ROOT}/venv"
MONITOR_SCRIPT="${REPO_ROOT}/tools/can_monitor.py"

# Output paths (use data/ folder, not build/)
DATA_DIR="${REPO_ROOT}/data"
MONITOR_LOG="${DATA_DIR}/monitor.log"
CSV_FILE="${DATA_DIR}/decoded_signals.csv"

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

# Step 0: Check vcan availability and load module
echo "[setup] Loading vcan kernel module..."
if ! sudo modprobe vcan 2>/dev/null; then
    echo "[setup] vcan kernel module not available on this system."
    exit 2
fi

# Step 1: Set up vcan0
echo "[setup] Creating/bringing up ${VCAN_INTERFACE}..."
sudo ip link add dev "${VCAN_INTERFACE}" type vcan || true  # Ignore if already exists
sudo ip link set up "${VCAN_INTERFACE}" || die "Failed to bring up ${VCAN_INTERFACE}"
echo "[setup] ${VCAN_INTERFACE} is ready"

# Step 2: Create data directory
#-p: no error if already exists
mkdir -p "${DATA_DIR}"

# Step 3: Verify binaries exist
# -f checks if file exists
# -x checks if file exists and is executable
[[ -x "${MOTOR_BIN}" ]] || die "motor_ecu executable not found at ${MOTOR_BIN}. Run 'cmake --build ${BINARY_DIR} -j2' first."
[[ -x "${ABS_BIN}" ]] || die "abs_ecu executable not found at ${ABS_BIN}. Run 'cmake --build ${BINARY_DIR} -j2' first."
[[ -f "${DBC_FILE}" ]] || die "DBC file not found at ${DBC_FILE}"
[[ -f "${MONITOR_SCRIPT}" ]] || die "Monitor script not found at ${MONITOR_SCRIPT}"

# Step 4: Start ECU processes in background
echo "[demo] Starting motor_ecu..."
"${MOTOR_BIN}" &
MOTOR_PID=$!
echo "[demo] motor_ecu started (PID $MOTOR_PID)"

echo "[demo] Starting abs_ecu..."
"${ABS_BIN}" &
ABS_PID=$!
echo "[demo] abs_ecu started (PID $ABS_PID)"

# Brief pause to let ECUs initialize
sleep 0.5

# Step 5: Run monitor for the specified duration, collect CSV and logs
echo "[demo] Running monitor for ${DEMO_DURATION_SECONDS}s, writing to ${DATA_DIR}..."

# Determine python executable
PYTHON_BIN="${PYTHON_VENV}/bin/python"
if [[ ! -f "${PYTHON_BIN}" ]]; then
    PYTHON_BIN="python3"
fi

# Run monitor with timeout and capture output
timeout "${DEMO_DURATION_SECONDS}" "${PYTHON_BIN}" "${MONITOR_SCRIPT}" \
    --interface "${VCAN_INTERFACE}" \
    --dbc "${DBC_FILE}" \
    --csv "${CSV_FILE}" \
    >"${MONITOR_LOG}" 2>&1 || true  # timeout returns 124 on expiration; || true prevents set -e from aborting

echo "[demo] Monitor finished. Artifacts in ${DATA_DIR}:"
echo "  CSV: ${CSV_FILE}"
echo "  Log: ${MONITOR_LOG}"

# Step 6: Let processes finish gracefully
echo "[demo] Waiting for ECU processes to finish..."
sleep 1

# Verify artifacts were created
if [[ -f "${CSV_FILE}" ]]; then
    echo "[demo] ✓ CSV file created ($(wc -l < "${CSV_FILE}") lines)"
else
    echo "[demo] ⚠ CSV file not created (check monitor output)"
fi

if [[ -f "${MONITOR_LOG}" ]]; then
    echo "[demo] ✓ Monitor log created ($(wc -l < "${MONITOR_LOG}") lines)"
else
    echo "[demo] ⚠ Monitor log not created"
fi

echo "[demo] Demo complete!"
exit 0
