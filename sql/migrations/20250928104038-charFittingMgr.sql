-- Added table to hold ship fittings items, and added name/description to chrShipFittings

-- +migrate Up
alter table chrShipFittings drop column shipDNA;
alter table chrShipFittings add name text null;
alter table chrShipFittings add description text null;
create table shipFittings
(
    fittingID int not null,
    itemID    int not null,
    flagID    int not null,
    qty       int not null
);

-- +migrate Down
drop table shipFittings;
alter table chrShipFittings drop column name;
alter table chrShipFittings drop column description;
alter table chrShipFittings add shipDNA text null;