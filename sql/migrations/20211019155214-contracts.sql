-- Add table for contracts data
-- +migrate Up
create table if not exists ctrContracts
(
    contractId int auto_increment primary key,
    assigneeID int not null,
    startStationID int not null,
    startStationDivision int not null,
    endStationID int not null,
    forCorp int default 0 not null,
    price double not null,
    reward double not null,
    collateral double not null,
    expiretime bigint not null,
    duration bigint not null,
    title varchar(100) not null,
    description varchar(100) not null
    );
create table if not exists ctrRequestedItems
(
    contractId int not null,
    requestedItemTypeId int not null,
    requestedItemQuantity int not null
);
create table if not exists ctrTradedItems
(
    contractId int not null,
    entityId int not null
);

-- +migrate Down
drop table ctrContracts;
drop table ctrRequestedItems;
drop table ctrTradedItems;