# Security policy

## Reporting a vulnerability

Use the repository's GitHub Security tab and its **Report a vulnerability** action when private vulnerability reporting is enabled. If this action is unavailable, contact the repository owner through a private channel listed on their public profile. Do not post vulnerability details or secrets in a public issue. There is no guaranteed response time.

Include the affected version or commit, operating system, device model and firmware if relevant, impact and minimal reproduction steps. Use disposable test keys and redact identifying information from captures.

## Supported versions

Security fixes are developed against the current default branch. Older versions do not have a guaranteed maintenance period. Review changes and validate them against your hardware before deployment.

## Handling keys and captures

Keys supplied as command-line arguments may be visible in process listings and shell history. Use a controlled environment and avoid storing real keys in scripts, issues or examples.

Commissioning commands disable frame and key logging. Other diagnostic commands redact KEYSET frames and decoded key material, and do not print base or session keys. Captures may still contain device identifiers, credential data and other sensitive information; review them before sharing.

A key change with an uncertain outcome must be reconciled by authenticating with the target key before attempting another write. A forced process termination may prevent a final result from being emitted.
