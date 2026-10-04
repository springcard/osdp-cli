# Contributing

Contributions are welcome. The maintainer sets the roadmap and decides which changes are accepted. There is no guaranteed support response time or release schedule.

## Issues and pull requests

Use English for issues, documentation and pull requests. For a bug report, include the operating system, compiler, tool version, command with secrets removed, expected behavior and observed behavior. Include device manufacturer, model and firmware when relevant. Report security vulnerabilities privately as described in [SECURITY.md](SECURITY.md).
Keep pull requests focused, explain the behavior change and include the relevant validation results. Avoid unrelated formatting changes.

## Validation

Build and run the existing tests locally:

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Python 3 enables process-level tests. On Linux, the serial integration tests use pseudo-terminals and OpenSSL for protected exchanges. State which platform and optional dependencies were used, and identify tests that were skipped. Hardware results should identify the device and firmware tested.

## Licensing

By submitting a contribution for inclusion, you agree to license it under Apache-2.0, unless explicitly agreed otherwise, and confirm that you have the right to submit it. Preserve third-party attribution and license notices. Do not submit credentials, real device keys, private captures or proprietary specifications.
