-- Remove avatar modifiers that cannot be resolved by the client bulk data.
-- Take and validate a database backup before applying this migration.
-- +migrate Up
DELETE m
FROM avatar_modifiers AS m
LEFT JOIN paperdollModifierLocations AS l
    ON l.modifierLocationID = m.modifierLocationID
LEFT JOIN paperdollResources AS r
    ON r.paperdollResourceID = m.paperdollResourceID
WHERE l.modifierLocationID IS NULL
   OR r.paperdollResourceID IS NULL;

-- +migrate Down
-- This cleanup is irreversible; restore the pre-migration backup if needed.
