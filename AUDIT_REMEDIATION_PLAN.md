# Server Audit Remediation Plan

This plan converts the findings in the audit documents into small, ordered
changes. It intentionally does not prescribe a broad rewrite of the server.

## Phase 0: Containment

Do these actions before exposing the server to untrusted clients.

1. Set `autoAccountRole` to zero in the production configuration.
2. Remove default database credentials from tracked configuration and rotate
   every credential that used the defaults.
3. Disable placebo authentication outside an isolated development profile.
4. Remove or role-gate developer, QA, dungeon, and editor services.
5. Restrict the image server to a trusted interface until its HTTP limits are
   implemented.
6. Take a database backup before any migration or destructive data repair.

These changes do not alter valid game service names or packet layouts.

## Phase 1: Protocol Containment

Implement `NET-001` through `NET-005` before handler-level authorization work.

- Reject zero and oversized frame lengths before body buffering.
- Add named limits for frames, pending bytes, queued packets, nesting depth,
  saved objects, strings, buffers, and containers.
- Replace unchecked marshal iterator reads with a bounded reader.
- Bound decompression output and reject empty input.
- Treat null named arguments as an empty dictionary.
- Delete each processed packet exactly once.
- Repair exception-safe reset of `m_canThrow`.
- Add connection, handshake, idle, and image HTTP timeouts.

The first implementation should preserve the existing valid packet path and
return a normal protocol failure for invalid input.

## Phase 2: Ownership and Memory Safety

Implement `MEM-001` and `MEM-002` as isolated changes with sanitizer coverage.

1. Document ownership transfer for `PyRep`, tuple, list, dictionary, packet,
   and call-stream objects.
2. Add focused container cleanup tests before changing destructors.
3. Repair container cleanup and packet cleanup together.
4. Repair malformed decode cleanup for every consumed root object.
5. Replace the fixed log buffer with bounded formatting.
6. Add width limits to configuration parsing and redact production payloads.

Do not combine this phase with a service authorization refactor. The existing
reference-count model is inconsistent, so a partial cleanup could introduce a
use-after-free while attempting to remove leaks.

## Phase 3: Authentication Migration

Implement `AUTH-001`, `AUTH-002`, and `AUTH-003` with a migration runbook.

- Define the new authenticated transport negotiation and downgrade policy.
- Add a versioned, salted password KDF record.
- Dual-read only the legacy hash during a bounded migration window.
- Rehash after successful verification and never write plaintext again.
- Use constant-time verification and generic failure responses.
- Add per-account and per-address failure throttling with operator recovery.
- Remove legacy columns only after migration completion and backup validation.

Keep the public service and packet interfaces unchanged after authentication.

## Phase 4: Authorization Boundaries

Apply ownership checks in this order:

1. Character selection, preparation, cancellation, and deletion.
2. Corporation wallet balances, journals, divisions, and withdrawals.
3. Mail reads, labels, state changes, moves, and deletion.
4. Chat membership, passwords, ownership, moderation, configuration, and
   message access.
5. Developer, QA, dungeon, keeper, and bound editor services.

Use a shared authorization helper only for rules that have the same subject,
actor, and role semantics. Keep handler-specific rules in the handler when
they differ. Every database mutation must repeat the ownership predicate at
the database boundary.

## Phase 5: SQL and Input Safety

Implement `DATA-002` and the input limits alongside Phase 4.

- Prefer parameterized database calls.
- If the wrapper cannot bind a value, use the connection-aware escape helper
  and enforce a maximum field length first.
- Keep dynamic identifiers in internal allowlists.
- Escape or remove user-controlled regular-expression queries.
- Cap all client lists before building a query.
- Use transactions for multi-row mail and character operations.
- Never log generated SQL containing credentials or message contents.

## Phase 6: Image Server Hardening

Implement `IMG-001` without changing the public image URL shape.

- Limit request headers and request duration.
- Parse category, ID, size, and trailing bytes strictly.
- Bound image upload and file read sizes.
- Check open, seek, read, write, and close results.
- Reject an empty base path during startup validation.
- Add a concurrent connection limit and safe fallback URL configuration.

## Phase 7: Deployment and Supply Chain

Implement `DEP-001` through `DEP-008` after source containment is in place.

1. Pin actions, OS packages, source archives, and EVEDBTool by digest.
2. Use HTTPS and SHA-256 or signature verification for downloads.
3. Remove `.git` and secrets from build contexts and image layers.
4. Split build and runtime images; remove compilers and debuggers from the
   runtime image.
5. Render effective configuration atomically at runtime without editing the
   image source tree.
6. Replace shell SQL interpolation with validated, parameterized operations.
7. Add backup gates and explicit policy for destructive migrations.
8. Add tests, sanitizers, secret scanning, dependency scanning, and artifact
   verification to CI.

## Verification Order

Run the following after each compatible change set:

1. Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
2. Build: `cmake --build build --parallel`
3. Test: `ctest --test-dir build --output-on-failure`
4. Run malformed packet, authorization, SQL quoting, and image HTTP tests.
5. Run sanitizer builds for protocol and representation ownership changes.
6. Run the container build with a clean context and inspect its contents.
7. Test migrations on a disposable copy and verify backup restoration.

## Exit Criteria

- No Critical or High finding remains without a tested mitigation.
- Invalid input is rejected before unbounded allocation or database mutation.
- Cross-account and cross-role access tests fail closed.
- No password, hash, message body, or database credential appears in logs.
- Valid wire packets and service names remain compatible.
- CI runs build, tests, sanitizers where supported, and security checks.
- Deployment uses non-default credentials and an explicit safe role policy.

Source implementation should begin only after this document set is reviewed
as the single remediation backlog.
