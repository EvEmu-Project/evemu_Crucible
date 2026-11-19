-- DEV/ADMIN-only SOV development index Level 5 test overrides.
-- +migrate Up
-- No changes to claimTime, TCU, infrastructure hub inventories or existing SOV rows.
CREATE TABLE IF NOT EXISTS mapSovDevelopmentIndexOverrides (
    solarSystemID INT(10) UNSIGNED NOT NULL,
    claimStructureID INT(10) UNSIGNED NOT NULL,
    militaryPoints SMALLINT(5) UNSIGNED NOT NULL DEFAULT 5,
    industrialPoints SMALLINT(5) UNSIGNED NOT NULL DEFAULT 5,
    strategicDays SMALLINT(5) UNSIGNED NOT NULL DEFAULT 100,
    PRIMARY KEY (solarSystemID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GM sovereignty index test overrides; TCU-bound';

-- +migrate Down
DROP TABLE IF EXISTS mapSovDevelopmentIndexOverrides;
