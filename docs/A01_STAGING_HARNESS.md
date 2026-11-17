# A01 — repeatable, isolated EVEmu Crucible staging harness

**Status:** First implementation in a separate PR; *automated synthetic DB checks ≠ game-client validation*. No live database, local install or existing `master` deployment is changed by the PR.

## 1. What A01 verifies

- A unique Docker Compose project named `evemu-a01-...` with an internal-only MariaDB 11.8 database and isolated named volumes. The MariaDB port is **never published to the host**; the runner has no database host/DSN option.
- A deterministic fixture with two fictional characters (`90010001`, `90010002`), separate corporations (`98010001`, `98010002`), wallet balances, inventory items, mission offers, market orders and an outstanding contract. The fixture uses **only prefixed `a01_*` tables**, never real game tables.
- A five-change synthetic transition, actual MariaDB queries, row-by-row before/after snapshots, wallet delta checks, and protection against a change in the second character's balance or items.
- Optional startup of the real built EVEmu server, whose existing Docker startup script initializes a full *new* database with EVEDBTool. On Windows, the operator then uses the **authentic Crucible client** to exercise real features, with snapshots around each test.
- Linux GitHub Actions runs the isolated fixture against MariaDB. The full-server startup workflow is **manual/opt-in** and does not simulate an EVE client or claim that any gameplay RPC succeeds.

**Boundaries:** The first step intentionally does not try to generate EVE accounts/characters with fabricated columns or install an unsanitized production dump. The operator creates two disposable real test characters after the real server is ready. The snapshot tool is read-only against actual runtime tables, and tracks only specifically selected character/corporation IDs. It does not upload those snapshots automatically.

## 2. Windows 11 operator setup

