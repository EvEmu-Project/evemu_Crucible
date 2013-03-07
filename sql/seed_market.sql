-- seeds specified rgion with skills and ships

set @regionid=10000002; -- regionID, this is The Forge
set @saturation=0.2; -- 20% of stations are filled with orders
-- change "categoryID in (16,6)" to categories you want to be seeded

use evemu;

-- select stations to fill
create temporary table if not exists tStations (stationId int, solarSystemID int);
truncate table tStations;
select round(count(stationID)*@saturation) into @lim from staStations where regionID=@regionid ;
set @i=0;
insert into tStations 
  select stationID,solarSystemID from staStations where (@i:=@i+1)<=@lim AND regionID=@regionid order by rand();

--actual seeding
INSERT INTO market_orders (typeID, charID, regionID, stationID, bid, price, volEntered, volRemaining, issued, orderState, minVolume, contraband, accountID, duration, isCorp, solarSystemID, escrow, jumps) 
  SELECT typeID,1 as charID, @regionid as regionID, stationID, 0 as bid, IF(basePrice=0, 1000, basePrice/10000) as price, 55 as volEntered, 55 as volRemaining, 130065976636875000 as issued,1 as orderState, 1 as minVolume,0 as contraband, 0 as accountID, 18250 as duration,0 as isCorp, solarSystemID, 0 as escrow, 1 as jumps 
  FROM tStations, invTypes inner join invGroups on invTypes.groupID=invGroups.groupID
  WHERE invTypes.published = 1 and categoryID IN (16, 6); --16=skill, 6=ship

drop table tStations;
