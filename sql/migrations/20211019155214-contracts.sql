-- Add table for contracts data
-- +migrate Up
create table if not exists ctrContracts
(
    contractId int auto_increment primary key,
    contractType int not null,
    isPrivate int not null,
    assigneeID int,
    expireTime int not null,
    duration bigint not null,
    startStationID int not null,
    endStationID int,
    price int not null,
    reward int not null,
    collateral int not null,
    title varchar(100) not null,
    description varchar(100) not null,
    startStationDivision int not null,
    forCorp int default 0 not null
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
    entityId int not null,
    quantity int not null
);

-- +migrate Down
drop table ctrContracts;
drop table ctrRequestedItems;
drop table ctrTradedItems;