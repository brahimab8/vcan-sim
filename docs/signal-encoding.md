# Signal Encoding

This document describes how CAN signals are defined in the DBC file and how encoding and decoding are handled in the project.
Both sides must agree on the same bit layout, scaling, and offset for encoding and decoding to be consistent end-to-end.

## Encoding Approach

Signal encoding and decoding are handled by C code generated at build time from `dbc/vcansim.dbc` using `cantools generate_c_source`.
The generated code lives in `src/dbc/` and is compiled into the `can_dbc` static library, which is linked by both the ECUs and the monitor.
This ensures the DBC file is the single source of truth.

The generated pipeline for each signal is:
- `_encode(float)` — converts a physical value to a raw integer (applies scale and offset)
- `_pack()` — writes raw integers into a frame byte buffer
- `_unpack()` — reads raw integers from a received frame byte buffer
- `_decode(raw)` — converts a raw integer back to a physical value
- `_is_in_range(raw)` — checks the raw value is within DBC-defined bounds

To regenerate the code manually (also runs automatically at build time via CMake):

```bash
./venv/bin/python -m cantools generate_c_source --use-float \
    --output-directory src/dbc/ dbc/vcansim.dbc
```

See the [cantools documentation](https://cantools.readthedocs.io/en/latest/#cantools-generate-c-source) for more details.

## Motor ECU

### MotorStatus (`0x100`) — transmitted

#### RPM

| Property | Value |
|---|---|
| Type | uint16, Little Endian |
| Position | bytes 0–1 |
| Scale | 0.5 |
| Offset | 0 |
| Range | 0–8000 rpm |

Example:

| rpm | raw | byte 0 | byte 1 |
|---|---|---|---|
| 3000 | 6000 = `0x1770` | `0x70` | `0x17` |

#### Temperature

| Property | Value |
|---|---|
| Type | uint8 |
| Position | byte 2 |
| Scale | 1 |
| Offset | -40 |
| Range | -40–150 °C |

Example:

| temp (°C) | raw | byte 2 |
|---|---|---|
| 85 | 125 = `0x7D` | `0x7D` |

### MotorControl (`0x300`) — received

| Property | Value |
|---|---|
| Type | uint16, Little Endian |
| Position | bytes 0–1 |
| Scale | 0.5 |
| Offset | 0 |
| Range | 0–8000 rpm |

Example:

| target rpm | raw | byte 0 | byte 1 |
|---|---|---|---|
| 3000 | 6000 = `0x1770` | `0x70` | `0x17` |

## ABS ECU

### ABSStatus (`0x200`) — transmitted

All four wheel speeds are packed into one 8-byte frame, each as a 16-bit unsigned Little Endian integer.

| Signal | Position | Scale | Offset | Range |
|---|---|---|---|---|
| Wheel_FL | bytes 0–1 | 0.1 | 0 | 0–300 km/h |
| Wheel_FR | bytes 2–3 | 0.1 | 0 | 0–300 km/h |
| Wheel_RL | bytes 4–5 | 0.1 | 0 | 0–300 km/h |
| Wheel_RR | bytes 6–7 | 0.1 | 0 | 0–300 km/h |

## DBC File: `dbc/vcansim.dbc`

The DBC file is the single source of truth for signal definitions.
It follows the standard DBC format used by tools like CANalyzer, CANdb++, and `cantools`.

```dbc
BU_: MotorECU ABSECU Monitor

BO_ 256 MotorStatus: 3 MotorECU
 SG_ RPM         : 0|16@1+ (0.5,0)   [0|8000]  "rpm"  Monitor
 SG_ Temperature : 16|8@1+ (1,-40)   [-40|150] "degC" Monitor

BO_ 512 ABSStatus: 8 ABSECU
 SG_ Wheel_FL : 0|16@1+  (0.1,0) [0|300] "km/h" Monitor
 SG_ Wheel_FR : 16|16@1+ (0.1,0) [0|300] "km/h" Monitor
 SG_ Wheel_RL : 32|16@1+ (0.1,0) [0|300] "km/h" Monitor
 SG_ Wheel_RR : 48|16@1+ (0.1,0) [0|300] "km/h" Monitor

BO_ 768 MotorControl: 2 Monitor
 SG_ TargetRPM : 0|16@1+ (0.5,0) [0|8000] "rpm" MotorECU
```

The `Monitor` node sends `MotorControl` frames and passively consumes all status messages, making the DBC explicit about participants and data flow.

### DBC Syntax Reference

| Field | Meaning |
|---|---|
| `BO_` | Message definition: ID, name, length in bytes, sender |
| `BU_` | Node list: declared participants on the CAN network |
| `SG_` | Signal definition |
| `0\|16` | Start bit \| length in bits |
| `@1+` | Little Endian (`1`), unsigned (`+`) |
| `(0.5, 0)` | Scale, offset |
| `[0\|8000]` | Min, max value |
| `"rpm"` | Unit |