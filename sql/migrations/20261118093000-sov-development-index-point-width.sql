-- Upgrade /sovindex5 DEV-only raw development points from 16-bit.
-- +migrate Up
ALTER TABLE mapSovDevelopmentIndexOverrides
    MODIFY militaryPoints INT(10) UNSIGNED NOT NULL DEFAULT 1000000000,
    MODIFY industrialPoints INT(10) UNSIGNED NOT NULL DEFAULT 30000000;

-- Existing (5,5) values incorrectly represented a displayed level, not points.
UPDATE mapSovDevelopmentIndexOverrides
SET militaryPoints = 1000000000, industrialPoints = 30000000
WHERE militaryPoints = 5 AND industrialPoints = 5;

-- +migrate Down
UPDATE mapSovDevelopmentIndexOverrides
SET militaryPoints = 5, industrialPoints = 5
WHERE militaryPoints > 65535 OR industrialPoints > 65535;
ALTER TABLE mapSovDevelopmentIndexOverrides
    MODIFY militaryPoints SMALLINT(5) UNSIGNED NOT NULL DEFAULT 5,
    MODIFY industrialPoints SMALLINT(5) UNSIGNED NOT NULL DEFAULT 5;
