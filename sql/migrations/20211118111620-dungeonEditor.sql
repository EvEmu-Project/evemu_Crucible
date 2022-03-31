-- Dungeon Editor data
-- +migrate Up
ALTER TABLE dunTemplates
    ADD COLUMN dunTemplateDescription VARCHAR(400) NOT NULL DEFAULT "" AFTER dunTemplateName;
ALTER TABLE dunTemplates
    ADD COLUMN dunTemplateFactionID INT(10) NOT NULL DEFAULT 0 AFTER dunTemplateDescription;
CREATE TABLE dunArchetypes (
    archetypeID INT(10),
    archetypeName VARCHAR(100));
INSERT INTO dunArchetypes
VALUES
    (1,"Mission"),
    (2,"Gravimetric"),
    (3,"Magnetometric"),
    (4,"Radar"),
    (5,"Ladar"),
    (6,"Wormhole"),
    (7,"Anomaly"),
    (8,"Unrated"),
    (9,"Escalation"),
    (10,"Rated");
CREATE TABLE dunDungeons (
    dungeonID INT(10) NOT NULL,
    dungeonName VARCHAR(100) NOT NULL DEFAULT "",
    dungeonStatus INT(10) NOT NULL DEFAULT 0,
    factionID INT(10) NOT NULL DEFAULT 0,
    archetypeID INT(10) NOT NULL DEFAULT 0,
    PRIMARY KEY (dungeonID));
CREATE TABLE dunGroups (
    groupID INT(10) NOT NULL,
    groupName VARCHAR(100) NOT NULL DEFAULT "",
    yaw DOUBLE NOT NULL DEFAULT 0,
    pitch DOUBLE NOT NULL DEFAULT 0,
    roll DOUBLE NOT NULL DEFAULT 0);
CREATE TABLE dunRooms (
    roomID INT(10) NOT NULL,
    roomName VARCHAR(100) NOT NULL DEFAULT "",
    dungeonID INT(10) NOT NULL);
INSERT INTO dunGroups (groupID)
SELECT DISTINCT
    dunGroupID as groupID
FROM
    dunGroupData;
UPDATE dunGroups g
    SET groupName=CONCAT(
    CASE
        WHEN SUBSTRING(g.groupID,1,1) = 1 THEN "Decorative"
        WHEN SUBSTRING(g.groupID,1,1) = 2 THEN "SEB"
        WHEN SUBSTRING(g.groupID,1,1) = 3 THEN "Mining"
        WHEN SUBSTRING(g.groupID,1,1) = 4 THEN "LCO"
        WHEN SUBSTRING(g.groupID,1,1) = 5 THEN "Ships"
        WHEN SUBSTRING(g.groupID,1,1) = 6 THEN "Base"
        WHEN SUBSTRING(g.groupID,1,1) = 7 THEN "StGun"
        WHEN SUBSTRING(g.groupID,1,1) = 8 THEN "StWreck"
        WHEN SUBSTRING(g.groupID,1,1) = 9 THEN "Misc"
        ELSE ""
    END, "_",
    CASE
        WHEN SUBSTRING(g.groupID,2,1) = 1 THEN "SpObj"
        WHEN SUBSTRING(g.groupID,2,1) = 2 THEN "EfBeac"
        WHEN SUBSTRING(g.groupID,2,1) = 3 THEN "RoidType"
        WHEN SUBSTRING(g.groupID,2,1) = 4 THEN "IceType"
        WHEN SUBSTRING(g.groupID,2,1) = 6 THEN "RoidColony"
        WHEN SUBSTRING(g.groupID,2,1) = 9 THEN "Misc"
        ELSE "null"
    END, "_(legacy)");

CREATE TABLE dunRoomObjects (
    objectID INT(10) NOT NULL AUTO_INCREMENT,
    roomID INT(10) NOT NULL,
    typeID INT(10) NOT NULL,
    groupID INT(10) NOT NULL,
    x DOUBLE NOT NULL DEFAULT 0,
    y DOUBLE NOT NULL DEFAULT 0,
    z DOUBLE NOT NULL DEFAULT 0,
    yaw DOUBLE NOT NULL DEFAULT 0,
    pitch DOUBLE NOT NULL DEFAULT 0,
    roll DOUBLE NOT NULL DEFAULT 0,
    radius INT(11) NOT NULL DEFAULT 0,
    PRIMARY KEY(objectID)
);

INSERT INTO dunRoomObjects (
    roomID,
    typeID,
    groupID,
    x,
    y,
    z
)
SELECT DISTINCT
    dunRoomID as roomID,
    itemTypeID as typeID,
    dunGroupID as groupID,
    -100+200*RAND() as x,
    -100+200*RAND() as y,
    -100+200*RAND() as z
    FROM
        dunRoomData INNER JOIN dunGroupData USING (dunGroupID);

UPDATE dunRoomObjects 
	SET radius = (
		SELECT
			radius
		FROM
			invTypes
		WHERE
			invTypes.typeID = dunRoomObjects.typeID
	);

DROP TABLE dunGroupData;
DROP TABLE dunRoomData;

-- +migrate Down
ALTER TABLE dunTemplates
    DROP COLUMN dunTemplateDescription;
ALTER TABLE dunTemplates
    DROP COLUMN dunTemplateFactionID;
DROP TABLE dunArchetypes;
DROP TABLE dunDungeons;
DROP TABLE dunGroups;
DROP TABLE dunRooms;
DROP TABLE dunRoomObjects;

