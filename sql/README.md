# DATABASE SETUP:
Author: James

EVEDBTool manages the EVEmu database including installing, updating and reverting schema changes.

## Obtaining EVEDBTool

To download the latest release of EVEDBTool, simply run ./get_evedbtool.sh (Linux x64)

## Installing the DB

To install the EVEmu database, run './evedbtool install'

## Disposable database verification

On Windows PowerShell, run the isolated install, backup/restore, migration,
and character-deletion checks with:

```
.\verify_database.ps1 -Port 13306
```

The runner creates its temporary MariaDB data under this project, removes it
after a successful run, and refuses work directories outside the workspace.

## EVEDBTool Usage
```
Usage: evedbtool [--version] [--help] <command> [<args>]

Available commands are:
    down       Undo a database migration
    install    Installs the base database and migrates to the most recent version available.
    new        Create a new migration
    redo       Reapply the last migration
    skip       Sets the database level to the most recent version available, without running the migrations
    status     Show migration status
    up         Migrates the database to the most recent version available

```

## Configuration

EVEDBTool is configured from the `evedb.yaml` file. This file must be in your current working directory for the tool to work. The tool will create a new default configuration file for you on first run if it is not already present.

## Other Architectures / Building Manually

If the default binary does not work for you due to having a different system architecture or OS, you may want to build EVEDBTool manually. This can be done with the following command: (golang is required)
```
go get github.com/evemu-project/evedbtool
```

## Adding new schema changes

All schema changes MUST be made using migrations. This will avoid breaking the DB.

To create a new migration file:
```
./evedbtool new <name of migration>
```

When editing the migration file, ensure that the sql queries for up and down are both created so the schema change is reversable. Please see existing examples of how this is done in the `migrations` directory.

## new directory structure
sql/
    contains this README and the tool

sql/seed_and_clean/
    contains market seeding sql scripts and scripts to clean the db

sql/base/
    contains all the base db tables for the server emulator for EVEmu stored in GZipped format. These should NEVER be updated/modified.

sql/migrations/
    contains versioned migrations scripts in plaintext (for easier versioning). We will apply them in order.
