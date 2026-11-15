
use evemu;   -- set this to your db name
-- seeds specified regionID
-- some region ids found in seed_data.sql

set @regionid=10000001;
set @saturation=0.8; -- fuzzy logic.  % of stations to fill with orders (random selection)

-- select stations to fill
create temporary table if not exists tStations (stationId int, solarSystemID int, regionID int, corporationID int, security float);
truncate table tStations;
insert into tStations
  select stationID,solarSystemID,regionID, corporationID, security
  from staStations
  where regionID=@regionid and rand() < @saturation;

-- actual seeding
INSERT INTO mktOrders (typeID, ownerID, regionID, stationID, price, volEntered, volRemaining, issued,
minVolume, duration, solarSystemID, jumps)
  SELECT typeID, corporationID, regionID, stationID, basePrice / security,
         CASE WHEN invGroups.categoryID = 8 THEN 550000 ELSE 550 END,
         CASE WHEN invGroups.categoryID = 8 THEN 550000 ELSE 550 END,
         132478179209572976, 1, 250, solarSystemID, 1
  FROM tStations, invTypes inner join invGroups USING (groupID)
  WHERE invTypes.published = 1
  AND (
      invGroups.categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46)
      -- Published, market-listed Celestial cargo/secure/audit/freight containers.
      OR (invGroups.groupID IN (12, 340, 448, 649)
          AND invTypes.marketGroupID IS NOT NULL)
  );

-- Officer, Deadspace, Faction and Storyline equipment. Do not restrict this
-- to category 7: special meta-group equipment can exist in other categories.
INSERT INTO mktOrders
    (typeID, ownerID, regionID, stationID, price, volEntered, volRemaining,
     issued, minVolume, duration, solarSystemID, jumps)
SELECT
    it.typeID,
    stations.corporationID,
    stations.regionID,
    stations.stationId,
    CASE
        WHEN it.basePrice IS NULL OR it.basePrice <= 0 THEN 100
        WHEN stations.security <= 0 THEN it.basePrice
        ELSE it.basePrice / stations.security
    END,
    10,
    10,
    132478179209572976,
    1,
    250,
    stations.solarSystemID,
    1
FROM tStations AS stations
CROSS JOIN invTypes AS it
INNER JOIN invMetaTypes AS imt ON imt.typeID = it.typeID
INNER JOIN invMetaGroups AS mg ON mg.metaGroupID = imt.metaGroupID
WHERE LOWER(COALESCE(mg.metaGroupName, '')) IN
    ('officer', 'deadspace', 'faction', 'storyline')
  AND NOT EXISTS (
      SELECT 1
      FROM mktOrders AS existing
      WHERE existing.stationID = stations.stationId
        AND existing.typeID = it.typeID
        AND existing.bid = 0
  );

UPDATE mktOrders SET price = 100 WHERE price = 0;

