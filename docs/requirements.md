# Requirements

## Functional Requirements

| ID | Description |
|---|---|
| FR-01 | The system shall use `vcan0` as the virtual CAN bus |
| FR-02 | Motor ECU shall transmit RPM (0–8000 rpm) every 100 ms |
| FR-03 | Motor ECU shall transmit engine temperature (−40–150 °C) every 100 ms |
| FR-04 | ABS ECU shall transmit four wheel speeds (0–300 km/h) every 20 ms |
| FR-05 | All transmitted CAN signals shall be defined in a `.dbc` file including CAN ID, bit position, scaling, offset, and unit |
| FR-06 | Motor ECU shall receive a target RPM command signal from the host over CAN |
| FR-07 | The monitor shall decode incoming CAN frames using DBC-generated C code and display signal values in real time |
| FR-08 | The monitor shall log all decoded signals with timestamps to a CSV file, one file per message type |
| FR-09 | The GUI shall display live decoded signal values for all ECUs |
| FR-10 | The GUI shall highlight any signal whose raw value exceeds its DBC-defined range |
| FR-11 | The GUI shall allow the user to send a target RPM command to the Motor ECU |

## Non-Functional Requirements

| ID | Description |
|---|---|
| NFR-01 | Each ECU shall run as an independent process |
| NFR-02 | ECU logic shall be decoupled from the CAN driver via the `ICanDriver` interface and from shared behaviour via a `BaseEcu` abstract class |
| NFR-03 | Frame data structures shall use fixed-size types with no dynamic memory allocation (`std::array`, stack allocation) |
| NFR-04 | After `vcan0` is configured, ECU executables and monitoring tools shall run without root privileges |
| NFR-05 | The project shall build using a standard CMake workflow |
| NFR-06 | DBC-generated source files shall be produced automatically at build time via a CMake custom command |
| NFR-07 | Signal encoding and decoding logic shall be unit-testable in isolation from SocketCAN |
| NFR-08 | The codebase shall use C++17 |
| NFR-09 | The DBC file shall be the single source of truth for all signal definitions; both ECU encoding and monitor decoding shall use the same DBC-generated code |
| NFR-10 | The GUI shall be implemented using Qt Widgets |

## Out of Scope

- Physical CAN layer (voltage levels, termination, bit timing)
- CAN error frame simulation (Bus-Off, Error Passive, not supported by `vcan`)
- UDS / OBD2 diagnostic protocol implementation
