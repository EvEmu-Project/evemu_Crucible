/*
 * Moving corporation roles from chrCorporationRoles to character_.
 */

ALTER TABLE character_
 ADD COLUMN corpRole BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' AFTER corporationID,
 ADD COLUMN rolesAtAll BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' AFTER corpRole,
 ADD COLUMN rolesAtBase BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' AFTER rolesAtAll,
 ADD COLUMN rolesAtHQ BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' AFTER rolesAtBase,
 ADD COLUMN rolesAtOther BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' AFTER rolesAtHQ
;

DROP TABLE IF EXISTS chrCorporationRoles;
