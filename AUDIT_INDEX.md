# EVEmu Server Audit

Status: static audit write-up. Source fixes are not applied yet.

Audit date: 2026-08-10

## Scope

The audited source root is:

`C:\Users\Metal\Desktop\Socius\evemu_Crucible`

The review covers the active C++ server path, its common protocol and
database code, the active image server, and deployment tooling. Generated,
vendored, build-output, and unrelated tool trees were not treated as server
implementation targets.

The active runtime path is the CMake chain `eve-core` to `eve-common` to
`eve-server`. The game TCP listener and image server are active. The legacy API
server is not registered by the active `main` path or target graph.

## Constraints

- Preserve existing service names, method signatures, and valid wire packets.
- Preserve database compatibility and use staged migrations where required.
- Disable unsafe defaults without removing an explicit development mode.
- Do not log passwords, authentication material, or message contents.
- Add bounded behavior before allocation, decompression, or database work.
- Treat every finding as requiring a regression test before source changes are
  considered complete.

## Documents

- `AUDIT_PROTOCOL_MEMORY_SAFETY.md` covers network parsing, memory ownership,
  logging memory safety, and the image HTTP listener.
- `AUDIT_AUTH_AUTHZ_DATA.md` covers authentication, authorization, IDOR, SQL
  injection, and sensitive data handling.
- `AUDIT_DEPLOYMENT_SUPPLY_CHAIN.md` covers containers, credentials, scripts,
  dependencies, migrations, and CI.
- `AUDIT_REMEDIATION_PLAN.md` defines the order of implementation and
  verification.

## Finding Index

Severity uses Critical, High, Medium, and Low. Findings marked confirmed are
supported by a complete source path in this pass. A validation item is a
required follow-up, not a claim that the issue is exploitable as written.

### Critical

- `AUTH-001` Plaintext and weak password handling.
- `AUTH-002` Placebo crypto provides no authenticated encryption.
- `AUTHZ-001` New accounts receive the full development role by default.
- `AUTHZ-002` Character selection and deletion lack account ownership checks.
- `AUTHZ-003` Corporation wallet operations lack role checks.
- `DATA-001` Mail operations accept arbitrary message identifiers.
- `DATA-002` Chat and mail fields reach SQL without escaping.
- `NET-001` Partial and oversized frames are buffered without an early cap.
- `NET-002` Marshal lengths and container counts are not bounds checked.
- `NET-003` Decompression has no output limit and accepts empty input.
- `MEM-001` Network packet and Python representation ownership leaks.
- `MEM-002` Log formatting can write beyond its fixed allocation.

### High

- `AUTHZ-004` Developer, QA, and dungeon editor services are remotely exposed.
- `CHAT-001` Chat membership, password, owner, and message checks are missing.
- `NET-004` Malformed calls can crash on null named arguments or null state.
- `NET-005` Connection, frame, queue, and decompression resources lack caps.
- `IMG-001` The image HTTP parser has unbounded input and unsafe parsing.
- `LOG-001` Credentials, PII, and message content can enter legacy logs.
- `DEP-001` Deployment files contain default database credentials.
- `DEP-002` Deployment scripts expose credentials and interpolate shell input.
- `DEP-003` Build and download dependencies are not sufficiently pinned.

### Medium

- `DEP-004` The runtime image includes build and debugger tooling.
- `DEP-005` CI builds only and does not run tests or security checks.
- `DEP-006` Several migrations intentionally destroy or replace data.
- `DEP-007` The operator character deletion script interpolates its argument.
- `DEP-008` Container configuration initialization has an ordering defect.
- `AUTH-003` Password hash migration and login failure controls need a defined
  compatibility policy.

## Evidence Boundary

This write-up records source evidence and safe remediation requirements. No
source, configuration, database, or generated file was modified by this audit
session. Pre-existing worktree modifications were left untouched. Builds,
tests, and live network tests have not been run yet.

The outer Git repository has no commits, so Git history cannot be used to
separate old findings from recent changes. The inner project directory is
currently uncommitted in that outer repository.

## Implementation Rule

Implement the shared containment and authentication prerequisites first. Then
apply ownership and SQL fixes by service family. Keep each change small enough
to build and test independently. The detailed sequence is in
`AUDIT_REMEDIATION_PLAN.md`.
