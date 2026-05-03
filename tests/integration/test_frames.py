"""
Python integration tests (DBC validation)

Note:
- These tests run `frame_dump`, a small C++ utility that outputs deterministic
    CAN frames for the Motor and ABS ECUs. The test locates the binary via the
    `VCANSIM_FRAME_DUMP_BIN` environment variable; if unset it falls back to
    `build/frame_dump` inside the project tree.

"""

import os
import subprocess
from pathlib import Path

import cantools
import pytest


def _project_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _frame_dump_bin() -> Path:
    env_bin = os.environ.get("VCANSIM_FRAME_DUMP_BIN")
    if env_bin:
        return Path(env_bin)

    return _project_root() / "build" / "frame_dump"


def _load_dbc():
    dbc_path = _project_root() / "dbc" / "vcansim.dbc"
    return cantools.database.load_file(str(dbc_path))


def _parse_frames(output: str):
    frames = {"motor": [], "abs": []}

    for raw_line in output.strip().splitlines():
        line = raw_line.strip()
        if not line:
            continue

        topic, frame_id, dlc, data_hex = line.split(",")
        data_bytes = bytes.fromhex(data_hex)
        frames[topic].append(
            {
                "id": int(frame_id),
                "dlc": int(dlc),
                "data": data_bytes,
            }
        )

    return frames


def _run_frame_dump() -> str:
    binary = _frame_dump_bin()
    if not binary.exists():
        raise RuntimeError(
            f"frame_dump binary not found at {binary}; build it with: cmake --build build --target frame_dump"
        )

    result = subprocess.run(
        [str(binary)],
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def test_motor_frames_decode_with_dbc():
    db = _load_dbc()
    frames = _parse_frames(_run_frame_dump())

    assert len(frames["motor"]) == 2

    m0 = db.get_message_by_frame_id(frames["motor"][0]["id"]).decode(frames["motor"][0]["data"])
    m1 = db.get_message_by_frame_id(frames["motor"][1]["id"]).decode(frames["motor"][1]["data"])

    assert m0["RPM"] == 800
    assert m0["Temperature"] == 20

    assert m1["RPM"] == 1200
    assert m1["Temperature"] == 45


def test_abs_frames_decode_with_dbc():
    db = _load_dbc()
    frames = _parse_frames(_run_frame_dump())

    assert len(frames["abs"]) == 2

    a0 = db.get_message_by_frame_id(frames["abs"][0]["id"]).decode(frames["abs"][0]["data"])
    a1 = db.get_message_by_frame_id(frames["abs"][1]["id"]).decode(frames["abs"][1]["data"])

    assert a0["Wheel_FL"] == 0
    assert a0["Wheel_FR"] == 0
    assert a0["Wheel_RL"] == 0
    assert a0["Wheel_RR"] == 0

    assert a1["Wheel_FL"] == pytest.approx(10.0)
    assert a1["Wheel_FR"] == pytest.approx(10.2)
    assert a1["Wheel_RL"] == pytest.approx(9.8)
    assert a1["Wheel_RR"] == pytest.approx(9.9)
