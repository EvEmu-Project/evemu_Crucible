-- Add table for LP data
-- +migrate Up
create table if not exists lpWallet
(
    characterID int default 0 not null,
    corporationID int not null,
    balance int
);

-- +migrate Down
drop table lpWallet;