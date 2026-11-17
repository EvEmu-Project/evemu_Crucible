# A01 staging-only fixtures

`a01_fixture.sql` and `a01_fixture_transition.sql` create/change **only** `a01_*`
tables in the isolated database owned by the `evemu-a01-...` Docker project.
These tables intentionally resemble selected runtime columns but are **not**
EVEmu's actual `chrCharacters`, `entity`, `agtOffers`, `mktOrders` or
`ctrContracts`. They prove that real MariaDB, snapshot collection and diff
assertions are wired correctly; they **do not** prove gameplay or transactional
safety in the actual C++ services.

Run `python tools/staging/a01.py init`, `up`, then `selftest` on Docker Desktop
(Windows) or Docker Engine (Linux). For real engine/client tests, use the
optional `up --server` path and create two disposable characters from the
Crucible client after the server has installed its SDE. Never import an
unsanitized production database into this project or its CI workflow.