Install [Docker Desktop with Linux containers](https://docs.docker.com/desktop/setup/install/windows-install/), start Docker Desktop, and open **PowerShell in the root of the A01 branch checkout** (where `docker-compose.a01.yml` lives). Python 3.10+ should be on your PATH. Do **not** run `docker compose up` from the original `docker-compose.yml` for these tests.

**Step 1 — create a separate test DB and verify the harness:**

```powershell
python tools/staging/a01.py init
python tools/staging/a01.py up
python -m unittest discover -s tools/staging/tests -v
python tools/staging/a01.py selftest
python tools/staging/a01.py status
```

`selftest` prints **PASS** and saves three local JSON files under `.a01/runs/`: before, after, and diff. It verifies exactly one changed record for each of character wallet, item, mission offer, market order and contract. Character B and both corporation wallets remain unchanged. This is an *integration test of the snapshot harness*, not a test of real EVEmu reward/contract code.

**Step 2 — run the real server in the same isolated project (optional):**

```powershell
python tools/staging/a01.py up --server
python tools/staging/a01.py logs
```

The server image builds the current branch and initializes a fresh full SDE through the existing EVEDBTool startup procedure. **Check `logs` for successful server initialization before connecting**; Docker reporting `started` is not proof that the game is ready. The server uses only the new project's internal DB. Ports **26000** (game) and **26001** (image) bind to `127.0.0.1` on your computer, so the client must run on the **same computer** unless network rules are explicitly configured later. Do not point a public server, LAN port-forward or live game client at this test stack.

Start the *compatible Crucible 1.6.5 / build 360229 client* configured to connect to `127.0.0.1:26000`. Create **two new disposable accounts/characters** in this isolated database, ideally with different NPC/player corporations if the client workflow allows it. Record their numeric character IDs and corporation IDs in your private test notes. Do not paste passwords, auth tokens, full DB dumps or client session secrets into issues or chat.

**Step 3 — before/after snapshots for a specific game test:**

```powershell
# Replace these four example IDs with the two disposable test identities.
python tools/staging/a01.py snapshot --mode game --label before-market --character-a 90010001 --character-b 90010002 --corporation-a 98010001 --corporation-b 98010002 --build-sha YOUR_BUILD_SHA

# In the Crucible client, perform ONE action (e.g. buy an item for character A).

python tools/staging/a01.py snapshot --mode game --label after-market --character-a 90010001 --character-b 90010002 --corporation-a 98010001 --corporation-b 98010002 --build-sha YOUR_BUILD_SHA

# Insert the two actual generated .a01/runs/*.json paths below:
python tools/staging/a01.py compare --before .a01/runs/BEFORE.json --after .a01/runs/AFTER.json --expect-b-isk 0
```

The comparator reports exact changed primary keys and wallet deltas; e.g. if you expect character A's wallet to decrease by 100 ISK, add `--expect-a-isk -100`. `--fail-on-removal` is an optional guard for scenarios where no rows should disappear. It intentionally **does not** interpret a valid item transfer as duplication or a particular mission payout as correct without your explicit expected outcome. Inspect the diff and correlate it with the server log and the client's UI.

**Recommended first real tests:** (1) buy one ordinary item, (2) transfer one stack between your own hangar/cargo, (3) accept and complete a disposable agent mission, (4) create/accept a minimal-ISK private contract with character B. Start each from a new snapshot pair and inspect asset/wallet records. **Do not test valuable items or actual accounts in A01.**

## 3. What to send back for verification

You can paste this short sanitized summary (or attach the A01 diff JSON after removing any data you do not wish to share):

```text
A01 build SHA (from BUILD-INFO.txt or GitHub Actions):
Docker project name: evemu-a01-...
Windows client build/version:
Action tested: market purchase / inventory move / mission / contract
Before -> after wallet amounts for A and B:
Before -> after affected itemID/typeID/quantity/locationID:
Before -> after offerID / orderID / contractId and state:
Expected behavior and actual behavior:
Relevant warnings/errors (no credentials):
```

We can use that evidence to add targeted assertions to later A02/A03/A05 tests and troubleshoot any real EVEmu state discrepancy. No live MariaDB connection is required to help from GitHub.

## 4. Stopping and deleting staging safely

```powershell
python tools/staging/a01.py stop         # stops containers, preserves the test DB volume
python tools/staging/a01.py status       # inspect project name
# Irreversibly DELETE this disposable project's test DB and caches only:
python tools/staging/a01.py destroy --confirm evemu-a01-YOUR_EXACT_SESSION_NAME
```

The `destroy` confirmation must match the project's exact generated name. Session metadata, snapshots and server logs stay under `.a01/` and are **Git-ignored**. Delete or archive them locally when finished. Do not commit private snapshots or generated server configuration.

## 5. CI gates and acceptance

`/.github/workflows/staging-a01.yml` runs Python `unittest` plus an actual MariaDB container with synthetic `a01_*` data for PRs touching the harness. A separate `workflow_dispatch` input `full_server_smoke=true` builds/starts the full server in a disposable project, waits for its local TCP game port and captures startup logs. The server test can be run from Actions once this workflow has landed on the default branch.

| Gate | Automation | Pass criteria |
| --- | --- | --- |
| Safety / isolation | Python unit tests | Production project names rejected; only A01 Compose used |
| Fixture database | MariaDB 11.8 on Linux CI | Exactly five intended row changes; character B unaffected |
| Snapshot repeatability | Python + real SQL | Stable primary-key representation and expected wallet delta |
| Real server startup | Manual opt-in CI or Windows Docker | Fresh SDE install and listening game port 26000; no server exit |
| Client purchase / inventory | Operator, Windows 11 | Exact wallet, item ownership/location/quantity changes, no double effect |
| Client mission / contract | Operator, Windows 11 | Once-only expected transitions, no unauthorized cross-character effects |
| Destruction safety | Script | `destroy` requires exact isolated project name; no host DB mounted |

**A01 should remain OPEN until the real-server startup and at least one Windows Crucible client test pass.** CI source/fixture success alone is not the full A01 acceptance test; this distinction is intentional.
