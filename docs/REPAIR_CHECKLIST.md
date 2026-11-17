# EVEmu Crucible — living repair checklist

**Updated:** 17 November 2026 · **Branch:** `master` · **Server:** 0.8.7-Rizonx

Status entries apply to the **named workstream** only. Source/build CI does not certify every Crucible gameplay path. Operator declarations are documented separately from CI evidence.

## Phase A — data safety and correctness

- [x] **A01 — isolated staging / client integration and verification** — **RESOLVED, OPERATOR TESTED AND ACCEPTED (17 Nov 2026)**. [PR #32 merged](https://github.com/xrizonx/evemu_Crucible/pull/32) · [MariaDB fixture CI pass](https://github.com/xrizonx/evemu_Crucible/actions/runs/44072201705) · [master Windows/Linux/Docker CI pass](https://github.com/xrizonx/evemu_Crucible/actions/runs/44073936776). The operator reported all A01 live tests passed; raw client evidence was not archived. Completed A01-only fixture files were retired; see [closure record](A01_STAGING_HARNESS.md).
- [ ] **A02 — mission-bound offline courier cargo provenance** — pending.
- [ ] **A03 — transactional contract escrow, accept and completion** — pending.
- [ ] **A04 — contract inbox/corporation-contract completion** — pending.
- [ ] **A05 — industry job exactly-once delivery and rollback** — pending.
- [ ] **A06 — inventory/POS permissions and SQL graph integrity** — pending.
- [ ] **A07 — trading/reprocessing/repair safety** — pending.
- [ ] **A08 — one-time NPC market order historical cleanup** — verify deployment separately.
- [ ] **A09 — remaining ship warp-speed data verification** — verify migration separately.

## Independent gameplay regression tracking

[Full Crucible client checklist](CRUCIBLE_CLIENT_GAMEPLAY_VERIFICATION_CHECKLIST_2026-11-16.md) remains valid for tests that are **not part of A01 acceptance**. Do not mark agent, industry, contracts, POS, warp, weapons or mail as fully passed based only on closing A01.

## Process and evidence

For future work: implement changes on an isolated branch; run Windows/Linux/Docker/CTest; record actual client/server build and SQL results; request operator sign-off; then update this checklist and merge. Current `master` retains the C++ test suite and standard build workflows. A disposable DB harness for subsequent issues can be restored from [PR #32](https://github.com/xrizonx/evemu_Crucible/pull/32) if required.
