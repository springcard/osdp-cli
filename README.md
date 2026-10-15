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

### Windows with Visual Studio C++

Open an **x64 Native Tools Command Prompt for Visual Studio** (or a Visual Studio
Developer PowerShell), then configure, build and test the Release configuration:

```bat
cmake -S . -B build-msvc
cmake --build build-msvc --config Release
ctest --test-dir build-msvc -C Release --output-on-failure
```

CMake selects the installed Visual Studio generator automatically. The
executable is `build-msvc/bin/windows/osdp-cli.exe`.

The executable is `build/bin/linux/osdp-cli` on Linux and `build/bin/windows/osdp-cli.exe` on Windows. Subsequent examples assume it is on `PATH`.

## Commands

Linux boards with a manually driven RS485 transceiver can optionally specify `--tx-control <gpio>`, where the argument is a nonnegative Linux sysfs GPIO number (0–65535), not a gpiochip offset. The GPIO is low in receive mode and high while a frame is transmitted. The serial driver must support `TIOCSERGETLSR` so the CLI can wait for actual UART transmitter empty within its deadline before returning to receive. Permission, GPIO initialization or drain errors fail the operation rather than being ignored. Existing invocations without this option are unchanged, including on Windows and USB adapters. Windows rejects an explicitly supplied GPIO option.

```sh
osdp-cli --list --port /dev/ttyS5 --baud 38400 --tx-control 121 --json --progress
osdp-cli --unattended probe --port /dev/ttyS5 --baud 38400 --addr 0 --timeout 1000 --tx-control 121
osdp-cli --tx-control 121 info --port /dev/ttyS5 --baud 38400 --addr 0
```

For regular commands place the option before the command; `list` and unattended commands also accept it among their command options. GPIO control is generic and independent of any controller's wiring, Keyset or product profile. Local break indications during disabled reception are ignored; input accumulated during transmit is discarded before, never after, returning to receive. A controlled commissioning/list cancellation and normal serial close restore receive. Sysfs GPIO output state is not guaranteed to reset on SIGKILL or power loss: electrical pull-downs and system-level recovery remain required. Do not run another serial owner or the electrical loopback test concurrently.

```sh
osdp-cli --help
osdp-cli --version
osdp-cli probe --help
osdp-cli --list --help
osdp-cli sniff --help
osdp-cli info --help
osdp-cli istat --help
osdp-cli out --help
osdp-cli ostat --help
osdp-cli sckmgmt --help
osdp-cli config --help
osdp-cli cmd --help
osdp-cli cmd raw --help
osdp-cli file put --help
osdp-cli cp --help
```

Global options precede the command; command options follow it. Use the executable's help for the available options and defaults.
`--verbose` and `-v` are accepted before the command, between command/subcommand names, or among command options. For `istat`, `ostat`, `out`, `probe` and `list`, they enable protocol frame diagnostics too. With JSON results, traces go to stderr so stdout remains machine-readable. The strict unattended interface continues to reject tracing options.
`--baudrate` is accepted as an alias for `--baud` by every command that uses a serial port.

