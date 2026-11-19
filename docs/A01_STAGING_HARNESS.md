# A01 — staging and verification: RESOLVED (operator signed off)

**Status:** ✅ **Closed / accepted 17 November 2026**. The maintainer reported all A01 tests passed in the live existing-character environment and requested integration/cleanup.

## Delivered and validated

- [PR #32](https://github.com/xrizonx/evemu_Crucible/pull/32) merged on 17 November 2026 at `7f600be1fa06191707f4352d46f2240bfe3d2159`.
- [MariaDB 11.8 fixture CI — pass](https://github.com/xrizonx/evemu_Crucible/actions/runs/44072201705): deterministic two-character synthetic table fixture, wallet/asset/journal diffs and scope checks.
- [Post-merge master build — pass](https://github.com/xrizonx/evemu_Crucible/actions/runs/44073936776): Windows UCRT64 GCC, Linux GCC and Docker.
- **Operator acceptance:** All A01 live tests were reported passed by the maintainer on 17 November 2026. That declaration closes A01; private logs, in-game captures and SQL comparisons were not independently supplied for repository archiving. The optional GitHub Actions full-server smoke remained skipped and is **not** claimed to have run.
- Acceptance applies **only to the A01 testing milestone**, not to every untested game subsystem.

## Post-acceptance housekeeping

The A01-specific disposable Docker Compose stack, Python runner/unit checks, synthetic `a01_*` SQL fixtures and dedicated workflow were removed from **current** `master` after sign-off, at the maintainer's request. The standard production `docker-compose.yml`, EVEmu runtime source, real database and general CTest suite were not changed.

The original A01 harness source remains recoverable from the [PR #32 history](https://github.com/xrizonx/evemu_Crucible/tree/74d4a328ec516abcc8367903ffc6c90d692249df) when a future feature needs a fresh isolated staging campaign. No command in this housekeeping change deletes Docker volumes or the local operator's `.a01/` snapshots.

**Local-only cleanup:** On the operator's machine, previously created `evemu-a01-...` Docker Compose projects and private `.a01/` logs/snapshots must be reviewed and removed separately if no longer required. Never run a broad `docker volume prune` or delete the production MariaDB data directory.

See the [repair checklist](REPAIR_CHECKLIST.md) for A01's completion and remaining tasks.
