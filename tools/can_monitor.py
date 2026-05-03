#!/usr/bin/env python3
"""Live CAN monitor for VcanSim.

Reads CAN frames from a SocketCAN interface, decodes known messages using the
project DBC, prints decoded signals, and optionally writes rows to CSV.
"""

from __future__ import annotations

import argparse
import csv
import json
import signal
import sys
import time
from pathlib import Path
from typing import Optional

import can
import cantools


def _project_root() -> Path:
    """Resolve the repository root directory relative to this script's location.
    
    Returns:
        Path to the vcan-sim repository root (parent of tools/ directory).
    """
    return Path(__file__).resolve().parents[1]


def parse_args() -> argparse.Namespace:
    """Parse command-line arguments for the monitor.
    
    Returns:
        Parsed arguments with interface name, DBC file path, optional CSV output,
        and CAN bus receive timeout.
    """
    parser = argparse.ArgumentParser(description="Live CAN monitor using python-can + cantools")
    parser.add_argument("--interface", default="vcan0", help="SocketCAN interface name (default: vcan0)")
    parser.add_argument(
        "--dbc",
        default=str(_project_root() / "dbc" / "vcansim.dbc"),
        help="Path to DBC file",
    )
    parser.add_argument("--csv", default=None, help="Optional CSV output path")
    parser.add_argument(
        "--timeout",
        type=float,
        default=0.5,
        help="Bus receive timeout in seconds (default: 0.5)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    # Load the DBC file
    dbc = cantools.database.load_file(args.dbc)

    # Connect to the SocketCAN interface (vcan0 by default)
    # This allows us to read live CAN frames from the kernel's virtual CAN interface
    try:
        bus = can.interface.Bus(channel=args.interface, bustype="socketcan")
    except Exception as exc:
        print(f"failed to open interface {args.interface}: {exc}", file=sys.stderr)
        return 1

    # Set up CSV output if requested
    csv_file = None
    csv_writer: Optional[csv.DictWriter] = None
    if args.csv:
        csv_file = open(args.csv, "w", newline="", encoding="utf-8")
        csv_writer = csv.DictWriter(
            csv_file,
            fieldnames=["timestamp", "frame_id", "message", "signals_json"],
        )
        csv_writer.writeheader()

    running = True

    def _stop_handler(signum, frame):  # type: ignore[no-untyped-def]
        """Handle SIGINT (Ctrl+C) and SIGTERM gracefully.
        """
        del signum, frame
        nonlocal running
        running = False

    signal.signal(signal.SIGINT, _stop_handler)
    signal.signal(signal.SIGTERM, _stop_handler)

    print(f"monitoring {args.interface} using DBC: {args.dbc}")
    if args.csv:
        print(f"writing CSV: {args.csv}")

    try:
        # Main event loop: continuously listen for CAN frames until interrupted
        while running:
            # Receive a frame with timeout (non-blocking); return None if no frame received
            msg = bus.recv(timeout=args.timeout)
            if msg is None:
                continue

            # Attempt to decode the frame using the DBC database
            # If the frame ID is known, extract signal names and values; otherwise, log as UNKNOWN
            try:
                message = dbc.get_message_by_frame_id(msg.arbitration_id)
                decoded = message.decode(msg.data)  # Convert raw bytes to signal dict
                message_name = message.name
            except KeyError:
                # Frame ID not found in DBC: log raw hex for debugging
                message_name = "UNKNOWN"
                decoded = {"raw_hex": bytes(msg.data).hex()}

            # Use frame's timestamp if available; otherwise use system time
            timestamp = msg.timestamp if msg.timestamp is not None else time.time()
            
            # Print to stdout: log of the decoded CAN message
            print(
                f"{timestamp:.6f} id=0x{msg.arbitration_id:03X} "
                f"dlc={msg.dlc} msg={message_name} signals={decoded}"
            )

            # Write row to CSV for post-processing and analysis
            if csv_writer is not None:
                csv_writer.writerow(
                    {
                        "timestamp": f"{timestamp:.6f}",
                        "frame_id": f"0x{msg.arbitration_id:X}",
                        "message": message_name,
                        "signals_json": json.dumps(decoded, separators=(",", ":")),
                    }
                )
                # Flush to ensure data is written to disk immediately.
                csv_file.flush()
    finally:
        # Clean up resources: close bus and CSV file
        bus.shutdown()
        if csv_file is not None:
            csv_file.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