| Command | Purpose |
| --- | --- |
| `probe` | Read device identification and capabilities, optionally using an explicit candidate key. |
| `--list` / `list` | Scan addresses 0 through 126 and report discovered device identity and capabilities. |
| `info` | Query device metadata through the legacy interface. |
| `istat` | Read input contact states with explicit per-input text or JSON output. |
| `out` | Control one relay output, permanently or for a timed duration, with text or JSON results. |
| `ostat` | Read all output states with explicit per-output text or JSON output. |
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
osdp-cli --list --port COM11 --baud 9600
osdp-cli --json --list --port COM11 --baud 9600
osdp-cli sniff --port COM8 --baud 38400 --show-decoded
```

Discovery uses a 100 ms timeout and one attempt per request by default, so a full silent bus takes about 13 seconds. `--timeout`, `--retries`, `--first-addr` and `--last-addr` may be placed after `--list` to tune the scan. JSON mode writes one document to stdout; diagnostics stay on stderr. A device that answers identification but not capabilities is retained with `"informationStatus":"partial"`.

Angle-bracket values are placeholders; replace them before running a command. A Secure Channel key is 16 bytes encoded as exactly 32 hexadecimal characters. Use disposable test keys when exploring the tool, and never include real keys or unreviewed captures in an issue or pull request.

## File transfer syntax

`file put` accepts one positional input path before, between or after its options, or the existing `--in <path>` form. Quote paths containing spaces. Options following a positional path are parsed normally; specifying both a positional path and `--in` is rejected. The default file type is `firmware`; `--type` remains accepted for compatibility.

```sh
osdp-cli file put ./_firmware/IO_Module_1_2_68_DEBUG.bin --port COM27 --addr 1 --scbk-d --baudrate 38400
osdp-cli file put --port COM27 --addr 1 --scbk-d --baud 38400 --in ./_firmware/IO_Module_1_2_68_DEBUG.bin --type firmware
```

## Input contact states

`istat` sends `osdp_ISTAT` (0x65) without a payload and reads `osdp_ISTATR` (0x49). It reports every returned byte as an input, numbered from 1 in reply order. State 0 is `inactive`, state 1 is `active`, and other values are preserved as `unknown` with their raw value. Active/inactive does not establish whether the physical contact is open or closed; that depends on the device configuration and wiring. An empty ISTATR is a successful report of zero inputs.

```sh
osdp-cli istat --port COM27 --baud 38400 --addr 0
osdp-cli istat --port COM27 --baudrate 38400 --addr 0 --scbk-d
osdp-cli istat --port COM27 --baud 38400 --addr 0 --scbk <32-hex-characters> --json
osdp-cli --json istat --port COM27 --baud 38400 --addr 0
```

Without a key option, the request uses plaintext. `--scbk` and `--scbk-d` are mutually exclusive, and secure requests never fall back to plaintext. `--scsk` is unsupported. `--timeout` (1..60000 ms, default 1000) and `--retries` (1..100 attempts, default 3) may appear before or after `istat`; command options override global values. The total operation is capped at ten minutes and supports cooperative cancellation.

Example text output for a device reporting two inputs:

```text
istat: ok; address=0; authenticated=no
Reply: 0x49 (ISTATR)
Inputs read: 2 (numbered from 1, in reply order)
  Input 1: inactive (raw=0x00)
  Input 2: active (raw=0x01)
