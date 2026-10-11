-- +migrate Up
-- Retain actual sell stock, including attributes and blueprint metadata.
-- Keeping this small mapping separate avoids rewriting millions of NPC orders.
CREATE TABLE IF NOT EXISTS mktOrderItems (
    orderID INT(10) UNSIGNED NOT NULL,
    itemID INT(10) UNSIGNED NOT NULL,
    PRIMARY KEY (orderID),
    UNIQUE KEY market_stock_item (itemID),
    CONSTRAINT market_stock_order FOREIGN KEY (orderID) REFERENCES mktOrders(orderID) ON DELETE CASCADE,
    CONSTRAINT market_stock_entity FOREIGN KEY (itemID) REFERENCES entity(itemID) ON DELETE RESTRICT
) ENGINE=InnoDB;

-- +migrate Down
-- Cancel all retained-stock sell orders before any manual downgrade.
-- Do not drop this mapping while it contains stock: that would lose its reservation.
