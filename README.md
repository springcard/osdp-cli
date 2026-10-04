# osdp-cli

Command-line utility for OSDP devices connected over an RS-485 serial link. It runs on Windows and Linux and provides device queries, Secure Channel commissioning, operational commands and traffic inspection.

The project uses its own OSDP implementation and does not depend on an external libosdp library. Its AES implementation is derived from tiny-AES-c.
DO NOT USE IT WITH SENSITIVE KEY MATERIAL.

## Build

Requirements: a C99 compiler, CMake 3.16 or later and a build tool. Ninja is used in the examples. Linux also requires POSIX threads. Windows supports MSVC or a MinGW-w64 toolchain, such as the MSYS2 UCRT64 environment.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

With MSVC, run these commands from a developer shell. With a multi-configuration generator, select the configuration when building and testing, for example `--config Release` and `-C Release`.

The executable is `build/bin/linux/osdp-cli` on Linux and `build/bin/windows/osdp-cli.exe` on Windows. Subsequent examples assume it is on `PATH`.

## Commands

```sh
osdp-cli --help
osdp-cli --version
osdp-cli probe --help
osdp-cli sniff --help
osdp-cli info --help
osdp-cli sckmgmt --help
osdp-cli config --help
osdp-cli cmd --help
osdp-cli cmd raw --help
osdp-cli file put --help
osdp-cli cp --help
```

Global options precede the command; command options follow it. Use the executable's help for the available options and defaults.

| Command | Purpose |
| --- | --- |
| `probe` | Read device identification and capabilities, optionally using an explicit candidate key. |
| `info` | Query device metadata through the legacy interface. |
| `sckmgmt` | Change the Secure Channel Base Key and verify a fresh session with the target key. |
| `sniff` | Inspect traffic, optionally decrypting Secure Channel exchanges with a known key. |
| `cmd` | Send buzzer, LED, relay, text or raw commands. |
| `config` | Change communication parameters using COMSET. |
| `file put` | Transfer a file to a device. |
| `cp` | Run a basic Control Panel loop. |

COMSET and file transfer are legacy paths with incomplete hardware qualification. Peripheral Device simulation is not provided as a supported command. Device-specific behavior must be validated on the intended hardware and firmware.

```sh
osdp-cli probe --port /dev/ttyUSB0 --baud 9600 --addr 0
osdp-cli probe --port COM11 --baud 9600 --addr 0 --key <32-hex-characters>
osdp-cli sniff --port COM8 --baud 38400 --show-decoded
```

Angle-bracket values are placeholders; replace them before running a command. A Secure Channel key is 16 bytes encoded as exactly 32 hexadecimal characters. Use disposable test keys when exploring the tool, and never include real keys or unreviewed captures in an issue or pull request.

## Key changes

`sckmgmt` requires `--set-key` and exactly one initial authentication mode: an explicit `--scbk` or `--scbk-d`. It rejects `--scsk`. After an authenticated acknowledgement, it closes the port and establishes a fresh Secure Channel using the target key. Verification always runs; `--verify` remains accepted for compatibility. Frame and key logging are disabled for this command.

```sh
osdp-cli sckmgmt --port COM11 --baud 9600 --addr 0 --scbk <current-key> --set-key <target-key>
```

Exit status is `0` for a verified change, `1` for a failure before the write or an authenticated refusal, and `2` when the device may have changed its key without proof of success. Syntax errors return a nonzero status. After an uncertain result, authenticate with the target key before attempting another write. The command does not automatically repeat KEYSET.

## Unattended mode

`--unattended` (shortcut `-u`) selects a separate, strict JSON interface for automation. Place the flag first, immediately before `probe`, `authenticate` or `set-key`; it takes no value. Every request requires explicit port, baud rate, address (`0` to `126`) and total timeout (`1` to `60000` milliseconds). Legacy diagnostic JSON is a separate output format and does not follow this interface.

```sh
osdp-cli --unattended probe --port COM11 --baud 9600 --addr 0 --timeout 5000
osdp-cli -u authenticate --port /dev/ttyUSB0 --baud 9600 --addr 0 --scbk <current-key> --timeout 5000
osdp-cli --unattended set-key --port COM11 --baud 9600 --addr 0 --scbk <current-key> --set-key <target-key> --timeout 5000
```

The process writes exactly one UTF-8 JSON object followed by LF to standard output. Diagnostic text, when present, goes to standard error. The required fields are `protocol`, `operation`, `address`, `outcome`, `code`, `authenticated` and `verifiedTargetKey`. Device identification and capability fields are included when available. The operation name for `set-key` is `set_key` in JSON.

| Exit status | Outcome | Meaning |
| --- | --- | --- |
| `0` | `succeeded` | The requested result was proved. |
| `1` | `failed` | The request failed without an uncertain key change. |
| `2` | `unknown` | A key change may have taken effect without a verified result. |

Result codes are `ok`, `invalid_request`, `port_unavailable`, `no_response`, `communication_error`, `authentication_failed`, `unsupported_device`, `verification_failed`, `verification_timeout`, `outcome_unknown` and `cancelled`.

Unattended `probe` accepts no key and reads plain identification and capabilities. `authenticate` requires an explicit `--scbk` and proves protected ID/CAP responses. `set-key` also requires `--set-key`; it first authenticates with the target key and avoids rewriting it if it already works. Only a definite target-key authentication failure permits an attempt with the current key. After one acknowledged KEYSET, the tool reconnects and verifies protected ID/CAP responses with the target key. All manufacturers and models are accepted; protocol and security capability checks still apply.

Unattended mode rejects unknown, repeated and tracing options, and never falls back to an implicit key or SCBK-D. Challenges use operating-system cryptographic randomness. The serial port is held exclusively for the operation. Cooperative cancellation closes the port and clears key buffers. A forced termination may prevent a final JSON result: automation must treat a potentially interrupted write as uncertain and reconcile the target key.

## Diagnostics and security

Base and session keys are omitted from diagnostics. KEYSET frames and decoded key material are redacted, including raw, text and JSON diagnostic outputs. Malformed frames that cannot be safely classified are also suppressed. The `--sc-params` option displays non-secret session parameters.

Command-line keys may remain visible in process listings and shell history. Captures can contain sensitive device and credential data. See [SECURITY.md](SECURITY.md) before sharing logs or reporting a vulnerability.

## Validation

CTest covers key changes, refusal, lost acknowledgements, idempotence, secret cleanup, frame bounds, cryptograms, MAC validation and process-level JSON behavior. Python 3 enables the process tests. Linux serial integration tests exercise the production executable over pseudo-terminals, with an OpenSSL-based peer for protected exchanges, simulated reader and I/O profiles, fragmented frames, EOF, deadlines and cancellation. Install OpenSSL to run the protected scenarios and review output for skipped tests.

Automated tests do not establish interoperability with every physical device. Hardware qualification, including Windows serial ports above COM9, remains outstanding.

## Contributing and license

See [CONTRIBUTING.md](CONTRIBUTING.md) for contributions and local validation. The maintainer sets the roadmap; there is no guaranteed support response time or release schedule.

Copyright 2026 SpringCard SAS, France - www.springcard.com
Licensed under the [Apache License, Version 2.0](LICENSE.txt), except for the third-party files specifically identified in [NOTICE.md](NOTICE.md). The software is provided without warranties as described in the license.
