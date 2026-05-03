# Signal Encoding

This document describes how CAN signals are encoded in C++ and how they are defined in the DBC file.
Both sides must agree on the same bit layout, scaling, and offset for encoding and decoding to be consistent end-to-end.

## Encoding Approach

Signals are encoded manually in C++ without any external library.
This gives direct control over bit layout, scaling, offset, and endianness at the frame level.

The same signals are defined in `dbc/vcansim.dbc` and decoded by `cantools` on the Python side,
which serves as an independent verification that C++ encoding and Python decoding are consistent.

Signal encoding is split across two layers:
- The `SignalEncoder` namespace provides primitive byte-level operations (pack/unpack uint8, uint16 Little Endian)
- Each ECU class applies its own scaling and offset before calling `SignalEncoder`

`SignalEncoder` methods return `bool` and perform bounds checks internally.
They return `false` when the byte offset is out of frame bounds.

For example, Motor ECU encodes RPM as:
`raw = rpm * 2` then calls `SignalEncoder::encodeUint16LE(frame, 0, raw)` and checks the returned `bool`.

## Motor ECU (`0x100`)

### RPM

| Property | Value |
|---|---|
| CAN ID | `0x100` |
| Type | uint16, Little Endian |
| Position | byte 0 (LSB), byte 1 (MSB) |
| Scale | 0.5 |
| Offset | 0 |
| Range | 0–8000 rpm |
| Raw formula | `raw = rpm * 2`  (scale = 0.5) |

Example:

| rpm | raw | byte 0 | byte 1 |
|---|---|---|---|
| 3000 | 6000 = `0x1770` | `0x70` | `0x17` |

### Temperature

| Property | Value |
|---|---|
| CAN ID | `0x100` |
| Type | uint8 |
| Position | byte 2 |
| Scale | 1 |
| Offset | -40 |
| Range | -40–150 °C |
| Raw formula | `raw = temp + 40` |

Example:

| temp (°C) | raw | byte 2 |
|---|---|---|
| 85 | 125 = `0x7D` | `0x7D` |
 
Note: The physical temperature value in C++ is handled as `int16_t` (range -40 to 150 °C), then encoded on the bus as `uint8` using `raw = temp + 40` (range 0 to 190).

## ABS ECU (`0x200`)

All four wheel speeds are packed into one 8-byte frame, each as a 16-bit unsigned integer.

| Signal | Position | Scale | Offset | Range |
|---|---|---|---|---|
| Wheel_FL | bytes 0-1 | 0.1 | 0 | 0–300 km/h |
| Wheel_FR | bytes 2-3 | 0.1 | 0 | 0–300 km/h |
| Wheel_RL | bytes 4-5 | 0.1 | 0 | 0–300 km/h |
| Wheel_RR | bytes 6-7 | 0.1 | 0 | 0–300 km/h |

Raw formula: `raw = speed / 0.1`

Note: In C++ implementation, ABS wheel speeds are represented internally as deci-km/h integers (0.1 km/h units), and encoded directly as raw uint16 values.

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
```

The `Monitor` node is a passive consumer used to make the DBC explicit about who receives the data.

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