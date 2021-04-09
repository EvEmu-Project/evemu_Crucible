# DATABASE SETUP:
Author: (Thunder)Groove
Updates: James

These scripts manage the importing and dumping of SQL tables for EVEmu.

## Manual Installation

To install the SQL tables, edit install.sh for pertinent information and run.

To dump existing database, edit dump.sh and run.

## Adding new schema changes

All schema changes MUST be made using migrations. This will avoid breaking the DB.

To create a new migration file:
```
./create-migration.sh <name of migration>
```

When editing the migration file, ensure that the sql queries for up and down are both created so the schema change is reversable. Please see existing examples of how this is done in the `migrations` directory.

## new directory structure
sql/
    contains this README and the new db scripts

sql/seed_and_clean/
    contains market seeding sql scripts and scripts to clean the db

sql/base/
    contains all the base db tables for the server emulator for EVEmu stored in GZipped format. These should NEVER be updated/modified.

sql/migrations/
    contains versioned migrations scripts in plaintext (for easier versioning). We will apply them in order.