-- Market money must retain cents at the same scale as order prices and journals.
-- Widening old FLOAT values cannot reconstruct precision already lost historically.
-- Apply offline with a verified snapshot; order escrow conversion rebuilds the table.

-- +migrate Up
ALTER TABLE mktOrders MODIFY COLUMN escrow DECIMAL(16,2) UNSIGNED NOT NULL DEFAULT 0.00;
ALTER TABLE mktTransactions MODIFY COLUMN price DECIMAL(16,2) NOT NULL DEFAULT 0.00;

-- +migrate Down
-- No automatic narrowing: reverting to FLOAT would discard monetary precision.
