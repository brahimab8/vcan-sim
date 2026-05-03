# Testing

## Test Tiers

This project uses three test tiers:

- **C++ unit tests** (GoogleTest): validation in isolation
- **C++ integration tests** (GoogleTest): component interaction
- **Python integration tests** (pytest + cantools): DBC signal validation

No hardware or `vcan0` is required.

**CI/CD:** GitHub Actions runs all three test tiers automatically on push. See `.github/workflows/ci.yml`.

## Quick: build and run everything

To build all prerequisites and run all tests:

```bash
mkdir -p build && cd build
cmake ..
# Build C++ test binaries and ensure build prerequisites are ready
cmake --build . -j2
# Run all tests: C++ (unit + integration) + Python DBC
ctest --verbose
```

Notes:
`ctest` runs all registered tests (C++ unit, C++ integration, Python DBC) without building.
If Python venv or dependencies are missing, `ctest` will still run C++ tests; Python tests run only if the environment is configured at CMake configure time.

## C++ unit tests

- **Purpose:** Validate signal encoding primitives and individual ECU component behavior in isolation using injected mocks. Tests verify frame structure, scaling rules, and control flow without external dependencies.
- **Build:**
  ```bash
  cmake --build build --target unit_tests -j2
  ```
- **Run:**
  ```bash
  ctest --verbose --tests-regex "unit"
  ```

## C++ integration tests

- **Purpose:** Validate ECU component integration across multiple tick cycles, run-loop lifecycle, and timer interaction. Tests verify frame sequencing and cycle timing without OS-level CAN or sleep mechanics.
- **Build:**
  ```bash
  cmake --build build --target integration_tests -j2
  ```
- **Run:**
  ```bash
  ctest --verbose --tests-regex "integration"
  ```

## Python integration tests (DBC)

- **Purpose:** Validate that C++ frame encoding produces byte sequences that decode correctly via the industry-standard DBC specification using cantools. Tests verify end-to-end signal value accuracy.
- **Setup (one-time):**
  ```bash
  python3 -m venv venv
  ./venv/bin/pip install -r requirements.txt
  ```
- **Build** (ensures frame_dump is ready):
  ```bash
  cmake --build build -j2
  ```
- **Run:**
  ```bash
  ctest --verbose --tests-regex "python"
  ```
  Or run pytest directly:
  ```bash
  ./venv/bin/python -m pytest -q tests/integration/test_frames.py
  ```

## Mocks

`tests/mocks/` contains `MockCanDriver`, `MockTimer`, and `MockSensor<T>` used to keep tests deterministic and free of OS/hardware.

---

For more context, see [docs/architecture.md](architecture.md).
