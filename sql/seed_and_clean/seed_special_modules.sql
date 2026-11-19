-- Supplemental global special-module market seed.
-- Adds Officer, Deadspace (including Pith), Faction and Storyline modules
-- to the same randomly selected station set used by seed_market.sql.
--
-- Run after seed_market.sql. The regular published modules are already seeded
-- by seed_market.sql; this script adds rare meta-group modules only.
-- Existing station/type sell orders are preserved, so this is safe to rerun.
--
-- Configure the target region and station saturation to match seed_market.sql.
USE evemu;
SET @regionid = 10000001;
SET @saturation = 0.8;

CREATE TEMPORARY TABLE IF NOT EXISTS tSpecialModuleStations (
    stationId INT,
    solarSystemID INT,
    regionID INT,
    corporationID INT,
    security FLOAT
);
TRUNCATE TABLE tSpecialModuleStations;

INSERT INTO tSpecialModuleStations
    (stationId, solarSystemID, regionID, corporationID, security)
SELECT stationID, solarSystemID, regionID, corporationID, security
FROM staStations
WHERE regionID = @regionid
  AND RAND() < @saturation;

-- Rare Officer, Deadspace, Faction and Storyline equipment: 10 units per selected station and item.
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
FROM tSpecialModuleStations AS stations
CROSS JOIN invTypes AS it
INNER JOIN invGroups AS ig
    ON it.groupID = ig.groupID
INNER JOIN invMetaTypes AS imt
    ON it.typeID = imt.typeID
INNER JOIN invMetaGroups AS mg
    ON imt.metaGroupID = mg.metaGroupID
WHERE LOWER(COALESCE(mg.metaGroupName, '')) IN
      ('officer', 'deadspace', 'faction', 'storyline')
  AND NOT EXISTS (
      SELECT 1
      FROM mktOrders AS existing
      WHERE existing.stationID = stations.stationId
        AND existing.typeID = it.typeID
        AND existing.bid = 0
  );

-- Report how many rare-module orders and units were added in the target region.
SELECT
    COUNT(*) AS specialModuleSellOrders,
    COALESCE(SUM(volRemaining), 0) AS specialModuleUnits
FROM mktOrders
WHERE regionID = @regionid
  AND stationID IN (SELECT stationId FROM tSpecialModuleStations)
  AND bid = 0
  AND typeID IN (
      SELECT it.typeID
      FROM invTypes AS it
      INNER JOIN invGroups AS ig ON it.groupID = ig.groupID
      INNER JOIN invMetaTypes AS imt ON it.typeID = imt.typeID
      INNER JOIN invMetaGroups AS mg ON imt.metaGroupID = mg.metaGroupID
      WHERE LOWER(COALESCE(mg.metaGroupName, '')) IN
            ('officer', 'deadspace', 'faction', 'storyline')
  );
