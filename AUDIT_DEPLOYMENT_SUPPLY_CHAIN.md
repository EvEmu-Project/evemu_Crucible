# Deployment, Supply-Chain, and Database Audit

All paths in this document are relative to the audited project root.

## DEP-001: Credentials in Deployment Configuration

Severity: High

Status: Confirmed

Evidence:

- `docker-compose.yml:35-39` defines database username, password, and database
  name directly in the compose file.
- `utils/container-scripts/db_init.sh:6-10` repeats a default password and
  uses environment values as command arguments.
- `sql/evedb.yaml:1-6` and the repository root `evedb.yaml:1-6` contain
  database connection fields intended for tooling.
- `src/eve-server/ServiceDB.cpp:150-162` can log a plaintext account password
  during account creation failure.

Impact:

Credentials can enter Git history, process listings, container metadata,
command logs, backups, and CI artifacts. Default credentials are predictable.

Required fix:

- Remove credential values from tracked files and use a secret provider or
  protected runtime environment.
- Do not pass passwords with `-pPASSWORD` on a command line.
- Rotate any credential that has been used with these defaults.
- Add secret scanning to CI and fail on newly introduced credential patterns.
- Ensure generated config files have restrictive permissions and are not
  included in logs or image layers.

## DEP-002: Shell and SQL Interpolation in Initialization

Severity: High

Status: Confirmed

Evidence:

- `utils/container-scripts/db_init.sh:19` leaves host, user, password, and
  database values unquoted in the `mysql` command.
- `db_init.sh:30-34` places environment values directly into `sed` replacement
  expressions.
- `db_init.sh:39-49` writes environment values into YAML without escaping.
- `db_init.sh:54-63` appends caller-controlled region names to YAML.
- `sql/seed_and_clean/delete_character.sh:7-33` interpolates its positional
  argument into multiple SQL statements and passes the password on the command
  line.

Impact:

Special characters can alter shell commands, generated configuration, or SQL.
An operator mistake in the deletion script can destroy unintended data.

Required fix:

- Validate hostnames, ports, database names, users, region names, and numeric
  IDs against strict formats before use.
- Use quoted shell variables and a safe configuration writer.
- Use a database client input file or protected environment mechanism for
  credentials.
- Replace the deletion script with a typed, transaction-aware command that
  requires an explicit confirmation and backup state.

## DEP-003: Unpinned and Weakly Verified Dependencies

Severity: High

Status: Confirmed

Evidence:

- `sql/get_evedbtool.sh:3-34` queries the GitHub `latest` release and downloads
  the selected binary without a signature or checksum verification.
- `Dockerfile:5-22` installs unpinned distribution packages.
- `Dockerfile:34` copies the entire Git directory into the build context.
- `cmake/BuildTinyXML.cmake:16-24` downloads TinyXML over HTTP and verifies
  only an MD5 value.
- `.github/workflows/cmake.yml:21` uses a moving major action reference.

Impact:

Builds are not reproducible and a compromised mirror, release, action, or
dependency can execute code during the build or become part of the server.

Required fix:

- Pin the EVEDBTool version, URL, SHA-256 digest, and platform.
- Verify signatures where available and fail closed on download errors.
- Use HTTPS for all source downloads and replace MD5 with SHA-256 or stronger.
- Pin action references and document dependency update ownership.
- Remove `.git` from the Docker build context unless a reproducible version
  value is explicitly required.

## DEP-004: Runtime Image Contains Build and Debug Tooling

Severity: Medium

Status: Confirmed

Evidence:

- `Dockerfile:2-22` uses the build dependency image as the runtime base.
- The runtime therefore includes compilers, Git, curl, wget, GDB, headers,
  MariaDB client tooling, and other build packages.
- `Dockerfile:43-45` builds a Debug image.
- `docker-compose.yml:21-25` retains an automatic GDB option and interactive
  TTY settings.
- `utils/container-scripts/entry.sh:3-4` starts as root to run `chown` before
  dropping to `evemu`.

Impact:

The attack surface and post-compromise capability of the server container are
larger than necessary. Debugging paths can expose memory, files, or process
state.

Required fix:

- Use a minimal runtime stage with only runtime libraries and assets.
- Build Release or RelWithDebInfo by default.
- Remove GDB and interactive settings from production compose configuration.
- Set ownership during image build and run the server as a non-root user from
  the container entrypoint.
- Add read-only filesystems and narrowly scoped writable volumes where viable.

## DEP-005: CI Does Not Verify Runtime Safety

Severity: Medium

Status: Confirmed

Evidence:

- `.github/workflows/cmake.yml:20-33` checks out, installs dependencies,
  configures, and builds only.
- There is no test, sanitizer, static-analysis, secret-scan, dependency-scan,
  or artifact verification step in this workflow.

Required fix:

- Run `ctest` after the build and fail on test errors.
- Add protocol, authorization, and database unit/integration tests.
- Add AddressSanitizer and UndefinedBehaviorSanitizer jobs where supported.
- Add secret and dependency scanning.
- Publish only artifacts whose dependency digests and build metadata were
  verified.

## DEP-006: Destructive or Non-Reversible Migrations

Severity: Medium

Status: Confirmed

Evidence:

- `sql/migrations/20210911143004-market_decimal_prices.sql:1-12` truncates
  `mktOrders` in both directions.
- `sql/migrations/20240621173914-skillsvanilla.sql:25-42` drops and recreates
  `sklRaceSkills`.
- `sql/migrations/20240614183142-charattributes.sql:6-26` drops multiple
  columns before adding replacement data.

Impact:

A deployment or rollback can destroy live data without a transaction,
backup gate, or verifiable restoration path.

Required fix:

- Classify destructive migrations explicitly and require an operator backup
  confirmation before execution.
- Use additive, copy, validate, and switch steps where possible.
- Make down migrations restore from a backup or document them as unsupported;
  do not label destructive operations as reversible.
- Test migrations against a production-shaped copy before release.

## DEP-007: Unsafe Operator Character Deletion

Severity: Medium

Status: Confirmed

Evidence:

`sql/seed_and_clean/delete_character.sh:7-33` inserts the first command-line
argument directly into a multi-statement SQL script. It also uses fixed
credentials and does not require a transaction, backup, or confirmation.

Required fix:

- Accept only a bounded decimal character ID.
- Use a parameterized client invocation or a stored procedure.
- Require a deliberate confirmation and produce a bounded audit record.
- Wrap related deletions in a transaction where the schema permits it.

## DEP-008: Container Configuration Ordering

Severity: Medium

Status: Confirmed correctness issue

Evidence:

- `utils/container-scripts/start.sh:5-12` runs `db_init.sh` before copying a
  missing configuration into `/app/etc`.
- `db_init.sh:30-34` edits `/src/utils/config/eve-server.xml`, while a mounted
  `/app/etc/eve-server.xml` is not updated when it already exists.

Impact:

A mounted configuration can retain stale database settings while the startup
script reports successful initialization.

Required fix:

- Resolve the effective configuration path before database initialization.
- Render a temporary file, validate it, install it atomically, and never edit
  the image source tree at runtime.
- Do not print rendered secrets.

## Deployment Verification

- Build from a clean context with no `.git` or secret files.
- Inspect the final image for compilers, debuggers, credentials, and shell
  history.
- Verify all downloaded artifact digests.
- Start with non-default credentials and a production-safe role configuration.
- Run migrations against a disposable database and restore from backup.
- Confirm the server process and image server run with bounded resources.
