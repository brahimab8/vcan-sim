#!/bin/bash
# setup_vcan.sh: Create and bring up the vcan interface used by VcanSim
# Usage: bash scripts/setup_vcan.sh [interface]
#
# This script:
# 1. Checks for vcan kernel support (aborts if unavailable)
# 2. Creates the requested vcan interface if needed
# 3. Brings the interface up
#
# Exit codes:
#   0 = success
#   1 = error
#   2 = vcan not available

set -e

VCAN_INTERFACE="${1:-vcan0}"

die() {
    echo "ERROR: $*" >&2
    exit 1
}

echo "[setup] Loading vcan kernel module..."
if ! sudo modprobe vcan 2>/dev/null; then
    echo "[setup] vcan kernel module not available on this system."
    exit 2
fi

echo "[setup] Creating/bringing up ${VCAN_INTERFACE}..."
sudo ip link add dev "${VCAN_INTERFACE}" type vcan || true
sudo ip link set up "${VCAN_INTERFACE}" || die "Failed to bring up ${VCAN_INTERFACE}"
echo "[setup] ${VCAN_INTERFACE} is ready"
