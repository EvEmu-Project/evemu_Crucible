-- Add table for contracts data
-- +migrate Up
create table if not exists ctrContracts
(
    contractId int auto_increment primary key,
    contractType int not null,
    issuerID int not null,
    issuerCorpID int not null,
    forCorp int not null,
    isPrivate int not null,
    assigneeID int not null,
    acceptorID int default 0 not null,
    dateIssued bigint(20) unsigned not null,
    dateExpired bigint(20) unsigned not null,
    dateAccepted bigint(20) unsigned default 0 not null,
    expireTimeInMinutes int not null,
    duration bigint not null,
    numDays int not null,
    dateCompleted bigint(20) unsigned default 0 not null,
    startStationID int not null,
    startSolarSystemID int not null,
    startRegionID int not null,
    endStationID int default 0 not null,
    endSolarSystemID int default 0 not null,
    endRegionID int default 0 not null,
    price int not null,
    reward int not null,
    collateral int not null,
    title varchar(100) not null,
    description varchar(100) not null,
    status int default 0 not null,
    crateID int default 0 not null,
    volume decimal(16,2) default 0.00 not null,
    issuerWalletKey int default 0 not null,
    acceptorWalletKey int default 0 not null,
    startStationDivision int not null
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