/*
 * This file is here to deal with some tables needing to contain both
 * static and dynamic information. The solution for ease of release is
 * to keep all the static info in a seperate copy of the tables such
 * that it can be loaded into the non-static table.
 */

/*
 * Truncate all the dynamic tables
 */
TRUNCATE TABLE alliance_shortnames;
TRUNCATE TABLE billsPayable;
TRUNCATE TABLE billsReceivable;
TRUNCATE TABLE bookmarks;
TRUNCATE TABLE cacheLocations;
TRUNCATE TABLE cacheOwners;
TRUNCATE TABLE channelChars;
TRUNCATE TABLE channelMods;
TRUNCATE TABLE character_;
TRUNCATE TABLE chrApplications;
TRUNCATE TABLE chrEmployment;
TRUNCATE TABLE chrMissionState;
TRUNCATE TABLE chrNotes;
TRUNCATE TABLE chrNPCStandings;
TRUNCATE TABLE chrOffers;
TRUNCATE TABLE chrOwnerNote;
TRUNCATE TABLE chrSkillQueue;
TRUNCATE TABLE chrStandings;
TRUNCATE TABLE corporation;
TRUNCATE TABLE courierMissions;
TRUNCATE TABLE crpCharShares;
TRUNCATE TABLE crpOffices;
TRUNCATE TABLE droneState;
TRUNCATE TABLE entity;
TRUNCATE TABLE entity_attributes;
TRUNCATE TABLE eveMail;
TRUNCATE TABLE eveMailDetails;
TRUNCATE TABLE invBlueprints;
TRUNCATE TABLE market_history_old;
TRUNCATE TABLE market_journal;
TRUNCATE TABLE market_orders;
TRUNCATE TABLE market_transactions;
TRUNCATE TABLE npcStandings;
TRUNCATE TABLE ramAssemblyLineStationCostLogs;
TRUNCATE TABLE ramJobs;
TRUNCATE TABLE rentalInfo;
TRUNCATE TABLE srvStatus;

/*
 * Insert static characters (agents)
 * TODO: Develop a query which will populate character_ using agtAgents and
 * random value generation)
 */
INSERT INTO character_
 SELECT
  characterID,accountID,title,description,bounty,balance,securityRating,petitionMessage,logonMinutes,
  corporationID,0 AS corpRole,0 AS rolesAtAll,0 AS rolesAtBase,0 AS rolesAtHQ,0 AS rolesAtOther,
  corporationDateTime,startDateTime,createDateTime,
  ancestryID,careerID,schoolID,careerSpecialityID,gender,
  accessoryID,beardID,costumeID,decoID,eyebrowsID,eyesID,hairID,lipstickID,makeupID,skinID,backgroundID,lightID,
  headRotation1,headRotation2,headRotation3,
  eyeRotation1,eyeRotation2,eyeRotation3,
  camPos1,camPos2,camPos3,
  morph1e,morph1n,morph1s,morph1w,
  morph2e,morph2n,morph2s,morph2w,
  morph3e,morph3n,morph3s,morph3w,
  morph4e,morph4n,morph4s,morph4w,
  stationID,solarSystemID,constellationID,regionID,
  0 AS online
 FROM characterStatic;

/*
 * Copy over the static entities:
 * Static record of EVE System
 */
INSERT INTO entity (itemID, itemName, singleton, quantity)
 VALUES (1, 'EVE System', 1, 1);
/*
 * Insert solar systems
 */
INSERT INTO entity (itemID, itemName, typeID, ownerID, singleton, quantity, x, y, z)
 SELECT solarSystemID, solarSystemName, 5, 1, 1, 1, x, y, z
 FROM mapSolarSystems;
/*
 * Insert stations
 */
INSERT INTO entity (itemID, itemName, typeID, ownerID, locationID, singleton, quantity, x, y, z)
 SELECT stationID, stationName, stationTypeID, corporationID, solarSystemID, 1, 1, x, y, z
 FROM staStations;
/*
 * Insert characters
 */
INSERT INTO entity (itemID, itemName, typeID, ownerID, locationID, singleton, quantity)
 SELECT characterID, characterName, typeID, 1, stationID, 1, 1
  FROM characterStatic;
/*
 * Set the auto-increment lower bound
 */
ALTER TABLE entity AUTO_INCREMENT=140000000;

/*
 * Copy over the static corporation info
 */
INSERT INTO corporation
 SELECT * FROM corporationStatic;
/*
 * Set the auto-increment lower bound
 */
ALTER TABLE corporation AUTO_INCREMENT=2000001;

/*
 * Copy over the static owner info.
 * This is a bit hacky: we rebuild this table although it's static but it
 * allows us not to include its data in dump.
 */
TRUNCATE TABLE eveStaticOwners;
/*
 * Static record of EVE System
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 VALUES (1, 'EVE System', 0);
/*
 * Insert agents
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT characterID, characterName, typeID
 FROM characterStatic;
/*
 * Insert factions
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT factionID, factionName, 30 AS typeID
 FROM chrFactions;
/*
 * Insert corporations
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT corporationID, corporationName, 2 AS typeID
 FROM corporationStatic;