```

`--json` emits one JSON object followed by LF, with diagnostics on stderr. The result contains `protocol`, `operation`, `address`, `outcome`, `code`, `authenticated`, `replyCode`, `nakCode`, `inputCount` and `inputs`. Each input has `number`, `raw` and `state`. Reply and NAK codes are numeric or null when absent. `authenticated` is true only after a protected ISTATR was successfully read. The exit status is 0 only for a successful ISTATR; invalid arguments, silence, busy exhaustion, NAK, unexpected replies and communication or authentication failures return 1. Failure results have no input states; an empty list must be interpreted together with `outcome` and `code`.

```json
{"protocol":"osdp","operation":"istat","address":0,"outcome":"succeeded","code":"ok","authenticated":false,"replyCode":73,"nakCode":null,"inputCount":2,"inputs":[{"number":1,"raw":0,"state":"inactive"},{"number":2,"raw":1,"state":"active"}]}
```

## Relay outputs

Use `out` to send `osdp_OUT` (0x68); `ostat` sends the read-only `osdp_OSTAT` (0x66) and expects `osdp_OSTATR` (0x4A). Output indexes follow OSDP: `--output 0` selects the first relay, `--output 1` the second. `--mode on` activates the output and `--mode off` deactivates it. Without `--duration`, the state is permanent and any running timer is cancelled. With `--duration`, the state is temporary and the device resumes its configured permanent state when the timer expires. This does not necessarily invert the temporary state.

```sh
osdp-cli out --port COM27 --baud 38400 --addr 0 --output 0 --mode on --duration 1000
osdp-cli out --port COM27 --baud 38400 --addr 0 --output 0 --mode off
osdp-cli out --port COM27 --baud 38400 --addr 0 --output 1 --mode on --scbk-d --json
osdp-cli ostat --port COM27 --baud 38400 --addr 0 --scbk-d --json
```

`--duration` is in milliseconds, must be a multiple of 100, and ranges from 100 to 6553500. The OSDP timer is encoded as a little-endian 16-bit count of 100 ms units. The timer runs on the device; the CLI returns after its response. Permanent OFF/ON use control codes 1/2, and temporary ON/OFF use 5/6. The [SIA OSDP specification](https://s3-us-west-1.amazonaws.com/ptab-filings%2FIPR2023-00046%2F1014) describes these controls in clause 4.9.

Both commands accept `--scbk`, `--scbk-d`, `--json`, `--timeout` and `--retries` as for `istat`. `ostat` reports each output's zero-based `output` index, one-based relay `number`, raw byte and state (`inactive`, `active` or `unknown`). An empty report indicates zero outputs. `out` accepts either ACK or OSTATR; it explicitly states whether the command was acknowledged and whether output states were actually returned. An ACK proves device acceptance, not the physical position of the relay contacts. Use `ostat` for a separate status read.

`out` sends the control command once. `--retries` only applies to authentication and read-only status requests. Silence or a damaged/unexpected response after OUT can mean the relay changed: the JSON result has `outcome: "unknown"` and the exit status is 2. Query the output state before repeating the control. A definite NAK or BUSY, invalid request, unavailable port or authentication failure returns 1; ACK or OSTATR returns 0. `ostat` uses 0 for a valid OSTATR and 1 on failure. Text output states the requested action and actual returned states; JSON is one object followed by LF, including failures, with diagnostics on stderr. JSON includes `acknowledged`, `statusRead`, `replyCode`, `nakCode`, `outputCount` and `outputs`; `out` also includes `output`, `requestedState` and `durationMs` (null for permanent).

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

Transmission traces are disabled by default, including for `file put`. Normal command results and errors remain visible. For commands using the diagnostic logger, put `--trace` before the command to enable TX/RX hexadecimal traces, or `--verbose` (`-v`) for decoded frames. `--quiet` (`-q`) retains its legacy compact raw-frame trace mode, and `--sc-params` explicitly enables session diagnostics. `--json` alone does not enable transmission traces; combine it with `--trace` for legacy diagnostic frame JSON. File logging only starts when `--log <path>` is explicitly supplied. `sniff` is an explicit capture command with its own display/output options.

```sh
osdp-cli file put ./_firmware/IO_Module_1_2_68_DEBUG.bin --port COM27 --addr 1 --scbk-d --baudrate 38400
osdp-cli --trace file put ./_firmware/IO_Module_1_2_68_DEBUG.bin --port COM27 --addr 1 --scbk-d --baudrate 38400
osdp-cli --verbose --log transfer.log file put ./_firmware/IO_Module_1_2_68_DEBUG.bin --port COM27 --addr 1 --scbk-d --baudrate 38400
```

Base and session keys are omitted from diagnostics. KEYSET frames and decoded key material are redacted, including raw, text and JSON diagnostic outputs. Malformed frames that cannot be safely classified are also suppressed. The `--sc-params` option displays non-secret session parameters.

Command-line keys may remain visible in process listings and shell history. Captures can contain sensitive device and credential data. See [SECURITY.md](SECURITY.md) before sharing logs or reporting a vulnerability.

## Validation

CTest covers key changes, refusal, lost acknowledgements, idempotence, secret cleanup, frame bounds, cryptograms, MAC validation and process-level JSON behavior. Python 3 enables the process tests. Linux serial integration tests exercise the production executable over pseudo-terminals, with an OpenSSL-based peer for protected exchanges, simulated reader and I/O profiles, fragmented frames, EOF, deadlines and cancellation. Install OpenSSL to run the protected scenarios and review output for skipped tests.

Automated tests do not establish interoperability with every physical device. Hardware qualification, including Windows serial ports above COM9, remains outstanding.

## Contributing and license

See [CONTRIBUTING.md](CONTRIBUTING.md) for contributions and local validation. The maintainer sets the roadmap; there is no guaranteed support response time or release schedule.

Copyright 2026 SpringCard SAS, France - www.springcard.com
Licensed under the [Apache License, Version 2.0](LICENSE.txt), except for the third-party files specifically identified in [NOTICE.md](NOTICE.md). The software is provided without warranties as described in the license.
