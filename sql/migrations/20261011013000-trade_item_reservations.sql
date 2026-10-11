-- +migrate Up
-- Keep the return address of offered items until the complete exchange commits.
CREATE TABLE IF NOT EXISTS chrTradeItems (
    itemID INT UNSIGNED NOT NULL,
    containerID INT UNSIGNED NOT NULL,
    ownerID INT UNSIGNED NOT NULL,
    stationID INT UNSIGNED NOT NULL,
    originalLocationID INT UNSIGNED NOT NULL,
    originalFlag INT UNSIGNED NOT NULL,
    PRIMARY KEY (itemID),
    KEY tradeContainer (containerID),
    CONSTRAINT chrTradeItems_entity FOREIGN KEY (itemID) REFERENCES entity(itemID) ON DELETE RESTRICT
) ENGINE=InnoDB;

-- +migrate Down
-- No automatic downgrade: dropping live reservations can lose offered items.
-- Cancel all offers before any manual downgrade.
