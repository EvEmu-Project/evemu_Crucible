-- +migrate Up
-- Inventory and corporation wallets must participate in rollback and crash recovery.
ALTER TABLE entity ENGINE=InnoDB;
ALTER TABLE entity_attributes ENGINE=InnoDB;
ALTER TABLE crpWalletDivisons ENGINE=InnoDB,
    MODIFY balance1 DOUBLE NOT NULL DEFAULT 0,
    MODIFY balance2 DOUBLE NOT NULL DEFAULT 0,
    MODIFY balance3 DOUBLE NOT NULL DEFAULT 0,
    MODIFY balance4 DOUBLE NOT NULL DEFAULT 0,
    MODIFY balance5 DOUBLE NOT NULL DEFAULT 0,
    MODIFY balance6 DOUBLE NOT NULL DEFAULT 0,
    MODIFY balance7 DOUBLE NOT NULL DEFAULT 0;

-- Market delivery and blueprint metadata join the enclosing inventory transaction.
ALTER TABLE mktOrders ENGINE=InnoDB;
ALTER TABLE mktTransactions ENGINE=InnoDB;
ALTER TABLE invBlueprints ENGINE=InnoDB;
ALTER TABLE shipInsurance ENGINE=InnoDB;

-- +migrate Down
-- No automatic downgrade: converting back would remove crash safety and lose
-- wallet precision. Restore a verified pre-migration backup if rollback is needed.
