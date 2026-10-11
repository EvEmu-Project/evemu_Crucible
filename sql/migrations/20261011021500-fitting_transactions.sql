-- +migrate Up
-- Linked fittings and character attribute replacement join inventory transactions.
ALTER TABLE shipWeaponGroups ENGINE=InnoDB;
ALTER TABLE chrCharacterAttributes ENGINE=InnoDB;

-- +migrate Down
-- Restore a verified snapshot rather than removing transaction safety.
