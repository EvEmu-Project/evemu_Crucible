# EVEmu database tools

<p align="center">
  <img src="../docs/assets/readme/database-tools-banner.jpg" alt="Database and Tools — MariaDB, EVEDBTool and migrations" width="100%" />
</p>

[Windows setup and server installation](../README.md#windows-11-and-windows-10-x64-setup) · [Main project README](../README.md)

The `sql/` directory is included in the Windows and Linux release packages. It contains the base database data, ordered schema migrations, and the matching EVEDBTool executable.

## Included tools

- Windows: `evedbtool.exe`
- Linux x86_64: `evedbtool`
- Configuration template: `evedb.yaml.example`
- Schema changes: `migrations/`
- Base database files: `base/`

The release workflow builds EVEDBTool from the upstream 0.0.6 source and replaces its market-seed command with the maintained implementation in `tools/evedbtool/command_seed.go`. Both Windows and Linux packages therefore contain the same patched market-seeding behaviour. The standalone `get_evedbtool.bat` and `get_evedbtool.sh` scripts do the same source build locally and require Git plus Go 1.23 or newer.

## Configure the database tool

1. Stop the EVEmu server before changing the schema.
2. Make a backup of your database.
3. Open a terminal in this `sql` directory.
4. Copy `evedb.yaml.example` to `evedb.yaml`.
5. Edit the local `evedb.yaml` with your MariaDB/MySQL host, port, username, password, and database name. The example uses database name `EvE`; change it if your database has a different name.
6. Keep `evedb.yaml` private. It is ignored by Git and should never be committed.

Windows PowerShell:

```powershell
Copy-Item .\evedb.yaml.example .\evedb.yaml
.\evedbtool.exe --version
.\evedbtool.exe status
```

Linux:

```sh
cp evedb.yaml.example evedb.yaml
chmod +x ./evedbtool
./evedbtool --version
./evedbtool status
```

Run `status` first. It reports which migration files are recorded in the database migration table. Do not run `install` against an existing server database.

## Configure and reseed the market

The `seed-regions` and `seed-saturation` settings in `evedb.yaml` control which regions are seeded and the percentage of stations selected in each region. The default is Derelik at 80%; add region names or numeric region IDs to seed additional regions.

The patched `seed` command generates SQL from the same market rules as the repository's market seed scripts:

- Published market items are seeded across the selected stations.
- Charge/ammunition category items receive 550,000 units; other standard items receive 550.
- Officer, Deadspace, Faction and Storyline meta-group equipment receive 10 units each where no sell order already exists, regardless of inventory category.
- Prices are guarded against null/zero base prices and zero/negative station security.
- A normal first seed uses the existing `SEED_MARKET` migration marker. To explicitly rebuild the configured regional markets, use `-force`.

Windows PowerShell (from the `sql` directory):

```powershell
.\evedbtool.exe seed -dryrun
.\evedbtool.exe seed -force -dryrun
# After reviewing the SQL, backing up the DB, and stopping the server:
.\evedbtool.exe seed -force
```

Linux:

```sh
./evedbtool seed -dryrun
./evedbtool seed -force -dryrun
# After reviewing the SQL, backing up the DB, and stopping the server:
./evedbtool seed -force
```

**Important:** `seed -force` deletes existing NPC corporation sell orders in the configured regions for the market item categories managed by the seed, then rebuilds them at a newly selected station set. It retains player-owned orders. It is a destructive market refresh: stop the server and take a verified database backup first. Always inspect `-dryrun` output before applying it. Each forced run is recorded under a unique seed-migration ID so it can be repeated on demand.

## Applying migrations manually with a SQL client

Manual SQL application is supported as an operator-managed process:

1. Stop the server and take a verified database backup.
2. Run `evedbtool status` and save the output so you know which migrations are pending.
3. Open each pending file in `migrations/` in filename order (oldest first).
4. Execute only the SQL under its `-- +migrate Up` section. Do not execute the `-- +migrate Down` section; those statements are rollback instructions.
5. Check the SQL client result after every file. Stop immediately if any statement fails; do not continue to later migrations.
6. After, and only after, every pending migration has been successfully applied and verified, run `evedbtool skip` once to record the migration versions in EVEDBTool's `migrations` tracking table. **This command does not execute SQL**; it only advances the recorded migration level. Never use it to bypass migrations that have not been applied.
7. Run `evedbtool status` again and confirm all listed migrations show as applied before restarting the server.

If a migration partially fails, restore the backup or resolve the partial changes deliberately before proceeding. Do not use `skip` to hide a failed or incomplete migration.

> Important: Some migrations make destructive schema changes (for example, dropping obsolete columns). Review each pending migration and confirm it matches your database before executing it. The tool's `status` output is a version check, not a database compatibility assessment.

## EVEDBTool commands

```
evedbtool status       # Show migration status
evedbtool up -dryrun  # Print pending SQL without applying it
evedbtool up          # Apply pending migrations automatically (not required for manual workflow)
evedbtool skip        # Record migrations as applied WITHOUT executing SQL
```

Use `up` only if you intentionally want EVEDBTool to execute the migrations itself. For the manual workflow above, use your SQL client for the SQL and `skip` only after successful completion.

## Adding schema changes

All schema changes must be added as reversible, timestamped files in `migrations/`, with both `-- +migrate Up` and `-- +migrate Down` sections. Never edit the compressed files in `base/` for ordinary schema changes.
