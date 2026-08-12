-- Store the versioned Argon2id verifier separately from the legacy client hash.
-- Take and validate a database backup before applying this migration.
-- +migrate Up
ALTER TABLE account
    ADD COLUMN passwordKdf VARCHAR(255)
        CHARACTER SET ascii COLLATE ascii_bin NULL
        AFTER hash;

UPDATE account SET password = '' WHERE password IS NOT NULL AND password <> '';

-- +migrate Down
-- This removes migrated verifiers. Restore from the pre-migration backup first.
ALTER TABLE account DROP COLUMN passwordKdf;
