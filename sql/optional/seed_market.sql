use evemu;   -- set this to your db name
-- seeds specified region with skills and ships
-- regionID, 02: The Forge - 01:derelik - 30:heimatar - 16:lonetrek - 42:metropolis - 43:domain - 32:Sinq Laison
--  10000065, 10000020, 10000064, 10000068
set @regionid=10000001;
set @saturation=0.8; -- fuzzy logic.  % of stations to fill with orders (random selection)

use evemu;   -- set this to your db name

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



  ****************************
 -- use this to spawn items in market for single station

set @stationid=60014809;     --Ryddinjorn VIM2 - Pator Tech School
set @solarSystemID=30003410; --Ryddinjorn  - minmatar noob system for pator tech (pts)
set @regionid=10000042;      --metropolis
-----------------
set @stationid=60014140;     --Zemalu IXM2 - Thukker Mix Factory(60014140)
set @solarSystemID=30000055; --Zemalu
set @regionid=10000001;      --Derelik
-----------------
set @stationid=60014137;     --Ibaria III - Thukker Mix Warehouse(60014137)
set @solarSystemID=30000053; --Ibaria
set @regionid=10000001;      --Derelik
-----------------
set @stationid=60004591;     --Abudban IX - Brutor Tribe Bureau
set @solarSystemID=30002507; --Abudban
set @regionid=10000030;      --Heimatar
-----------------
set @stationid=60014779;     --Hulm VIII - Republic University
set @solarSystemID=30002505; --Hulm     - minmatar noob system for republic uni. (run)
set @regionid=10000030;      --Heimatar
-----------------
set @stationid=60012178;     --Aranir VIIIM8 - Ammatar Fleet Logistic Support
set @solarSystemID=30000092; --Aranir
set @regionid=10000001;      --Derelik
-----------------
set @stationid=60009112;     --Odatrik V - TransStellar Shipping Storage
set @solarSystemID=30002509; --Odatrik
set @regionid=10000030;      --Heimatar
-----------------
set @stationid=60012949;     --Juddi VII - DED Logistic Support
set @solarSystemID=30000051; --Mossas
set @regionid=10000001;      --Derelik

create temporary table if not exists tStations (stationId int, solarSystemID int, regionID int);
truncate table tStations;
insert into tStations values (60014137, 30000053, 10000001);

-- actual seeding
INSERT INTO mktOrders (typeID, charID, regionID, stationID, price, volEntered, volRemaining, issued,
minVolume, duration, solarSystemID, jumps)
  SELECT typeID, stationID, regionID, stationID, basePrice, 550, 550, 132478179209572976, 1, 250, solarSystemID, 1
  FROM tStations, invTypes inner join invGroups on invTypes.groupID=invGroups.groupID
  WHERE invTypes.published = 1
  AND categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 20, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46);
UPDATE mktOrders SET price = 100 WHERE price = 0;
-- 11004 orders per station

-- categoryID  categoryName
-- 	4   Material
-- 	5   Accessories
-- 	6   Ship
-- 	7   Module
-- 	8   Charge
-- 	9   Blueprint
-- 	16  Skill
-- 	17  Commodity
-- 	18  Drone
-- 	20  Implant
-- 	22  Deployable
-- 	23  Structure
-- 	24  Reaction
-- 	25  Asteroid
-- 	32  Subsystem
-- 	34  Ancient Relics
-- 	35  Decryptors
-- 	39  Infrastructure Upgrades
-- 	40  Sovereignty Structures
-- 	41  Planetary Interaction
-- 	42  Planetary Resources
-- 	43  Planetary Commodities
-- 	46  Orbitals


-- fix stations security value (missing in dump)
UPDATE `staStations` SET `security`= (SELECT `security` FROM `mapSolarSystems` WHERE mapSolarSystems.solarSystemID = staStations.solarSystemID);
