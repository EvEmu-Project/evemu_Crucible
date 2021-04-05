
 -- use this to spawn items in market for single station
 -- pick station data from seed_data.sql

set @stationid=60014137;     --Ibaria III - Thukker Mix Warehouse(60014137)
set @solarSystemID=30000053; --Ibaria
set @regionid=10000001;      --Derelik


create temporary table if not exists tStations (stationId int, solarSystemID int, regionID int);
truncate table tStations;
insert into tStations values (60014137, 30000053, 10000001);

-- categoryID IDs and names found in seed_data.sql

-- actual seeding
INSERT INTO mktOrders (typeID, ownerID, regionID, stationID, price, volEntered, volRemaining, issued,
minVolume, duration, solarSystemID, jumps)
  SELECT typeID, stationID, regionID, stationID, basePrice, 550, 550, 132478179209572976, 1, 250, solarSystemID, 1
  FROM tStations, invTypes inner join invGroups on invTypes.groupID=invGroups.groupID
  WHERE invTypes.published = 1
  AND categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 20, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46);
UPDATE mktOrders SET price = 100 WHERE price = 0;
-- 11004 orders per station

