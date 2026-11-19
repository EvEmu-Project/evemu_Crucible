
 -- use this to spawn items in market for single station
 -- pick station data from seed_data.sql

create temporary table if not exists tStations (stationId int, solarSystemID int, regionID int);
truncate table tStations;
insert into tStations values (60014137, 30000053, 10000001);

-- categoryID IDs and names found in seed_data.sql

-- actual seeding
INSERT INTO mktOrders (typeID, ownerID, regionID, stationID, price, volEntered, volRemaining, issued,
minVolume, duration, solarSystemID, jumps)
  SELECT typeID, stationID, regionID, stationID,
         CASE WHEN basePrice IS NULL OR basePrice <= 0 THEN 100 ELSE GREATEST(0.01, ROUND(basePrice, 2)) END,
         CASE WHEN (invGroups.groupID = 18 OR invGroups.groupID BETWEEN 450 AND 462 OR invGroups.groupID IN (465, 467, 468, 469)) THEN 100000000
               WHEN invGroups.categoryID = 8 THEN 550000 ELSE 550 END,
         CASE WHEN (invGroups.groupID = 18 OR invGroups.groupID BETWEEN 450 AND 462 OR invGroups.groupID IN (465, 467, 468, 469)) THEN 100000000
               WHEN invGroups.categoryID = 8 THEN 550000 ELSE 550 END,
         132478179209572976, 1, 250, solarSystemID, 1
  FROM tStations, invTypes inner join invGroups on invTypes.groupID=invGroups.groupID
  WHERE invTypes.published = 1
  AND (
      invGroups.categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 20, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46)
      -- Published, market-listed Celestial cargo/secure/audit/freight containers.
      OR (invGroups.groupID IN (12, 340, 448, 649)
          AND invTypes.marketGroupID IS NOT NULL)
  );
-- No unscoped UPDATE: preserve prices on all pre-existing player orders.
-- 11004 orders per station

