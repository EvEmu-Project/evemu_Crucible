-- +migrate Up
-- Gate counters, visits and kill records participate in gameplay transactions.
ALTER TABLE chrKillTable ENGINE=InnoDB;
ALTER TABLE chrVisitedSystems ENGINE=InnoDB;
ALTER TABLE mapDynamicData ENGINE=InnoDB;

-- +migrate Down
-- Restore a verified snapshot rather than removing transaction safety.