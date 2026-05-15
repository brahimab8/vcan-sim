#!/usr/bin/env python3
"""Live CAN monitor for VcanSim.

Reads CAN frames from a SocketCAN interface, decodes known messages using the
project DBC, prints decoded signals, and optionally writes one CSV per CAN-message type
to a output directory.
"""

from __future__ import annotations

import argparse
import csv
import signal
import sys
import time
from pathlib import Path
from typing import IO, Optional

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
        Parsed arguments with interface name, DBC file path, optional CSV
        output directory, and CAN bus receive timeout.
    """
    parser = argparse.ArgumentParser(description="Live CAN monitor using python-can + cantools")
    parser.add_argument("--interface", default="vcan0", help="SocketCAN interface name (default: vcan0)")
    parser.add_argument(
        "--dbc",
        default=str(_project_root() / "dbc" / "vcansim.dbc"),
        help="Path to DBC file",
    )
    parser.add_argument("--csv-dir", default=None, help="Optional directory for per-message CSV output files")
    parser.add_argument(
        "--timeout",
        type=float,
        default=0.5,
        help="Bus receive timeout in seconds (default: 0.5)",
    )
    return parser.parse_args()


def _get_writer(
    writers: dict[str, csv.DictWriter],
    open_files: dict[str, IO],
    csv_dir: Path,
    message_name: str,
    signal_keys: list[str],
) -> csv.DictWriter:
    """Return the DictWriter for *message_name*, creating it on first call.

    Each message gets its own CSV file inside *csv_dir* with the columns:
        timestamp, frame_id, <signal_1>, <signal_2>, ...

    Signal columns are fixed from the first frame seen for that message.
    With a static DBC this is always consistent.

    Args:
        writers: Cache of already-created DictWriter objects keyed by message name.
        open_files: Cache of open file handles keyed by message name.
        csv_dir: Directory where CSV files are written.
        message_name: CAN message name (used as the file stem).
        signal_keys: Ordered signal names from the decoded frame.

    Returns:
        The DictWriter for this message.
    """
    if message_name not in writers:
        path = csv_dir / f"{message_name}.csv"
        f = open(path, "w", newline="", encoding="utf-8")
        fieldnames = ["timestamp", "frame_id"] + list(signal_keys)
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        open_files[message_name] = f
        writers[message_name] = w
    return writers[message_name]


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

    # Per-message CSV state (populated lazily on first frame of each message)
    csv_dir: Optional[Path] = None
    writers: dict[str, csv.DictWriter] = {}
    open_files: dict[str, IO] = {}

    if args.csv_dir:
        csv_dir = Path(args.csv_dir)
        csv_dir.mkdir(parents=True, exist_ok=True)

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
    if csv_dir:
        print(f"writing CSVs to: {csv_dir}/")

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

            # Write to the per-message CSV (skip UNKNOWN frames)
            if csv_dir is not None and message_name != "UNKNOWN":
                writer = _get_writer(
                    writers,
                    open_files,
                    csv_dir,
                    message_name,
                    list(decoded.keys()),
                )
                row: dict = {
                    "timestamp": f"{timestamp:.6f}",
                    "frame_id": f"0x{msg.arbitration_id:X}",
                }
                row.update(decoded)
                writer.writerow(row)
                open_files[message_name].flush()

    finally:
        bus.shutdown()
        for f in open_files.values():
            f.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())