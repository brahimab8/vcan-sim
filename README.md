# VcanSim

> Virtual CAN Network Simulator for Embedded Linux

VcanSim simulates a multi-ECU CAN network entirely in software, no hardware required.
Built on Linux SocketCAN (`vcan`), it runs the real kernel CAN stack without any physical CAN interface.

## Overview

VcanSim consists of two simulated ECU nodes that produce realistic CAN traffic over a virtual bus,
and a Python-based monitor that decodes and logs signals in real time using a DBC file.

ECU logic is decoupled from the platform-specific CAN driver via an `ICanDriver` interface,
improving portability, testability, and future migration to embedded hardware.

## Features

- Two C++ ECU simulators producing realistic CAN traffic
- DBC-based signal definition and manual encoding in C++
- Live signal monitoring and CSV logging via `python-can` and `cantools`
- Optional raw frame inspection using `candump`
- GoogleTest unit tests and Python integration tests
- Clean driver abstraction via `ICanDriver` interface

## Status

Work in progress.

## License

MIT