CREATE TABLE `dunRoomData` (
  `ai` int(11) NOT NULL AUTO_INCREMENT,
  `dunRoomID` int(11) NOT NULL DEFAULT 0,
  `dunGroupID` int(11) NOT NULL DEFAULT 0,
  `xpos` int(11) NOT NULL DEFAULT 0,
  `ypos` int(11) NOT NULL DEFAULT 0,
  `zpos` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`ai`),
  KEY `dunRoomID` (`dunRoomID`)
) ENGINE=MyISAM AUTO_INCREMENT=1121 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE `dunGroupData` (
  `ai` int(11) NOT NULL AUTO_INCREMENT,
  `dunGroupID` int(11) NOT NULL,
  `itemTypeID` int(11) NOT NULL,
  `itemGroupID` int(11) NOT NULL DEFAULT 0,
  `xpos` int(11) NOT NULL DEFAULT 0,
  `ypos` int(11) NOT NULL DEFAULT 0,
  `zpos` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`ai`),
  KEY `dunGroupID` (`dunGroupID`)
) ENGINE=MyISAM AUTO_INCREMENT=1340 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (110,26272,226,0,0,0),
	 (110,32386,226,0,0,0),
	 (110,32387,226,0,0,0),
	 (110,29494,226,0,0,0),
	 (130,2612,226,0,0,0),
	 (130,10782,226,0,0,0),
	 (130,10787,226,0,0,0),
	 (130,11209,226,0,0,0),
	 (130,23752,226,0,0,0),
	 (130,23753,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (130,23754,226,0,0,0),
	 (130,23755,226,0,0,0),
	 (130,23756,226,0,0,0),
	 (130,28622,467,0,0,0),
	 (131,10267,226,0,0,0),
	 (131,10269,226,0,0,0),
	 (131,10783,226,0,0,0),
	 (131,10784,226,0,0,0),
	 (131,10785,226,0,0,0),
	 (131,10786,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (131,10137,226,0,0,0),
	 (132,28625,450,0,0,0),
	 (132,28619,469,0,0,0),
	 (132,28624,452,0,0,0),
	 (132,28623,453,0,0,0),
	 (132,28626,468,0,0,0),
	 (132,28621,456,0,0,0),
	 (132,28620,457,0,0,0),
	 (132,28618,459,0,0,0),
	 (132,27028,462,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (132,28617,462,0,0,0),
	 (132,26851,452,0,0,0),
	 (132,26713,467,0,0,0),
	 (132,26868,456,0,0,0),
	 (140,10120,226,0,0,0),
	 (140,30436,226,0,0,0),
	 (140,16736,319,0,0,0),
	 (140,10121,226,0,0,0),
	 (160,10771,226,0,0,0),
	 (160,10772,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (160,10773,226,0,0,0),
	 (160,10774,226,0,0,0),
	 (160,10775,226,0,0,0),
	 (160,10778,226,0,0,0),
	 (160,10779,226,0,0,0),
	 (160,10780,226,0,0,0),
	 (160,10781,226,0,0,0),
	 (160,10256,226,0,0,0),
	 (160,23615,226,0,0,0),
	 (160,16728,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (160,16839,319,0,0,0),
	 (160,16840,319,0,0,0),
	 (160,16841,319,0,0,0),
	 (160,16842,319,0,0,0),
	 (160,16843,319,0,0,0),
	 (160,16844,319,0,0,0),
	 (160,20165,319,0,0,0),
	 (160,20177,319,0,0,0),
	 (160,16846,319,0,0,0),
	 (160,16847,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (160,26898,319,0,0,0),
	 (160,28608,319,0,0,0),
	 (160,17644,319,0,0,0),
	 (160,22742,319,0,0,0),
	 (160,22743,319,0,0,0),
	 (160,22744,319,0,0,0),
	 (160,23663,319,0,0,0),
	 (160,2928,226,0,0,0),
	 (190,2858,226,0,0,0),
	 (190,4319,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (190,29511,226,0,0,0),
	 (200,3626,226,0,0,0),
	 (210,28066,226,0,0,0),
	 (210,9876,226,0,0,0),
	 (210,9877,226,0,0,0),
	 (210,9878,226,0,0,0),
	 (210,9879,226,0,0,0),
	 (210,9880,226,0,0,0),
	 (210,26232,226,0,0,0),
	 (210,26233,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (210,26234,226,0,0,0),
	 (220,28070,226,0,0,0),
	 (220,9882,226,0,0,0),
	 (220,9883,226,0,0,0),
	 (220,9884,226,0,0,0),
	 (220,9881,226,0,0,0),
	 (220,26235,226,0,0,0),
	 (220,26236,226,0,0,0),
	 (230,9885,226,0,0,0),
	 (230,9886,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (230,9888,226,0,0,0),
	 (230,9889,226,0,0,0),
	 (230,9890,226,0,0,0),
	 (230,9891,226,0,0,0),
	 (230,9892,226,0,0,0),
	 (230,9887,226,0,0,0),
	 (230,26237,226,0,0,0),
	 (230,26238,226,0,0,0),
	 (230,26239,226,0,0,0),
	 (240,9875,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (240,9893,226,0,0,0),
	 (240,9894,226,0,0,0),
	 (240,9895,226,0,0,0),
	 (240,9896,226,0,0,0),
	 (240,9897,226,0,0,0),
	 (240,9898,226,0,0,0),
	 (240,28071,226,0,0,0),
	 (240,26229,226,0,0,0),
	 (240,26230,226,0,0,0),
	 (240,26231,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (400,25829,784,0,0,0),
	 (420,26860,226,0,0,0),
	 (420,23860,319,0,0,0),
	 (420,32128,226,0,0,0),
	 (420,23553,226,0,0,0),
	 (420,32419,226,0,0,0),
	 (420,32420,226,0,0,0),
	 (420,32421,226,0,0,0),
	 (420,32423,226,0,0,0),
	 (420,3952,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (420,29546,226,0,0,0),
	 (420,26739,226,0,0,0),
	 (420,26743,226,0,0,0),
	 (420,26746,226,0,0,0),
	 (420,26794,226,0,0,0),
	 (420,30895,226,0,0,0),
	 (420,30896,226,0,0,0),
	 (420,30897,226,0,0,0),
	 (420,30898,226,0,0,0),
	 (420,32133,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (420,32134,226,0,0,0),
	 (420,32135,226,0,0,0),
	 (420,32136,226,0,0,0),
	 (420,32137,226,0,0,0),
	 (420,26740,319,0,0,0),
	 (430,21799,226,0,0,0),
	 (430,23741,226,0,0,0),
	 (430,23742,226,0,0,0),
	 (430,23763,226,0,0,0),
	 (430,24032,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (430,24252,226,0,0,0),
	 (430,24480,226,0,0,0),
	 (430,32129,226,0,0,0),
	 (430,32132,226,0,0,0),
	 (430,29208,226,0,0,0),
	 (430,29250,226,0,0,0),
	 (430,24453,319,0,0,0),
	 (430,24245,319,0,0,0),
	 (430,2927,226,0,0,0),
	 (430,2284,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (430,2285,226,0,0,0),
	 (431,21821,226,0,0,0),
	 (431,21822,226,0,0,0),
	 (431,21824,226,0,0,0),
	 (431,21825,226,0,0,0),
	 (431,21826,226,0,0,0),
	 (431,21827,226,0,0,0),
	 (431,21828,226,0,0,0),
	 (432,25828,319,0,0,0),
	 (432,26853,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (432,26854,319,0,0,0),
	 (432,26855,319,0,0,0),
	 (432,28340,319,0,0,0),
	 (433,3620,226,0,0,0),
	 (433,32131,226,0,0,0),
	 (433,29251,226,0,0,0),
	 (433,30789,226,0,0,0),
	 (406,2468,226,0,0,0),
	 (406,29585,226,0,0,0),
	 (406,29586,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (406,29589,226,0,0,0),
	 (406,29590,226,0,0,0),
	 (406,29591,226,0,0,0),
	 (406,29592,226,0,0,0),
	 (406,29593,226,0,0,0),
	 (406,29594,226,0,0,0),
	 (406,26968,226,0,0,0),
	 (406,30900,226,0,0,0),
	 (407,23761,226,0,0,0),
	 (407,23762,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (407,23826,226,0,0,0),
	 (407,23833,226,0,0,0),
	 (407,23858,319,0,0,0),
	 (407,24310,319,0,0,0),
	 (407,3920,226,0,0,0),
	 (407,4099,226,0,0,0),
	 (407,32216,226,0,0,0),
	 (407,32275,226,0,0,0),
	 (407,29505,226,0,0,0),
	 (407,23859,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (407,29566,226,0,0,0),
	 (407,29567,226,0,0,0),
	 (407,29568,226,0,0,0),
	 (407,29569,226,0,0,0),
	 (407,29570,226,0,0,0),
	 (402,29551,226,0,0,0),
	 (402,29552,226,0,0,0),
	 (402,29553,226,0,0,0),
	 (402,29554,226,0,0,0),
	 (402,29555,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (402,29556,226,0,0,0),
	 (402,29557,226,0,0,0),
	 (402,29558,226,0,0,0),
	 (402,29559,226,0,0,0),
	 (402,3591,319,0,0,0),
	 (403,23968,226,0,0,0),
	 (403,24029,226,0,0,0),
	 (403,24457,226,0,0,0),
	 (403,24458,226,0,0,0),
	 (403,29561,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (403,29562,226,0,0,0),
	 (403,29563,226,0,0,0),
	 (403,29564,226,0,0,0),
	 (403,29565,226,0,0,0),
	 (408,29571,226,0,0,0),
	 (408,29572,226,0,0,0),
	 (408,29573,226,0,0,0),
	 (408,29574,226,0,0,0),
	 (408,29575,226,0,0,0),
	 (408,21807,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (408,29507,226,0,0,0),
	 (408,29506,226,0,0,0),
	 (408,29508,226,0,0,0),
	 (408,29509,226,0,0,0),
	 (408,4100,226,0,0,0),
	 (404,29576,226,0,0,0),
	 (404,29577,226,0,0,0),
	 (404,29578,226,0,0,0),
	 (404,29579,226,0,0,0),
	 (404,29580,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (404,29581,226,0,0,0),
	 (404,29582,226,0,0,0),
	 (404,29583,226,0,0,0),
	 (404,29584,226,0,0,0),
	 (404,25375,226,0,0,0),
	 (409,21820,226,0,0,0),
	 (409,29452,226,0,0,0),
	 (409,29453,226,0,0,0),
	 (409,29454,226,0,0,0),
	 (409,29455,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (409,29456,226,0,0,0),
	 (409,29457,226,0,0,0),
	 (409,29458,226,0,0,0),
	 (409,29459,226,0,0,0),
	 (409,29460,226,0,0,0),
	 (409,3967,226,0,0,0),
	 (409,23227,226,0,0,0),
	 (409,23963,319,0,0,0),
	 (409,24609,226,0,0,0),
	 (409,25232,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (410,21830,226,0,0,0),
	 (410,21831,226,0,0,0),
	 (410,21832,226,0,0,0),
	 (410,21833,226,0,0,0),
	 (410,21834,226,0,0,0),
	 (410,21835,226,0,0,0),
	 (410,21836,226,0,0,0),
	 (410,21837,226,0,0,0),
	 (410,21838,226,0,0,0),
	 (410,4007,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (410,25231,226,0,0,0),
	 (405,29596,226,0,0,0),
	 (405,29597,226,0,0,0),
	 (405,29598,226,0,0,0),
	 (405,29599,226,0,0,0),
	 (405,29600,226,0,0,0),
	 (405,29601,226,0,0,0),
	 (405,29602,226,0,0,0),
	 (405,29603,226,0,0,0),
	 (405,29604,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (405,3056,226,0,0,0),
	 (405,3059,226,0,0,0),
	 (405,3509,226,0,0,0),
	 (405,2234,226,0,0,0),
	 (401,29595,226,0,0,0),
	 (401,23223,226,0,0,0),
	 (401,23224,226,0,0,0),
	 (401,23225,226,0,0,0),
	 (401,23226,226,0,0,0),
	 (401,23228,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (401,23229,226,0,0,0),
	 (401,23230,226,0,0,0),
	 (401,23231,226,0,0,0),
	 (401,3957,226,0,0,0),
	 (401,2842,226,0,0,0),
	 (507,2097,226,0,0,0),
	 (507,2105,226,0,0,0),
	 (507,2111,226,0,0,0),
	 (507,2115,226,0,0,0),
	 (507,2121,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (508,2116,226,0,0,0),
	 (508,2122,226,0,0,0),
	 (508,2112,226,0,0,0),
	 (508,2106,226,0,0,0),
	 (508,2096,226,0,0,0),
	 (509,2101,226,0,0,0),
	 (509,2107,226,0,0,0),
	 (509,2119,226,0,0,0),
	 (509,2113,226,0,0,0),
	 (509,2123,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (509,2910,226,0,0,0),
	 (510,2102,226,0,0,0),
	 (510,2110,226,0,0,0),
	 (510,2114,226,0,0,0),
	 (510,2120,226,0,0,0),
	 (510,2124,226,0,0,0),
	 (500,26911,226,0,0,0),
	 (500,10138,226,0,0,0),
	 (500,29606,226,0,0,0),
	 (500,30514,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (500,30515,226,0,0,0),
	 (500,29386,226,0,0,0),
	 (500,29157,226,0,0,0),
	 (500,29158,226,0,0,0),
	 (500,29159,226,0,0,0),
	 (500,29160,226,0,0,0),
	 (500,29161,226,0,0,0),
	 (500,29348,226,0,0,0),
	 (500,29349,226,0,0,0),
	 (500,29350,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (500,29351,226,0,0,0),
	 (500,29352,226,0,0,0),
	 (500,29353,226,0,0,0),
	 (500,29354,226,0,0,0),
	 (500,29355,226,0,0,0),
	 (500,29356,226,0,0,0),
	 (500,29357,226,0,0,0),
	 (500,29358,226,0,0,0),
	 (500,29359,226,0,0,0),
	 (500,29360,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (500,29361,226,0,0,0),
	 (500,29362,226,0,0,0),
	 (500,29363,226,0,0,0),
	 (500,29364,226,0,0,0),
	 (510,3630,226,0,0,0),
	 (510,3631,226,0,0,0),
	 (510,3632,226,0,0,0),
	 (510,3633,226,0,0,0),
	 (510,3635,226,0,0,0),
	 (510,3637,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (510,3639,226,0,0,0),
	 (510,3641,226,0,0,0),
	 (510,3642,226,0,0,0),
	 (510,3644,226,0,0,0),
	 (510,3646,226,0,0,0),
	 (510,3648,226,0,0,0),
	 (510,3649,226,0,0,0),
	 (510,3650,226,0,0,0),
	 (510,3652,226,0,0,0),
	 (510,3657,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (510,3658,226,0,0,0),
	 (510,3659,226,0,0,0),
	 (510,3660,226,0,0,0),
	 (510,3661,226,0,0,0),
	 (510,3662,226,0,0,0),
	 (510,3667,226,0,0,0),
	 (510,30894,226,0,0,0),
	 (520,3670,226,0,0,0),
	 (520,3671,226,0,0,0),
	 (520,3672,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (520,3674,226,0,0,0),
	 (520,3675,226,0,0,0),
	 (520,3676,226,0,0,0),
	 (520,3677,226,0,0,0),
	 (520,3678,226,0,0,0),
	 (520,3679,226,0,0,0),
	 (520,3680,226,0,0,0),
	 (520,3681,226,0,0,0),
	 (520,3682,226,0,0,0),
	 (520,3684,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (520,3686,226,0,0,0),
	 (520,3688,226,0,0,0),
	 (520,3690,226,0,0,0),
	 (520,3523,226,0,0,0),
	 (520,3592,226,0,0,0),
	 (520,3593,226,0,0,0),
	 (520,3594,226,0,0,0),
	 (520,3595,226,0,0,0),
	 (520,3600,226,0,0,0),
	 (520,3601,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (530,3668,226,0,0,0),
	 (530,3669,226,0,0,0),
	 (530,3704,226,0,0,0),
	 (530,3706,226,0,0,0),
	 (530,3708,226,0,0,0),
	 (530,3710,226,0,0,0),
	 (530,3712,226,0,0,0),
	 (530,3714,226,0,0,0),
	 (530,3716,226,0,0,0),
	 (530,3718,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (530,3720,226,0,0,0),
	 (530,3722,226,0,0,0),
	 (530,3724,226,0,0,0),
	 (530,3726,226,0,0,0),
	 (530,3728,226,0,0,0),
	 (530,3602,226,0,0,0),
	 (530,3603,226,0,0,0),
	 (530,3604,226,0,0,0),
	 (530,3605,226,0,0,0),
	 (530,3610,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (530,3611,226,0,0,0),
	 (530,3612,226,0,0,0),
	 (530,3613,226,0,0,0),
	 (530,3614,226,0,0,0),
	 (530,3615,226,0,0,0),
	 (530,2734,226,0,0,0),
	 (540,3730,226,0,0,0),
	 (540,3757,226,0,0,0),
	 (540,3759,226,0,0,0),
	 (540,3760,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (540,3761,226,0,0,0),
	 (540,3762,226,0,0,0),
	 (540,3763,226,0,0,0),
	 (540,3770,226,0,0,0),
	 (540,3772,226,0,0,0),
	 (540,3774,226,0,0,0),
	 (540,3776,226,0,0,0),
	 (540,3778,226,0,0,0),
	 (540,3780,226,0,0,0),
	 (540,3781,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (540,3782,226,0,0,0),
	 (540,3783,226,0,0,0),
	 (540,3784,226,0,0,0),
	 (540,3785,226,0,0,0),
	 (540,3786,226,0,0,0),
	 (540,3787,226,0,0,0),
	 (540,3788,226,0,0,0),
	 (540,3789,226,0,0,0),
	 (540,3790,226,0,0,0),
	 (540,3791,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (601,16822,319,0,0,0),
	 (601,16823,319,0,0,0),
	 (601,16824,319,0,0,0),
	 (601,16825,319,0,0,0),
	 (601,16826,319,0,0,0),
	 (601,16827,319,0,0,0),
	 (601,16828,319,0,0,0),
	 (601,16829,319,0,0,0),
	 (601,16830,319,0,0,0),
	 (601,2831,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (601,32405,226,0,0,0),
	 (601,2926,226,0,0,0),
	 (601,11076,319,0,0,0),
	 (601,23949,319,0,0,0),
	 (601,26856,383,0,0,0),
	 (601,26857,383,0,0,0),
	 (601,26858,383,0,0,0),
	 (602,16757,319,0,0,0),
	 (602,16758,319,0,0,0),
	 (602,16759,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (602,16760,319,0,0,0),
	 (602,16761,319,0,0,0),
	 (602,16762,319,0,0,0),
	 (602,16763,319,0,0,0),
	 (602,16764,319,0,0,0),
	 (602,16765,319,0,0,0),
	 (602,11077,319,0,0,0),
	 (603,16766,319,0,0,0),
	 (603,16767,319,0,0,0),
	 (603,16768,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (603,16769,319,0,0,0),
	 (603,16770,319,0,0,0),
	 (603,16771,319,0,0,0),
	 (603,16772,319,0,0,0),
	 (603,16773,319,0,0,0),
	 (603,16774,319,0,0,0),
	 (603,16727,319,0,0,0),
	 (603,16731,319,0,0,0),
	 (603,2832,226,0,0,0),
	 (603,11081,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (603,17380,319,0,0,0),
	 (604,16796,319,0,0,0),
	 (604,16797,319,0,0,0),
	 (604,16798,319,0,0,0),
	 (604,16799,319,0,0,0),
	 (604,16794,319,0,0,0),
	 (604,16800,319,0,0,0),
	 (604,16801,319,0,0,0),
	 (604,16802,319,0,0,0),
	 (604,16803,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (604,16793,319,0,0,0),
	 (604,11079,319,0,0,0),
	 (604,19505,319,0,0,0),
	 (604,17382,319,0,0,0),
	 (605,16814,319,0,0,0),
	 (605,16815,319,0,0,0),
	 (605,16816,319,0,0,0),
	 (605,16817,319,0,0,0),
	 (605,16818,319,0,0,0),
	 (605,16819,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (605,16820,319,0,0,0),
	 (605,16813,319,0,0,0),
	 (605,16821,319,0,0,0),
	 (605,3510,226,0,0,0),
	 (605,2852,226,0,0,0),
	 (605,3499,319,0,0,0),
	 (605,11080,319,0,0,0),
	 (605,17381,319,0,0,0),
	 (606,18012,319,0,0,0),
	 (606,18013,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (606,18014,319,0,0,0),
	 (606,18015,319,0,0,0),
	 (606,18016,319,0,0,0),
	 (606,18017,319,0,0,0),
	 (606,18018,319,0,0,0),
	 (606,18019,319,0,0,0),
	 (606,18020,319,0,0,0),
	 (606,23767,227,0,0,0),
	 (606,16733,319,0,0,0),
	 (606,16732,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (606,18021,319,0,0,0),
	 (606,4011,319,0,0,0),
	 (607,16748,319,0,0,0),
	 (607,16749,319,0,0,0),
	 (607,16750,319,0,0,0),
	 (607,16751,319,0,0,0),
	 (607,16756,319,0,0,0),
	 (607,16752,319,0,0,0),
	 (607,16753,319,0,0,0),
	 (607,16754,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (607,16755,319,0,0,0),
	 (607,16726,319,0,0,0),
	 (607,16730,319,0,0,0),
	 (607,16836,319,0,0,0),
	 (607,16837,319,0,0,0),
	 (607,16838,319,0,0,0),
	 (607,17777,319,0,0,0),
	 (607,29469,319,0,0,0),
	 (607,10237,226,0,0,0),
	 (607,29165,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (607,29167,319,0,0,0),
	 (607,29168,319,0,0,0),
	 (607,29166,319,0,0,0),
	 (607,10235,226,0,0,0),
	 (607,10236,226,0,0,0),
	 (607,26895,319,0,0,0),
	 (607,26709,383,0,0,0),
	 (608,29301,319,0,0,0),
	 (608,16775,319,0,0,0),
	 (608,16776,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (608,16777,319,0,0,0),
	 (608,16778,319,0,0,0),
	 (608,16779,319,0,0,0),
	 (608,16780,319,0,0,0),
	 (608,16781,319,0,0,0),
	 (608,16782,319,0,0,0),
	 (608,10240,226,0,0,0),
	 (608,29172,319,0,0,0),
	 (608,29169,319,0,0,0),
	 (608,29175,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (608,29178,319,0,0,0),
	 (608,16850,319,0,0,0),
	 (608,16851,319,0,0,0),
	 (608,16852,319,0,0,0),
	 (608,17778,319,0,0,0),
	 (608,29279,226,0,0,0),
	 (608,10238,226,0,0,0),
	 (608,10239,226,0,0,0),
	 (609,16784,319,0,0,0),
	 (609,16785,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (609,16786,319,0,0,0),
	 (609,16787,319,0,0,0),
	 (609,16788,319,0,0,0),
	 (609,16789,319,0,0,0),
	 (609,16790,319,0,0,0),
	 (609,16791,319,0,0,0),
	 (609,16792,319,0,0,0),
	 (609,29173,319,0,0,0),
	 (609,29170,319,0,0,0),
	 (609,29176,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (609,29179,319,0,0,0),
	 (609,16855,319,0,0,0),
	 (609,32239,319,0,0,0),
	 (609,17779,319,0,0,0),
	 (609,32194,319,0,0,0),
	 (609,10234,226,0,0,0),
	 (609,24452,319,0,0,0),
	 (610,16805,319,0,0,0),
	 (610,16806,319,0,0,0),
	 (610,16807,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (610,16808,319,0,0,0),
	 (610,16809,319,0,0,0),
	 (610,16810,319,0,0,0),
	 (610,16811,319,0,0,0),
	 (610,16812,319,0,0,0),
	 (610,16804,319,0,0,0),
	 (610,10243,226,0,0,0),
	 (610,29174,319,0,0,0),
	 (610,29171,319,0,0,0),
	 (610,29177,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (610,29180,319,0,0,0),
	 (610,16857,319,0,0,0),
	 (610,16858,319,0,0,0),
	 (610,16859,319,0,0,0),
	 (610,17780,319,0,0,0),
	 (610,3974,226,0,0,0),
	 (610,10241,226,0,0,0),
	 (610,10242,226,0,0,0),
	 (611,30513,226,0,0,0),
	 (611,30901,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (611,30905,226,0,0,0),
	 (611,30293,226,0,0,0),
	 (611,30299,226,0,0,0),
	 (611,30300,226,0,0,0),
	 (611,30301,226,0,0,0),
	 (611,30302,226,0,0,0),
	 (612,30502,226,0,0,0),
	 (612,30503,226,0,0,0),
	 (612,30504,226,0,0,0),
	 (612,30505,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (612,30506,226,0,0,0),
	 (612,30797,226,0,0,0),
	 (612,30798,226,0,0,0),
	 (612,30902,226,0,0,0),
	 (612,30903,226,0,0,0),
	 (612,30904,226,0,0,0),
	 (612,30294,226,0,0,0),
	 (612,30295,226,0,0,0),
	 (612,30298,226,0,0,0),
	 (613,19671,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (613,24456,319,0,0,0),
	 (613,19672,319,0,0,0),
	 (613,19673,319,0,0,0),
	 (613,26895,319,0,0,0),
	 (620,26720,319,0,0,0),
	 (620,26721,319,0,0,0),
	 (620,26722,319,0,0,0),
	 (620,26723,319,0,0,0),
	 (620,26724,319,0,0,0),
	 (620,26725,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (620,26726,319,0,0,0),
	 (620,26727,319,0,0,0),
	 (620,26728,319,0,0,0),
	 (620,17781,319,0,0,0),
	 (620,17782,319,0,0,0),
	 (620,17783,319,0,0,0),
	 (620,17784,319,0,0,0),
	 (620,17785,319,0,0,0),
	 (620,17786,319,0,0,0),
	 (620,17787,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (620,17788,319,0,0,0),
	 (620,17789,319,0,0,0),
	 (620,17790,319,0,0,0),
	 (620,17775,319,0,0,0),
	 (620,17776,319,0,0,0),
	 (620,29325,319,0,0,0),
	 (620,29346,319,0,0,0),
	 (620,29437,319,0,0,0),
	 (620,29461,319,0,0,0),
	 (630,17394,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (630,17395,319,0,0,0),
	 (630,17396,319,0,0,0),
	 (630,17397,319,0,0,0),
	 (630,17398,319,0,0,0),
	 (630,17399,319,0,0,0),
	 (630,17400,319,0,0,0),
	 (630,17401,319,0,0,0),
	 (630,26894,319,0,0,0),
	 (640,17626,319,0,0,0),
	 (640,17627,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (640,17628,319,0,0,0),
	 (640,17629,319,0,0,0),
	 (640,23906,319,0,0,0),
	 (640,25363,319,0,0,0),
	 (640,29149,319,0,0,0),
	 (650,21823,226,0,0,0),
	 (650,21829,226,0,0,0),
	 (650,22064,226,0,0,0),
	 (650,23237,226,0,0,0),
	 (650,23889,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (650,29503,226,0,0,0),
	 (660,13200,319,0,0,0),
	 (660,17763,319,0,0,0),
	 (660,19743,319,0,0,0),
	 (660,22711,319,0,0,0),
	 (670,18567,319,0,0,0),
	 (670,18568,319,0,0,0),
	 (670,18569,319,0,0,0),
	 (670,29468,319,0,0,0),
	 (670,21659,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (680,22758,319,0,0,0),
	 (680,26751,319,0,0,0),
	 (680,16734,319,0,0,0),
	 (680,29382,226,0,0,0),
	 (690,26240,319,0,0,0),
	 (690,26704,319,0,0,0),
	 (690,30786,226,0,0,0),
	 (690,10788,226,0,0,0),
	 (691,26706,319,0,0,0),
	 (691,26899,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,26900,319,0,0,0),
	 (691,27063,319,0,0,0),
	 (691,28065,319,0,0,0),
	 (691,28258,319,0,0,0),
	 (691,28582,319,0,0,0),
	 (691,28854,319,0,0,0),
	 (691,29031,319,0,0,0),
	 (691,29262,319,0,0,0),
	 (691,29446,319,0,0,0),
	 (691,29467,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,32238,319,0,0,0),
	 (691,16738,319,0,0,0),
	 (691,16831,319,0,0,0),
	 (691,32264,319,0,0,0),
	 (691,32349,319,0,0,0),
	 (691,32412,319,0,0,0),
	 (691,10122,226,0,0,0),
	 (691,10123,226,0,0,0),
	 (691,10135,226,0,0,0),
	 (691,10136,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,10144,226,0,0,0),
	 (691,10270,226,0,0,0),
	 (691,10305,226,0,0,0),
	 (691,11369,226,0,0,0),
	 (691,21608,226,0,0,0),
	 (691,21797,226,0,0,0),
	 (691,21798,226,0,0,0),
	 (691,21883,226,0,0,0),
	 (691,23535,226,0,0,0),
	 (691,12989,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,13034,319,0,0,0),
	 (691,13106,319,0,0,0),
	 (691,16215,319,0,0,0),
	 (691,16729,319,0,0,0),
	 (691,16735,319,0,0,0),
	 (691,3970,226,0,0,0),
	 (691,2915,319,0,0,0),
	 (691,16848,319,0,0,0),
	 (691,16849,319,0,0,0),
	 (691,16853,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,16854,319,0,0,0),
	 (691,16856,319,0,0,0),
	 (691,16860,319,0,0,0),
	 (691,16861,319,0,0,0),
	 (691,16862,319,0,0,0),
	 (691,16864,319,0,0,0),
	 (691,16865,319,0,0,0),
	 (691,16866,319,0,0,0),
	 (691,17138,319,0,0,0),
	 (691,17140,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,17393,319,0,0,0),
	 (691,19533,319,0,0,0),
	 (691,32219,319,0,0,0),
	 (691,3500,319,0,0,0),
	 (691,3858,319,0,0,0),
	 (691,11167,319,0,0,0),
	 (691,11168,319,0,0,0),
	 (691,11169,319,0,0,0),
	 (691,11170,319,0,0,0),
	 (691,11171,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (691,2252,226,0,0,0),
	 (691,24589,226,0,0,0),
	 (691,2335,226,0,0,0),
	 (691,2407,226,0,0,0),
	 (691,2573,226,0,0,0),
	 (691,10041,226,0,0,0),
	 (691,10119,226,0,0,0),
	 (691,10127,226,0,0,0),
	 (707,17609,383,0,0,0),
	 (707,17160,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (707,17161,383,0,0,0),
	 (707,16739,383,0,0,0),
	 (707,17588,383,0,0,0),
	 (707,17589,383,0,0,0),
	 (707,17590,383,0,0,0),
	 (707,17591,383,0,0,0),
	 (707,27668,383,0,0,0),
	 (707,27669,383,0,0,0),
	 (707,27670,383,0,0,0),
	 (702,17875,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (702,17605,383,0,0,0),
	 (702,27282,383,0,0,0),
	 (702,17146,383,0,0,0),
	 (702,17147,383,0,0,0),
	 (702,16740,383,0,0,0),
	 (702,13114,383,0,0,0),
	 (702,27280,383,0,0,0),
	 (702,17572,383,0,0,0),
	 (702,17573,383,0,0,0),
	 (702,17574,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (702,17575,383,0,0,0),
	 (702,28139,383,0,0,0),
	 (702,28144,383,0,0,0),
	 (702,28150,383,0,0,0),
	 (702,28571,383,0,0,0),
	 (703,17638,383,0,0,0),
	 (703,17610,383,0,0,0),
	 (703,28572,383,0,0,0),
	 (703,28140,383,0,0,0),
	 (703,28145,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (703,28149,383,0,0,0),
	 (703,27283,383,0,0,0),
	 (703,17144,383,0,0,0),
	 (703,17145,383,0,0,0),
	 (703,16741,383,0,0,0),
	 (703,13116,383,0,0,0),
	 (703,27281,383,0,0,0),
	 (703,17592,383,0,0,0),
	 (703,17593,383,0,0,0),
	 (703,17594,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (703,17595,383,0,0,0),
	 (708,17612,383,0,0,0),
	 (708,17148,383,0,0,0),
	 (708,17149,383,0,0,0),
	 (708,16742,383,0,0,0),
	 (708,17600,383,0,0,0),
	 (708,17601,383,0,0,0),
	 (708,17602,383,0,0,0),
	 (708,17603,383,0,0,0),
	 (708,27792,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (708,27793,383,0,0,0),
	 (708,27794,383,0,0,0),
	 (709,17880,383,0,0,0),
	 (709,17608,383,0,0,0),
	 (709,27795,383,0,0,0),
	 (709,27796,383,0,0,0),
	 (709,27797,383,0,0,0),
	 (709,17150,383,0,0,0),
	 (709,17151,383,0,0,0),
	 (709,16743,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (709,17584,383,0,0,0),
	 (709,17585,383,0,0,0),
	 (709,17586,383,0,0,0),
	 (709,17587,383,0,0,0),
	 (709,22848,383,0,0,0),
	 (704,17611,383,0,0,0),
	 (704,27284,383,0,0,0),
	 (704,17152,383,0,0,0),
	 (704,17153,383,0,0,0),
	 (704,16744,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (704,25706,383,0,0,0),
	 (704,13068,383,0,0,0),
	 (704,17596,383,0,0,0),
	 (704,17597,383,0,0,0),
	 (704,17598,383,0,0,0),
	 (704,17599,383,0,0,0),
	 (704,28141,383,0,0,0),
	 (704,28146,383,0,0,0),
	 (704,28151,383,0,0,0),
	 (704,26747,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (704,24767,383,0,0,0),
	 (704,25626,383,0,0,0),
	 (710,17606,383,0,0,0),
	 (710,17154,383,0,0,0),
	 (710,17155,383,0,0,0),
	 (710,16745,383,0,0,0),
	 (710,17576,383,0,0,0),
	 (710,17577,383,0,0,0),
	 (710,17578,383,0,0,0),
	 (710,17579,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (710,27798,383,0,0,0),
	 (710,27799,383,0,0,0),
	 (710,27800,383,0,0,0),
	 (710,25827,383,0,0,0),
	 (705,17876,383,0,0,0),
	 (705,17607,383,0,0,0),
	 (705,17156,383,0,0,0),
	 (705,17157,383,0,0,0),
	 (705,16746,383,0,0,0),
	 (705,24772,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (705,13113,383,0,0,0),
	 (705,17580,383,0,0,0),
	 (705,17581,383,0,0,0),
	 (705,17582,383,0,0,0),
	 (705,17583,383,0,0,0),
	 (705,28142,383,0,0,0),
	 (705,28147,383,0,0,0),
	 (705,28152,383,0,0,0),
	 (705,28573,383,0,0,0),
	 (701,17877,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (701,13115,383,0,0,0),
	 (701,17163,383,0,0,0),
	 (701,28574,383,0,0,0),
	 (701,28143,383,0,0,0),
	 (701,28148,383,0,0,0),
	 (701,28153,383,0,0,0),
	 (701,27285,383,0,0,0),
	 (701,17158,383,0,0,0),
	 (701,17159,383,0,0,0),
	 (701,16747,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (701,17164,383,0,0,0),
	 (701,17568,383,0,0,0),
	 (701,17569,383,0,0,0),
	 (701,17570,383,0,0,0),
	 (701,17571,383,0,0,0),
	 (701,18034,383,0,0,0),
	 (713,19668,383,0,0,0),
	 (713,19669,383,0,0,0),
	 (713,19670,383,0,0,0),
	 (713,19664,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (713,19665,383,0,0,0),
	 (713,19666,383,0,0,0),
	 (713,19667,383,0,0,0),
	 (706,27956,383,0,0,0),
	 (706,18026,383,0,0,0),
	 (706,18027,383,0,0,0),
	 (706,18028,383,0,0,0),
	 (706,18023,383,0,0,0),
	 (706,18031,383,0,0,0),
	 (706,18032,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (706,18033,383,0,0,0),
	 (706,18035,383,0,0,0),
	 (706,17879,383,0,0,0),
	 (706,27953,383,0,0,0),
	 (706,27954,383,0,0,0),
	 (706,27955,383,0,0,0),
	 (706,17878,383,0,0,0),
	 (706,28575,383,0,0,0),
	 (706,18000,383,0,0,0),
	 (706,18001,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (700,21744,383,0,0,0),
	 (700,21745,383,0,0,0),
	 (700,21746,383,0,0,0),
	 (700,21747,383,0,0,0),
	 (700,21748,383,0,0,0),
	 (700,21749,383,0,0,0),
	 (700,25385,383,0,0,0),
	 (700,25840,383,0,0,0),
	 (711,30460,383,0,0,0),
	 (711,30461,383,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (711,30462,383,0,0,0),
	 (710,1194,99,0,0,0),
	 (720,3740,99,0,0,0),
	 (720,3741,99,0,0,0),
	 (720,3739,99,0,0,0),
	 (730,3742,99,0,0,0),
	 (740,3743,99,0,0,0),
	 (750,10648,336,0,0,0),
	 (750,10665,336,0,0,0),
	 (750,10666,336,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (750,10667,336,0,0,0),
	 (750,10668,336,0,0,0),
	 (760,10766,336,0,0,0),
	 (760,10767,336,0,0,0),
	 (760,10768,336,0,0,0),
	 (760,10769,336,0,0,0),
	 (760,10770,336,0,0,0),
	 (770,1372,180,0,0,0),
	 (770,11580,336,0,0,0),
	 (800,3880,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (800,13835,226,0,0,0),
	 (800,14477,226,0,0,0),
	 (800,21609,226,0,0,0),
	 (800,16737,319,0,0,0),
	 (801,10140,226,0,0,0),
	 (801,10141,226,0,0,0),
	 (801,10142,226,0,0,0),
	 (801,10143,226,0,0,0),
	 (801,30799,226,0,0,0),
	 (801,30800,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (801,30801,226,0,0,0),
	 (801,30802,226,0,0,0),
	 (801,13836,226,0,0,0),
	 (800,3626,226,0,0,0),
	 (810,28066,226,0,0,0),
	 (810,9876,226,0,0,0),
	 (810,9877,226,0,0,0),
	 (810,9878,226,0,0,0),
	 (810,9879,226,0,0,0),
	 (810,9880,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (810,26232,226,0,0,0),
	 (810,26233,226,0,0,0),
	 (810,26234,226,0,0,0),
	 (820,28070,226,0,0,0),
	 (820,9882,226,0,0,0),
	 (820,9883,226,0,0,0),
	 (820,9884,226,0,0,0),
	 (820,9881,226,0,0,0),
	 (820,26235,226,0,0,0),
	 (820,26236,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (830,9885,226,0,0,0),
	 (830,9886,226,0,0,0),
	 (830,9888,226,0,0,0),
	 (830,9889,226,0,0,0),
	 (830,9890,226,0,0,0),
	 (830,9891,226,0,0,0),
	 (830,9892,226,0,0,0),
	 (830,9887,226,0,0,0),
	 (830,26237,226,0,0,0),
	 (830,26238,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (830,26239,226,0,0,0),
	 (840,9875,226,0,0,0),
	 (840,9893,226,0,0,0),
	 (840,9894,226,0,0,0),
	 (840,9895,226,0,0,0),
	 (840,9896,226,0,0,0),
	 (840,9897,226,0,0,0),
	 (840,9898,226,0,0,0),
	 (840,28071,226,0,0,0),
	 (840,26229,226,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (840,26230,226,0,0,0),
	 (840,26231,226,0,0,0),
	 (910,30440,226,0,0,0),
	 (910,30444,226,0,0,0),
	 (910,30446,226,0,0,0),
	 (910,30447,226,0,0,0),
	 (910,30448,226,0,0,0),
	 (910,30449,226,0,0,0),
	 (960,28227,319,0,0,0),
	 (960,28228,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (960,28234,319,0,0,0),
	 (960,28235,319,0,0,0),
	 (960,28246,319,0,0,0),
	 (960,28247,319,0,0,0),
	 (960,28248,319,0,0,0),
	 (960,26896,319,0,0,0),
	 (960,28249,319,0,0,0),
	 (960,28250,319,0,0,0),
	 (960,28251,319,0,0,0),
	 (960,28252,319,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (960,28253,319,0,0,0),
	 (960,28254,319,0,0,0),
	 (7000,17470,462,0,0,0),
	 (7000,17471,462,0,0,0),
	 (7001,17463,460,0,0,0),
	 (7001,17464,460,0,0,0),
	 (7002,17459,459,0,0,0),
	 (7002,17460,459,0,0,0),
	 (7003,17455,458,0,0,0),
	 (7003,17456,458,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7004,17867,469,0,0,0),
	 (7004,17868,469,0,0,0),
	 (7005,17452,457,0,0,0),
	 (7005,17453,457,0,0,0),
	 (7006,17448,456,0,0,0),
	 (7006,17449,456,0,0,0),
	 (7007,17444,455,0,0,0),
	 (7007,17445,455,0,0,0),
	 (7008,17440,454,0,0,0),
	 (7008,17441,454,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7009,17865,467,0,0,0),
	 (7009,17866,467,0,0,0),
	 (7010,17436,453,0,0,0),
	 (7010,17437,453,0,0,0),
	 (7011,17432,452,0,0,0),
	 (7011,17433,452,0,0,0),
	 (7012,17466,461,0,0,0),
	 (7012,17467,461,0,0,0),
	 (7013,17428,451,0,0,0),
	 (7013,17429,451,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7014,17425,450,0,0,0),
	 (7014,17426,450,0,0,0),
	 (7015,17869,468,0,0,0),
	 (7015,17870,468,0,0,0),
	 (7801,16263,465,0,0,0),
	 (7802,16264,465,0,0,0),
	 (7803,16265,465,0,0,0),
	 (7804,16266,465,0,0,0),
	 (7805,16267,465,0,0,0),
	 (7806,16268,465,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7807,16269,465,0,0,0),
	 (7808,17975,465,0,0,0),
	 (7809,16262,465,0,0,0),
	 (7810,28628,465,0,0,0),
	 (7811,17978,465,0,0,0),
	 (7812,17976,465,0,0,0),
	 (7813,17977,465,0,0,0),
	 (7814,28627,465,0,0,0),
	 (7901,25268,711,0,0,0),
	 (7902,25273,711,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7903,25274,711,0,0,0),
	 (7904,25275,711,0,0,0),
	 (7905,25276,711,0,0,0),
	 (7906,25277,711,0,0,0),
	 (7907,25278,711,0,0,0),
	 (7908,25279,711,0,0,0),
	 (7909,28629,711,0,0,0),
	 (7910,28630,711,0,0,0),
	 (7911,28694,711,0,0,0),
	 (7912,28695,711,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7913,28696,711,0,0,0),
	 (7914,28697,711,0,0,0),
	 (7915,28698,711,0,0,0),
	 (7916,28699,711,0,0,0),
	 (7917,28700,711,0,0,0),
	 (7918,28701,711,0,0,0),
	 (7921,30370,711,0,0,0),
	 (7922,30371,711,0,0,0),
	 (7923,30372,711,0,0,0),
	 (7924,30373,711,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7925,30374,711,0,0,0),
	 (7926,30375,711,0,0,0),
	 (7927,30376,711,0,0,0),
	 (7928,30377,711,0,0,0),
	 (7929,30378,711,0,0,0),
	 (7100,0,0,0,0,0),
	 (7101,0,0,0,0,0),
	 (7102,0,0,0,0,0),
	 (7103,0,0,0,0,0),
	 (7104,0,0,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7105,0,0,0,0,0),
	 (7106,0,0,0,0,0),
	 (7107,0,0,0,0,0),
	 (7108,0,0,0,0,0),
	 (7109,0,0,0,0,0),
	 (7110,0,0,0,0,0),
	 (7111,0,0,0,0,0),
	 (7112,0,0,0,0,0),
	 (7113,0,0,0,0,0),
	 (7114,0,0,0,0,0);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (7115,0,0,0,0,0),
	 (1610,17395,319,15000,150,12000),
	 (1610,10120,226,10000,150,20000),
	 (1610,17395,319,21000,150,15000),
	 (1610,10120,226,14000,150,18000),
	 (1610,18015,319,2100,150,2000),
	 (1611,2612,226,20100,150,10000),
	 (1611,3626,226,40100,150,30000),
	 (1612,13835,226,21000,150,20000),
	 (1612,11167,319,-20000,150,12000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1612,30802,226,24000,150,25000),
	 (1612,10141,226,-20100,150,12000),
	 (1613,16736,319,10000,150,10000),
	 (1613,18033,383,21000,150,20000),
	 (1613,18033,383,-21000,150,20000),
	 (1613,18033,383,21000,150,-20000),
	 (1613,18033,383,-21000,150,-20000),
	 (1614,10121,226,2100,150,2000),
	 (1614,27953,383,-21000,150,20000),
	 (1614,27953,383,-21000,150,-20000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1614,16839,319,3500,1500,5000),
	 (1614,16843,319,35000,1500,50000),
	 (1614,27063,319,35000,1500,-50000),
	 (1615,18012,319,3500,1500,5000),
	 (1616,18012,319,3500,1500,5000),
	 (1617,18012,319,3500,1500,5000),
	 (1618,16736,319,6500,200,5000),
	 (1618,18015,319,2100,150,2000),
	 (1618,18017,319,2500,300,3500),
	 (1618,10120,226,-2500,800,4500);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1619,18012,319,3500,1500,5000),
	 (1619,18015,319,2100,150,2000),
	 (1619,18017,319,2500,300,3500),
	 (1619,16733,319,55000,-2200,90000),
	 (1619,18019,319,-3000,-500,-2500),
	 (1619,18013,319,3000,100,2000),
	 (1619,23767,227,100,100,100),
	 (1619,18016,319,5000,-150,2500),
	 (1621,18015,319,2100,150,2000),
	 (1621,18017,319,2500,300,3500);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1621,18021,319,-3000,50,2500),
	 (1621,10121,226,-2500,800,4500),
	 (1622,18015,319,2100,150,2000),
	 (1622,18017,319,2500,300,3500),
	 (1622,18019,319,-3000,-500,-2500),
	 (1622,2612,226,-5000,800,4500),
	 (1623,18015,319,2100,150,2000),
	 (1623,18017,319,2500,300,3500),
	 (1623,18019,319,-3000,-500,-2500),
	 (1623,18018,319,300,250,400);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1623,10120,226,-2500,800,4500),
	 (1624,18015,319,2100,150,2000),
	 (1624,18017,319,2500,300,3500),
	 (1624,18019,319,-3000,-500,-2500),
	 (1624,18013,319,3000,100,2000),
	 (1624,18018,319,300,250,400),
	 (1624,30436,226,-2100,600,3000),
	 (1625,18012,319,3500,1500,5000),
	 (1625,18015,319,2100,150,2000),
	 (1625,18017,319,2500,300,3500);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1625,18019,319,-3000,-500,-2500),
	 (1625,18013,319,3000,100,2000),
	 (1625,23767,227,-100,100,100),
	 (1625,30436,226,-2100,600,3000),
	 (1626,18012,319,3500,1500,5000),
	 (1626,18015,319,2100,150,2000),
	 (1626,18017,319,2500,300,3500),
	 (1626,18019,319,-3000,-500,-2500),
	 (1626,18013,319,3000,100,2000),
	 (1626,23767,227,100,100,100);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1626,18014,319,3500,0,2000),
	 (1626,30436,226,-2100,600,3000),
	 (1626,16736,319,6500,200,5000),
	 (1627,18012,319,3500,1500,5000),
	 (1627,18015,319,2100,150,2000),
	 (1627,18017,319,2500,300,3500),
	 (1627,18019,319,-3000,-500,-2500),
	 (1627,18013,319,3000,100,2000),
	 (1627,23767,227,100,100,100),
	 (1627,18016,319,5000,-150,2500);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1627,18014,319,3500,0,2000),
	 (1627,16736,319,6500,200,5000),
	 (1627,10121,226,-2500,800,4500),
	 (1628,18012,319,3500,1500,5000),
	 (1628,18015,319,2100,150,2000),
	 (1628,18017,319,2500,300,3500),
	 (1628,16732,319,75000,2000,80000),
	 (1628,18019,319,-3000,-500,-2500),
	 (1628,18013,319,3000,100,2000),
	 (1628,23767,227,100,100,100);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1628,18016,319,5000,-150,2500),
	 (1628,18014,319,3500,0,2000),
	 (1628,16736,319,6500,200,5000),
	 (1628,10121,226,-2500,800,4500),
	 (1629,18012,319,3500,1500,5000),
	 (1629,18015,319,2100,150,2000),
	 (1629,18017,319,2500,300,3500),
	 (1629,16733,319,55000,-2200,90000),
	 (1629,18019,319,-3000,-500,-2500),
	 (1629,18013,319,3000,100,2000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (1629,23767,227,100,100,100),
	 (1629,18016,319,5000,-150,2500),
	 (1629,18014,319,3500,0,2000),
	 (1629,4011,319,7000,-250,5000),
	 (1629,30436,226,-2100,600,3000),
	 (1629,16736,319,6500,200,5000),
	 (4101,29720,306,10000,1200,5000),
	 (4102,29725,306,5000,1200,10000),
	 (4103,29730,306,0,-1200,10000),
	 (4104,29735,306,10000,-1200,10000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (4105,29736,306,2500,1500,24000),
	 (4106,29745,306,15000,200,15000),
	 (4107,29898,306,18000,100,20000),
	 (4108,29903,306,20000,15000,18000),
	 (4201,29716,306,10000,1200,5000),
	 (4202,29721,306,5000,1200,10000),
	 (4203,29726,306,0,-1200,10000),
	 (4204,29731,306,10000,-1200,10000),
	 (4205,29740,306,2500,1500,24000),
	 (4206,29741,306,15000,200,15000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (4207,29894,306,18000,100,20000),
	 (4208,29899,306,20000,15000,18000),
	 (4301,29717,306,10000,1200,5000),
	 (4302,29722,306,5000,1200,10000),
	 (4303,29727,306,0,-1200,10000),
	 (4304,29732,306,10000,-1200,10000),
	 (4305,29739,306,2500,1500,24000),
	 (4306,29742,306,15000,200,15000),
	 (4307,29895,306,18000,100,20000),
	 (4308,29900,306,20000,15000,18000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (4401,29718,306,10000,1200,5000),
	 (4402,29723,306,5000,1200,10000),
	 (4403,29728,306,0,-1200,10000),
	 (4404,29733,306,10000,-1200,10000),
	 (4405,29738,306,2500,1500,24000),
	 (4406,29743,306,15000,200,15000),
	 (4407,29896,306,18000,100,20000),
	 (4408,29901,306,20000,15000,18000),
	 (4501,29719,306,10000,1200,5000),
	 (4502,29724,306,5000,1200,10000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (4503,29729,306,0,-1200,10000),
	 (4504,29734,306,10000,-1200,10000),
	 (4505,29737,306,2500,1500,24000),
	 (4506,29744,306,15000,200,15000),
	 (4507,29897,306,18000,100,20000),
	 (4508,29902,306,20000,15000,18000),
	 (5101,26644,306,10000,1200,5000),
	 (5102,26648,306,5000,1200,10000),
	 (5103,26652,306,0,-1200,10000),
	 (5104,26656,306,10000,-1200,10000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (5105,26674,306,2500,1500,24000),
	 (5106,26675,306,15000,200,15000),
	 (5107,26676,306,18000,100,20000),
	 (5108,26677,306,20000,15000,18000),
	 (5201,26612,306,10000,1200,5000),
	 (5202,26613,306,5000,1200,10000),
	 (5203,26614,306,0,-1200,10000),
	 (5204,26615,306,10000,-1200,10000),
	 (5205,26657,306,2500,1500,24000),
	 (5206,26658,306,15000,200,15000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (5207,26659,306,18000,100,20000),
	 (5208,26660,306,20000,15000,18000),
	 (5301,26641,306,10000,1200,5000),
	 (5302,26645,306,5000,1200,10000),
	 (5303,26649,306,0,-1200,10000),
	 (5304,26653,306,10000,-1200,10000),
	 (5305,26662,306,2500,1500,24000),
	 (5306,26663,306,15000,200,15000),
	 (5307,26664,306,18000,100,20000),
	 (5308,26665,306,20000,15000,18000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (5401,26642,306,10000,1200,5000),
	 (5402,26646,306,5000,1200,10000),
	 (5403,26650,306,0,-1200,10000),
	 (5404,26654,306,10000,-1200,10000),
	 (5405,26666,306,2500,1500,24000),
	 (5406,26667,306,15000,200,15000),
	 (5407,26668,306,18000,100,20000),
	 (5408,26669,306,20000,15000,18000),
	 (5501,26643,306,10000,1200,5000),
	 (5502,26647,306,5000,1200,10000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (5503,26651,306,0,-1200,10000),
	 (5504,26655,306,10000,-1200,10000),
	 (5505,26670,306,2500,1500,24000),
	 (5506,26671,306,15000,200,15000),
	 (5507,26672,306,18000,100,20000),
	 (5508,26673,306,20000,15000,18000),
	 (6101,26634,306,15000,150,12000),
	 (6102,26632,306,10000,150,20000),
	 (6103,26635,306,21000,150,15000),
	 (6104,26633,306,14000,150,18000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (6201,26608,306,15000,150,12000),
	 (6202,26609,306,10000,150,20000),
	 (6203,26610,306,21000,150,15000),
	 (6204,26611,306,14000,150,18000),
	 (6301,26620,306,15000,150,12000),
	 (6302,26621,306,10000,150,20000),
	 (6303,26622,306,21000,150,15000),
	 (6304,26623,306,14000,150,18000),
	 (6401,26627,306,15000,150,12000),
	 (6402,26624,306,10000,150,20000);
INSERT INTO evemu.dunGroupData (dunGroupID,itemTypeID,itemGroupID,xpos,ypos,zpos) VALUES
	 (6403,26626,306,21000,150,15000),
	 (6404,26625,306,14000,150,18000),
	 (6501,26628,306,15000,150,12000),
	 (6502,26631,306,10000,150,20000),
	 (6503,26629,306,21000,150,15000),
	 (6504,26630,306,14000,150,18000);

INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (1000,1000,0,0,0),
	 (1100,1010,0,0,0),
	 (1200,1020,0,0,0),
	 (1300,1030,0,0,0),
	 (1400,1040,0,0,0),
	 (1500,1050,0,0,0),
	 (1600,1060,0,0,0),
	 (1700,1070,0,0,0),
	 (1800,1080,0,0,0),
	 (1900,1090,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (1010,1610,0,0,0),
	 (1110,1611,0,0,0),
	 (1210,1612,0,0,0),
	 (1310,1613,0,0,0),
	 (1410,1614,0,0,0),
	 (1510,1615,0,0,0),
	 (1610,1616,0,0,0),
	 (1710,1617,0,0,0),
	 (1810,1618,0,0,0),
	 (1910,1619,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (1120,1621,0,0,0),
	 (1220,1622,0,0,0),
	 (1320,1623,0,0,0),
	 (1420,1624,0,0,0),
	 (1520,1625,0,0,0),
	 (1620,1626,0,0,0),
	 (1720,1627,0,0,0),
	 (1820,1628,0,0,0),
	 (1920,1629,0,0,0),
	 (4111,4101,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4211,4102,0,0,0),
	 (4311,4103,0,0,0),
	 (4411,4104,0,0,0),
	 (4511,4105,0,0,0),
	 (4611,4106,0,0,0),
	 (4711,4107,0,0,0),
	 (4811,4108,0,0,0),
	 (4121,4101,0,0,0),
	 (4221,4102,0,0,0),
	 (4321,4103,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4421,4104,0,0,0),
	 (4521,4105,0,0,0),
	 (4621,4106,0,0,0),
	 (4721,4107,0,0,0),
	 (4821,4108,0,0,0),
	 (4131,4101,0,0,0),
	 (4231,4102,0,0,0),
	 (4331,4103,0,0,0),
	 (4431,4104,0,0,0),
	 (4531,4105,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4631,4106,0,0,0),
	 (4731,4107,0,0,0),
	 (4831,4108,0,0,0),
	 (4112,4201,0,0,0),
	 (4212,4202,0,0,0),
	 (4312,4203,0,0,0),
	 (4412,4204,0,0,0),
	 (4512,4205,0,0,0),
	 (4612,4206,0,0,0),
	 (4712,4207,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4812,4208,0,0,0),
	 (4122,4201,0,0,0),
	 (4222,4202,0,0,0),
	 (4322,4203,0,0,0),
	 (4422,4204,0,0,0),
	 (4522,4205,0,0,0),
	 (4622,4206,0,0,0),
	 (4722,4207,0,0,0),
	 (4822,4208,0,0,0),
	 (4132,4201,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4232,4202,0,0,0),
	 (4332,4203,0,0,0),
	 (4432,4204,0,0,0),
	 (4532,4205,0,0,0),
	 (4632,4206,0,0,0),
	 (4732,4207,0,0,0),
	 (4832,4208,0,0,0),
	 (4113,4301,0,0,0),
	 (4213,4302,0,0,0),
	 (4313,4303,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4413,4304,0,0,0),
	 (4513,4305,0,0,0),
	 (4613,4306,0,0,0),
	 (4713,4307,0,0,0),
	 (4813,4308,0,0,0),
	 (4123,4301,0,0,0),
	 (4223,4302,0,0,0),
	 (4323,4303,0,0,0),
	 (4423,4304,0,0,0),
	 (4523,4305,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4623,4306,0,0,0),
	 (4723,4307,0,0,0),
	 (4823,4308,0,0,0),
	 (4133,4301,0,0,0),
	 (4233,4302,0,0,0),
	 (4333,4303,0,0,0),
	 (4433,4304,0,0,0),
	 (4533,4305,0,0,0),
	 (4633,4306,0,0,0),
	 (4733,4307,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4833,4308,0,0,0),
	 (4114,4401,0,0,0),
	 (4214,4402,0,0,0),
	 (4314,4403,0,0,0),
	 (4414,4404,0,0,0),
	 (4514,4405,0,0,0),
	 (4614,4406,0,0,0),
	 (4714,4407,0,0,0),
	 (4814,4408,0,0,0),
	 (4124,4401,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4224,4402,0,0,0),
	 (4324,4403,0,0,0),
	 (4424,4404,0,0,0),
	 (4524,4405,0,0,0),
	 (4624,4406,0,0,0),
	 (4724,4407,0,0,0),
	 (4824,4408,0,0,0),
	 (4134,4401,0,0,0),
	 (4234,4402,0,0,0),
	 (4334,4403,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4434,4404,0,0,0),
	 (4534,4405,0,0,0),
	 (4634,4406,0,0,0),
	 (4734,4407,0,0,0),
	 (4834,4408,0,0,0),
	 (4115,4501,0,0,0),
	 (4215,4502,0,0,0),
	 (4315,4503,0,0,0),
	 (4415,4504,0,0,0),
	 (4515,4505,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4615,4506,0,0,0),
	 (4715,4507,0,0,0),
	 (4815,4508,0,0,0),
	 (4125,4501,0,0,0),
	 (4225,4502,0,0,0),
	 (4325,4503,0,0,0),
	 (4425,4504,0,0,0),
	 (4525,4505,0,0,0),
	 (4625,4506,0,0,0),
	 (4725,4507,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (4825,4508,0,0,0),
	 (4135,4501,0,0,0),
	 (4235,4502,0,0,0),
	 (4335,4503,0,0,0),
	 (4435,4504,0,0,0),
	 (4535,4505,0,0,0),
	 (4635,4506,0,0,0),
	 (4735,4507,0,0,0),
	 (4835,4508,0,0,0),
	 (5111,5101,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5211,5102,0,0,0),
	 (5311,5103,0,0,0),
	 (5411,5104,0,0,0),
	 (5511,5105,0,0,0),
	 (5611,5106,0,0,0),
	 (5711,5107,0,0,0),
	 (5811,5108,0,0,0),
	 (5121,5101,0,0,0),
	 (5221,5102,0,0,0),
	 (5321,5103,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5421,5104,0,0,0),
	 (5521,5105,0,0,0),
	 (5621,5106,0,0,0),
	 (5721,5107,0,0,0),
	 (5821,5108,0,0,0),
	 (5131,5101,0,0,0),
	 (5231,5102,0,0,0),
	 (5331,5103,0,0,0),
	 (5431,5104,0,0,0),
	 (5531,5105,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5631,5106,0,0,0),
	 (5731,5107,0,0,0),
	 (5831,5108,0,0,0),
	 (5112,5201,0,0,0),
	 (5212,5202,0,0,0),
	 (5312,5203,0,0,0),
	 (5412,5204,0,0,0),
	 (5512,5205,0,0,0),
	 (5612,5206,0,0,0),
	 (5712,5207,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5812,5208,0,0,0),
	 (5122,5201,0,0,0),
	 (5222,5202,0,0,0),
	 (5322,5203,0,0,0),
	 (5422,5204,0,0,0),
	 (5522,5205,0,0,0),
	 (5622,5206,0,0,0),
	 (5722,5207,0,0,0),
	 (5822,5208,0,0,0),
	 (5132,5201,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5232,5202,0,0,0),
	 (5332,5203,0,0,0),
	 (5432,5204,0,0,0),
	 (5532,5205,0,0,0),
	 (5632,5206,0,0,0),
	 (5732,5207,0,0,0),
	 (5832,5208,0,0,0),
	 (5113,5301,0,0,0),
	 (5213,5302,0,0,0),
	 (5313,5303,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5413,5304,0,0,0),
	 (5513,5305,0,0,0),
	 (5613,5306,0,0,0),
	 (5713,5307,0,0,0),
	 (5813,5308,0,0,0),
	 (5123,5301,0,0,0),
	 (5223,5302,0,0,0),
	 (5323,5303,0,0,0),
	 (5423,5304,0,0,0),
	 (5523,5305,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5623,5306,0,0,0),
	 (5723,5307,0,0,0),
	 (5823,5308,0,0,0),
	 (5133,5301,0,0,0),
	 (5233,5302,0,0,0),
	 (5333,5303,0,0,0),
	 (5433,5304,0,0,0),
	 (5533,5305,0,0,0),
	 (5633,5306,0,0,0),
	 (5733,5307,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5833,5308,0,0,0),
	 (5114,5401,0,0,0),
	 (5214,5402,0,0,0),
	 (5314,5403,0,0,0),
	 (5414,5404,0,0,0),
	 (5514,5405,0,0,0),
	 (5614,5406,0,0,0),
	 (5714,5407,0,0,0),
	 (5814,5408,0,0,0),
	 (5124,5401,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5224,5402,0,0,0),
	 (5324,5403,0,0,0),
	 (5424,5404,0,0,0),
	 (5524,5405,0,0,0),
	 (5624,5406,0,0,0),
	 (5724,5407,0,0,0),
	 (5824,5408,0,0,0),
	 (5134,5401,0,0,0),
	 (5234,5402,0,0,0),
	 (5334,5403,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5434,5404,0,0,0),
	 (5534,5405,0,0,0),
	 (5634,5406,0,0,0),
	 (5734,5407,0,0,0),
	 (5834,5408,0,0,0),
	 (5115,5501,0,0,0),
	 (5215,5502,0,0,0),
	 (5315,5503,0,0,0),
	 (5415,5504,0,0,0),
	 (5515,5505,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5615,5506,0,0,0),
	 (5715,5507,0,0,0),
	 (5815,5508,0,0,0),
	 (5125,5501,0,0,0),
	 (5225,5502,0,0,0),
	 (5325,5503,0,0,0),
	 (5425,5504,0,0,0),
	 (5525,5505,0,0,0),
	 (5625,5506,0,0,0),
	 (5725,5507,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (5825,5508,0,0,0),
	 (5135,5501,0,0,0),
	 (5235,5502,0,0,0),
	 (5335,5503,0,0,0),
	 (5435,5504,0,0,0),
	 (5535,5505,0,0,0),
	 (5635,5506,0,0,0),
	 (5735,5507,0,0,0),
	 (5835,5508,0,0,0),
	 (6111,6101,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6211,6101,0,0,0),
	 (6311,6102,0,0,0),
	 (6411,6102,0,0,0),
	 (6511,6103,0,0,0),
	 (6611,6103,0,0,0),
	 (6711,6104,0,0,0),
	 (6811,6104,0,0,0),
	 (6121,6101,0,0,0),
	 (6221,6101,0,0,0),
	 (6321,6102,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6421,6102,0,0,0),
	 (6521,6103,0,0,0),
	 (6621,6103,0,0,0),
	 (6721,6104,0,0,0),
	 (6821,6104,0,0,0),
	 (6131,6101,0,0,0),
	 (6231,6101,0,0,0),
	 (6331,6102,0,0,0),
	 (6431,6102,0,0,0),
	 (6531,6103,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6631,6103,0,0,0),
	 (6731,6104,0,0,0),
	 (6831,6104,0,0,0),
	 (6112,6201,0,0,0),
	 (6212,6201,0,0,0),
	 (6312,6202,0,0,0),
	 (6412,6202,0,0,0),
	 (6512,6203,0,0,0),
	 (6612,6203,0,0,0),
	 (6712,6204,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6812,6204,0,0,0),
	 (6122,6201,0,0,0),
	 (6222,6201,0,0,0),
	 (6322,6202,0,0,0),
	 (6422,6202,0,0,0),
	 (6522,6203,0,0,0),
	 (6622,6203,0,0,0),
	 (6722,6204,0,0,0),
	 (6822,6204,0,0,0),
	 (6132,6201,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6232,6201,0,0,0),
	 (6332,6202,0,0,0),
	 (6432,6202,0,0,0),
	 (6532,6203,0,0,0),
	 (6632,6203,0,0,0),
	 (6732,6204,0,0,0),
	 (6832,6204,0,0,0),
	 (6113,6301,0,0,0),
	 (6213,6301,0,0,0),
	 (6313,6302,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6413,6302,0,0,0),
	 (6513,6303,0,0,0),
	 (6613,6303,0,0,0),
	 (6713,6304,0,0,0),
	 (6813,6304,0,0,0),
	 (6123,6301,0,0,0),
	 (6223,6301,0,0,0),
	 (6323,6302,0,0,0),
	 (6423,6302,0,0,0),
	 (6523,6303,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6623,6303,0,0,0),
	 (6723,6304,0,0,0),
	 (6823,6304,0,0,0),
	 (6133,6301,0,0,0),
	 (6233,6301,0,0,0),
	 (6333,6302,0,0,0),
	 (6433,6302,0,0,0),
	 (6533,6303,0,0,0),
	 (6633,6303,0,0,0),
	 (6733,6304,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6833,6304,0,0,0),
	 (6114,6401,0,0,0),
	 (6214,6401,0,0,0),
	 (6314,6402,0,0,0),
	 (6414,6402,0,0,0),
	 (6514,6403,0,0,0),
	 (6614,6403,0,0,0),
	 (6714,6404,0,0,0),
	 (6814,6404,0,0,0),
	 (6124,6401,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6224,6401,0,0,0),
	 (6324,6402,0,0,0),
	 (6424,6402,0,0,0),
	 (6524,6403,0,0,0),
	 (6624,6403,0,0,0),
	 (6724,6404,0,0,0),
	 (6824,6404,0,0,0),
	 (6134,6401,0,0,0),
	 (6234,6401,0,0,0),
	 (6334,6402,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6434,6402,0,0,0),
	 (6534,6403,0,0,0),
	 (6634,6403,0,0,0),
	 (6734,6404,0,0,0),
	 (6834,6404,0,0,0),
	 (6115,6501,0,0,0),
	 (6215,6501,0,0,0),
	 (6315,6502,0,0,0),
	 (6415,6502,0,0,0),
	 (6515,6503,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6615,6503,0,0,0),
	 (6715,6504,0,0,0),
	 (6815,6504,0,0,0),
	 (6125,6501,0,0,0),
	 (6225,6501,0,0,0),
	 (6325,6502,0,0,0),
	 (6425,6502,0,0,0),
	 (6525,6503,0,0,0),
	 (6625,6503,0,0,0),
	 (6725,6504,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (6825,6504,0,0,0),
	 (6135,6501,0,0,0),
	 (6235,6501,0,0,0),
	 (6335,6502,0,0,0),
	 (6435,6502,0,0,0),
	 (6535,6503,0,0,0),
	 (6635,6503,0,0,0),
	 (6735,6504,0,0,0),
	 (6835,6504,0,0,0),
	 (7011,7000,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7011,7000,0,0,0),
	 (7011,7000,0,0,0),
	 (7012,7001,0,0,0),
	 (7012,7001,0,0,0),
	 (7012,7001,0,0,0),
	 (7013,7002,0,0,0),
	 (7013,7002,0,0,0),
	 (7013,7002,0,0,0),
	 (7014,7003,0,0,0),
	 (7014,7003,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7014,7003,0,0,0),
	 (7015,7001,0,0,0),
	 (7015,7000,0,0,0),
	 (7015,7000,0,0,0),
	 (7015,7001,0,0,0),
	 (7015,7001,0,0,0),
	 (7015,7000,0,0,0),
	 (7021,7000,0,0,0),
	 (7021,7000,0,0,0),
	 (7021,7000,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7021,7000,0,0,0),
	 (7021,7000,0,0,0),
	 (7022,7001,0,0,0),
	 (7022,7001,0,0,0),
	 (7022,7001,0,0,0),
	 (7022,7001,0,0,0),
	 (7022,7001,0,0,0),
	 (7023,7002,0,0,0),
	 (7023,7002,0,0,0),
	 (7023,7002,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7023,7002,0,0,0),
	 (7023,7002,0,0,0),
	 (7024,7003,0,0,0),
	 (7024,7003,0,0,0),
	 (7024,7003,0,0,0),
	 (7024,7003,0,0,0),
	 (7024,7003,0,0,0),
	 (7025,7001,0,0,0),
	 (7025,7000,0,0,0),
	 (7025,7000,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7025,7000,0,0,0),
	 (7025,7001,0,0,0),
	 (7025,7001,0,0,0),
	 (7025,7000,0,0,0),
	 (7025,7001,0,0,0),
	 (7025,7000,0,0,0),
	 (7025,7001,0,0,0),
	 (7031,7000,0,0,0),
	 (7031,7000,0,0,0),
	 (7031,7000,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7031,7000,0,0,0),
	 (7031,7000,0,0,0),
	 (7031,7000,0,0,0),
	 (7031,7000,0,0,0),
	 (7032,7001,0,0,0),
	 (7032,7001,0,0,0),
	 (7032,7001,0,0,0),
	 (7032,7001,0,0,0),
	 (7032,7001,0,0,0),
	 (7032,7001,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7032,7001,0,0,0),
	 (7033,7002,0,0,0),
	 (7033,7002,0,0,0),
	 (7033,7002,0,0,0),
	 (7033,7002,0,0,0),
	 (7033,7002,0,0,0),
	 (7033,7002,0,0,0),
	 (7033,7002,0,0,0),
	 (7034,7003,0,0,0),
	 (7034,7003,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7034,7003,0,0,0),
	 (7034,7003,0,0,0),
	 (7034,7003,0,0,0),
	 (7034,7003,0,0,0),
	 (7034,7003,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7001,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7001,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7035,7001,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7001,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7001,0,0,0),
	 (7035,7001,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7000,0,0,0),
	 (7035,7001,0,0,0),
	 (7110,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7110,7010,0,0,0),
	 (7110,7009,0,0,0),
	 (7110,7010,0,0,0),
	 (7110,7011,0,0,0),
	 (7110,1110,0,0,0),
	 (7110,7010,0,0,0),
	 (7110,7014,0,0,0),
	 (7110,7009,0,0,0),
	 (7110,7009,0,0,0),
	 (7110,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7110,7014,0,0,0),
	 (7110,7009,0,0,0),
	 (7110,7013,0,0,0),
	 (7110,7013,0,0,0),
	 (7110,7013,0,0,0),
	 (7110,7011,0,0,0),
	 (7110,7015,0,0,0),
	 (7110,7012,0,0,0),
	 (7110,7012,0,0,0),
	 (7110,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7110,7014,0,0,0),
	 (7111,7004,0,0,0),
	 (7111,7004,0,0,0),
	 (7111,7004,0,0,0),
	 (7112,7004,0,0,0),
	 (7112,7005,0,0,0),
	 (7112,7004,0,0,0),
	 (7112,7004,0,0,0),
	 (7112,7005,0,0,0),
	 (7112,7005,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7113,7004,0,0,0),
	 (7113,7006,0,0,0),
	 (7113,7006,0,0,0),
	 (7113,7004,0,0,0),
	 (7113,7005,0,0,0),
	 (7113,7004,0,0,0),
	 (7113,7006,0,0,0),
	 (7113,7005,0,0,0),
	 (7113,7005,0,0,0),
	 (7121,7004,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7121,7004,0,0,0),
	 (7121,7004,0,0,0),
	 (7121,7004,0,0,0),
	 (7121,7004,0,0,0),
	 (7122,7004,0,0,0),
	 (7122,7005,0,0,0),
	 (7122,7004,0,0,0),
	 (7122,7005,0,0,0),
	 (7122,7004,0,0,0),
	 (7122,7005,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7122,7004,0,0,0),
	 (7122,7004,0,0,0),
	 (7122,7005,0,0,0),
	 (7122,7005,0,0,0),
	 (7123,7004,0,0,0),
	 (7123,7004,0,0,0),
	 (7123,7005,0,0,0),
	 (7123,7006,0,0,0),
	 (7123,7006,0,0,0),
	 (7123,7006,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7123,7005,0,0,0),
	 (7123,7004,0,0,0),
	 (7123,7006,0,0,0),
	 (7123,7005,0,0,0),
	 (7123,7006,0,0,0),
	 (7123,7004,0,0,0),
	 (7123,7005,0,0,0),
	 (7123,7005,0,0,0),
	 (7123,7004,0,0,0),
	 (7131,7004,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7131,7004,0,0,0),
	 (7131,7004,0,0,0),
	 (7131,7004,0,0,0),
	 (7131,7004,0,0,0),
	 (7131,7004,0,0,0),
	 (7131,7004,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7005,0,0,0),
	 (7132,7005,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7132,7005,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7005,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7005,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7005,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7004,0,0,0),
	 (7132,7005,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7133,7004,0,0,0),
	 (7133,7005,0,0,0),
	 (7133,7004,0,0,0),
	 (7133,7006,0,0,0),
	 (7133,7006,0,0,0),
	 (7133,7005,0,0,0),
	 (7133,7004,0,0,0),
	 (7133,7005,0,0,0),
	 (7133,7006,0,0,0),
	 (7133,7004,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7133,7005,0,0,0),
	 (7133,7006,0,0,0),
	 (7133,7004,0,0,0),
	 (7133,7005,0,0,0),
	 (7133,7006,0,0,0),
	 (7133,7005,0,0,0),
	 (7133,7006,0,0,0),
	 (7133,7005,0,0,0),
	 (7133,7004,0,0,0),
	 (7133,7006,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7133,7004,0,0,0),
	 (7211,7005,0,0,0),
	 (7211,7006,0,0,0),
	 (7211,7005,0,0,0),
	 (7211,7007,0,0,0),
	 (7211,7007,0,0,0),
	 (7211,7006,0,0,0),
	 (7211,7006,0,0,0),
	 (7211,7005,0,0,0),
	 (7211,7007,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7212,7007,0,0,0),
	 (7212,7008,0,0,0),
	 (7212,7008,0,0,0),
	 (7212,7007,0,0,0),
	 (7212,7006,0,0,0),
	 (7212,7006,0,0,0),
	 (7212,7006,0,0,0),
	 (7212,7008,0,0,0),
	 (7212,7007,0,0,0),
	 (7213,7008,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7213,7007,0,0,0),
	 (7213,7007,0,0,0),
	 (7213,7008,0,0,0),
	 (7213,7008,0,0,0),
	 (7213,7007,0,0,0),
	 (7214,7009,0,0,0),
	 (7214,7009,0,0,0),
	 (7214,7009,0,0,0),
	 (7220,7015,0,0,0),
	 (7220,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7220,7010,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7010,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7015,0,0,0),
	 (7220,7011,0,0,0),
	 (7220,7014,0,0,0),
	 (7220,7014,0,0,0),
	 (7220,7014,0,0,0),
	 (7220,7013,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7220,7013,0,0,0),
	 (7220,7011,0,0,0),
	 (7220,7011,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7013,0,0,0),
	 (7220,7011,0,0,0),
	 (7220,7011,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7012,0,0,0),
	 (7220,7012,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7220,7009,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7012,0,0,0),
	 (7220,7012,0,0,0),
	 (7220,7009,0,0,0),
	 (7220,7012,0,0,0),
	 (7220,7012,0,0,0),
	 (7220,7009,0,0,0),
	 (7221,7006,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7221,7005,0,0,0),
	 (7221,7006,0,0,0),
	 (7221,7005,0,0,0),
	 (7221,7007,0,0,0),
	 (7221,7007,0,0,0),
	 (7221,7005,0,0,0),
	 (7221,7005,0,0,0),
	 (7221,7007,0,0,0),
	 (7221,7006,0,0,0),
	 (7221,7006,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7221,7007,0,0,0),
	 (7221,7006,0,0,0),
	 (7221,7005,0,0,0),
	 (7221,7007,0,0,0),
	 (7222,7008,0,0,0),
	 (7222,7006,0,0,0),
	 (7222,7007,0,0,0),
	 (7222,7007,0,0,0),
	 (7222,7006,0,0,0),
	 (7222,7006,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7222,7008,0,0,0),
	 (7222,7008,0,0,0),
	 (7222,7008,0,0,0),
	 (7222,7006,0,0,0),
	 (7222,7007,0,0,0),
	 (7222,7008,0,0,0),
	 (7222,7007,0,0,0),
	 (7222,7007,0,0,0),
	 (7222,7006,0,0,0),
	 (7223,7008,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7223,7007,0,0,0),
	 (7223,7008,0,0,0),
	 (7223,7007,0,0,0),
	 (7223,7007,0,0,0),
	 (7223,7007,0,0,0),
	 (7223,7008,0,0,0),
	 (7223,7008,0,0,0),
	 (7223,7008,0,0,0),
	 (7223,7007,0,0,0),
	 (7224,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7224,7009,0,0,0),
	 (7224,7009,0,0,0),
	 (7224,7009,0,0,0),
	 (7224,7009,0,0,0),
	 (7231,7006,0,0,0),
	 (7231,7007,0,0,0),
	 (7231,7005,0,0,0),
	 (7231,7005,0,0,0),
	 (7231,7006,0,0,0),
	 (7231,7005,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7231,7006,0,0,0),
	 (7231,7005,0,0,0),
	 (7231,7007,0,0,0),
	 (7231,7007,0,0,0),
	 (7231,7007,0,0,0),
	 (7231,7005,0,0,0),
	 (7231,7006,0,0,0),
	 (7231,7005,0,0,0),
	 (7231,7007,0,0,0),
	 (7231,7007,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7231,7006,0,0,0),
	 (7231,7006,0,0,0),
	 (7231,7007,0,0,0),
	 (7231,7006,0,0,0),
	 (7231,7005,0,0,0),
	 (7232,7008,0,0,0),
	 (7232,7006,0,0,0),
	 (7232,7007,0,0,0),
	 (7232,7008,0,0,0),
	 (7232,7007,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7232,7006,0,0,0),
	 (7232,7008,0,0,0),
	 (7232,7007,0,0,0),
	 (7232,7006,0,0,0),
	 (7232,7006,0,0,0),
	 (7232,7007,0,0,0),
	 (7232,7008,0,0,0),
	 (7232,7006,0,0,0),
	 (7232,7007,0,0,0),
	 (7232,7008,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7232,7006,0,0,0),
	 (7232,7007,0,0,0),
	 (7232,7008,0,0,0),
	 (7232,7006,0,0,0),
	 (7232,7007,0,0,0),
	 (7232,7008,0,0,0),
	 (7233,7008,0,0,0),
	 (7233,7007,0,0,0),
	 (7233,7008,0,0,0),
	 (7233,7007,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7233,7008,0,0,0),
	 (7233,7007,0,0,0),
	 (7233,7007,0,0,0),
	 (7233,7008,0,0,0),
	 (7233,7007,0,0,0),
	 (7233,7008,0,0,0),
	 (7233,7007,0,0,0),
	 (7233,7008,0,0,0),
	 (7233,7007,0,0,0),
	 (7233,7008,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7234,7009,0,0,0),
	 (7234,7009,0,0,0),
	 (7234,7009,0,0,0),
	 (7234,7009,0,0,0),
	 (7234,7009,0,0,0),
	 (7234,7009,0,0,0),
	 (7234,7009,0,0,0),
	 (7311,7010,0,0,0),
	 (7311,7009,0,0,0),
	 (7311,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7311,7009,0,0,0),
	 (7311,7010,0,0,0),
	 (7311,7010,0,0,0),
	 (7312,7011,0,0,0),
	 (7312,7010,0,0,0),
	 (7312,7009,0,0,0),
	 (7312,7009,0,0,0),
	 (7312,7010,0,0,0),
	 (7312,7011,0,0,0),
	 (7312,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7312,7010,0,0,0),
	 (7312,7009,0,0,0),
	 (7313,7012,0,0,0),
	 (7313,7012,0,0,0),
	 (7313,7011,0,0,0),
	 (7313,7010,0,0,0),
	 (7313,7011,0,0,0),
	 (7313,7011,0,0,0),
	 (7313,7012,0,0,0),
	 (7313,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7313,7010,0,0,0),
	 (7321,7009,0,0,0),
	 (7321,7010,0,0,0),
	 (7321,7009,0,0,0),
	 (7321,7009,0,0,0),
	 (7321,7010,0,0,0),
	 (7321,7009,0,0,0),
	 (7321,7010,0,0,0),
	 (7321,7009,0,0,0),
	 (7321,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7321,7010,0,0,0),
	 (7322,7010,0,0,0),
	 (7322,7009,0,0,0),
	 (7322,7009,0,0,0),
	 (7322,7009,0,0,0),
	 (7322,7011,0,0,0),
	 (7322,7011,0,0,0),
	 (7322,7011,0,0,0),
	 (7322,7010,0,0,0),
	 (7322,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7322,7010,0,0,0),
	 (7322,7011,0,0,0),
	 (7322,7011,0,0,0),
	 (7322,7009,0,0,0),
	 (7322,7010,0,0,0),
	 (7322,7009,0,0,0),
	 (7323,7011,0,0,0),
	 (7323,7010,0,0,0),
	 (7323,7010,0,0,0),
	 (7323,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7323,7012,0,0,0),
	 (7323,7010,0,0,0),
	 (7323,7011,0,0,0),
	 (7323,7012,0,0,0),
	 (7323,7012,0,0,0),
	 (7323,7010,0,0,0),
	 (7323,7011,0,0,0),
	 (7323,7012,0,0,0),
	 (7323,7010,0,0,0),
	 (7323,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7323,7012,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7010,0,0,0),
	 (7330,7010,0,0,0),
	 (7330,7010,0,0,0),
	 (7330,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7330,7009,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7012,0,0,0),
	 (7330,7015,0,0,0),
	 (7330,7015,0,0,0),
	 (7330,7015,0,0,0),
	 (7330,7012,0,0,0),
	 (7330,7012,0,0,0),
	 (7330,7012,0,0,0),
	 (7330,7012,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7330,7012,0,0,0),
	 (7330,7012,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7011,0,0,0),
	 (7330,7013,0,0,0),
	 (7330,7011,0,0,0),
	 (7330,7011,0,0,0),
	 (7330,7011,0,0,0),
	 (7330,7013,0,0,0),
	 (7330,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7330,7013,0,0,0),
	 (7330,7011,0,0,0),
	 (7330,7009,0,0,0),
	 (7330,7014,0,0,0),
	 (7330,7014,0,0,0),
	 (7330,7014,0,0,0),
	 (7330,7013,0,0,0),
	 (7331,7009,0,0,0),
	 (7331,7009,0,0,0),
	 (7331,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7331,7009,0,0,0),
	 (7331,7010,0,0,0),
	 (7331,7010,0,0,0),
	 (7331,7009,0,0,0),
	 (7331,7010,0,0,0),
	 (7331,7009,0,0,0),
	 (7331,7010,0,0,0),
	 (7331,7009,0,0,0),
	 (7331,7010,0,0,0),
	 (7331,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7331,7009,0,0,0),
	 (7332,7010,0,0,0),
	 (7332,7009,0,0,0),
	 (7332,7010,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7009,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7332,7009,0,0,0),
	 (7332,7009,0,0,0),
	 (7332,7010,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7009,0,0,0),
	 (7332,7010,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7011,0,0,0),
	 (7332,7010,0,0,0),
	 (7332,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7332,7009,0,0,0),
	 (7332,7010,0,0,0),
	 (7333,7012,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7010,0,0,0),
	 (7333,7010,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7010,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7012,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7333,7012,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7010,0,0,0),
	 (7333,7010,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7012,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7010,0,0,0),
	 (7333,7012,0,0,0),
	 (7333,7012,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7333,7010,0,0,0),
	 (7333,7011,0,0,0),
	 (7333,7012,0,0,0),
	 (7511,7013,0,0,0),
	 (7511,7013,0,0,0),
	 (7511,7013,0,0,0),
	 (7512,7013,0,0,0),
	 (7512,7014,0,0,0),
	 (7512,7014,0,0,0),
	 (7512,7013,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7512,7014,0,0,0),
	 (7512,7013,0,0,0),
	 (7513,7013,0,0,0),
	 (7513,7014,0,0,0),
	 (7513,7015,0,0,0),
	 (7513,7013,0,0,0),
	 (7513,7014,0,0,0),
	 (7513,7015,0,0,0),
	 (7513,7013,0,0,0),
	 (7513,7015,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7513,7014,0,0,0),
	 (7521,7013,0,0,0),
	 (7521,7013,0,0,0),
	 (7521,7013,0,0,0),
	 (7521,7013,0,0,0),
	 (7521,7013,0,0,0),
	 (7522,7013,0,0,0),
	 (7522,7013,0,0,0),
	 (7522,7014,0,0,0),
	 (7522,7014,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7522,7014,0,0,0),
	 (7522,7013,0,0,0),
	 (7522,7014,0,0,0),
	 (7522,7013,0,0,0),
	 (7522,7014,0,0,0),
	 (7522,7013,0,0,0),
	 (7523,7015,0,0,0),
	 (7523,7013,0,0,0),
	 (7523,7013,0,0,0),
	 (7523,7014,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7523,7015,0,0,0),
	 (7523,7015,0,0,0),
	 (7523,7014,0,0,0),
	 (7523,7013,0,0,0),
	 (7523,7015,0,0,0),
	 (7523,7014,0,0,0),
	 (7523,7014,0,0,0),
	 (7523,7014,0,0,0),
	 (7523,7015,0,0,0),
	 (7523,7013,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7523,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7531,7013,0,0,0),
	 (7532,7013,0,0,0),
	 (7532,7014,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7532,7013,0,0,0),
	 (7532,7014,0,0,0),
	 (7532,7013,0,0,0),
	 (7532,7013,0,0,0),
	 (7532,7013,0,0,0),
	 (7532,7014,0,0,0),
	 (7532,7013,0,0,0),
	 (7532,7014,0,0,0),
	 (7532,7014,0,0,0),
	 (7532,7014,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7532,7014,0,0,0),
	 (7532,7013,0,0,0),
	 (7533,7014,0,0,0),
	 (7533,7013,0,0,0),
	 (7533,7013,0,0,0),
	 (7533,7014,0,0,0),
	 (7533,7015,0,0,0),
	 (7533,7014,0,0,0),
	 (7533,7015,0,0,0),
	 (7533,7015,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7533,7014,0,0,0),
	 (7533,7013,0,0,0),
	 (7533,7013,0,0,0),
	 (7533,7013,0,0,0),
	 (7533,7014,0,0,0),
	 (7533,7015,0,0,0),
	 (7533,7013,0,0,0),
	 (7533,7014,0,0,0),
	 (7533,7015,0,0,0),
	 (7533,7013,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7533,7014,0,0,0),
	 (7533,7015,0,0,0),
	 (7533,7015,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7009,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7015,0,0,0),
	 (7540,7015,0,0,0),
	 (7540,7015,0,0,0),
	 (7540,7012,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7540,7012,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7012,0,0,0),
	 (7540,7015,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7011,0,0,0),
	 (7540,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7540,7011,0,0,0),
	 (7540,7013,0,0,0),
	 (7540,7013,0,0,0),
	 (7540,7013,0,0,0),
	 (7540,7013,0,0,0),
	 (7540,7014,0,0,0),
	 (7540,7014,0,0,0),
	 (7540,7014,0,0,0),
	 (7540,7011,0,0,0),
	 (7540,7011,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7540,7011,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0),
	 (7540,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7540,7011,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7015,0,0,0),
	 (7750,7015,0,0,0),
	 (7750,7015,0,0,0),
	 (7750,7015,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7012,0,0,0),
	 (7750,7015,0,0,0),
	 (7750,7009,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7750,7014,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7013,0,0,0),
	 (7750,7013,0,0,0),
	 (7750,7013,0,0,0),
	 (7750,7013,0,0,0),
	 (7750,7013,0,0,0),
	 (7750,7013,0,0,0),
	 (7750,7013,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7750,7014,0,0,0),
	 (7750,7014,0,0,0),
	 (7750,7014,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7009,0,0,0),
	 (7750,7010,0,0,0),
	 (7750,7010,0,0,0),
	 (7750,7010,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7750,7010,0,0,0),
	 (7750,7010,0,0,0),
	 (7750,7010,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7011,0,0,0),
	 (7750,7011,0,0,0),
	 (7066,7060,0,0,0),
	 (7067,7060,0,0,0);
INSERT INTO evemu.dunRoomData (dunRoomID,dunGroupID,xpos,ypos,zpos) VALUES
	 (7068,7060,0,0,0),
	 (7069,7060,0,0,0),
	 (7076,7071,0,0,0),
	 (7077,7071,0,0,0),
	 (7078,7071,0,0,0),
	 (7079,7071,0,0,0),
	 (7086,7081,0,0,0),
	 (7087,7081,0,0,0),
	 (7088,7081,0,0,0),
	 (7089,7081,0,0,0);