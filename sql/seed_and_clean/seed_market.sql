
use evemu;   -- set this to your db name
-- seeds specified regionID
-- some region ids found in seed_data.sql

set @regionid=10000001;
set @saturation=0.8; -- fuzzy logic.  % of stations to fill with orders (random selection)

-- select stations to fill
create temporary table if not exists tStations (stationId int, solarSystemID int, regionID int, corporationID int, security float);
truncate table tStations;
select round(count(stationID)*@saturation) into @lim from staStations where regionID=@regionid ;
set @i=0;
insert into tStations
  select stationID,solarSystemID,regionID, corporationID, security from staStations where (@i:=@i+1)<=@lim AND regionID=@regionid  order by rand();

-- actual seeding
INSERT INTO mktOrders (typeID, charID, regionID, stationID, price, volEntered, volRemaining, issued,
minVolume, duration, solarSystemID, jumps)
  SELECT typeID, corporationID, regionID, stationID, basePrice / security, 550, 550, 132478179209572976, 1, 250, solarSystemID, 1
  FROM tStations, invTypes inner join invGroups USING (groupID)
  WHERE invTypes.published = 1
  AND invGroups.categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46);
UPDATE mktOrders SET price = 100 WHERE price = 0;

