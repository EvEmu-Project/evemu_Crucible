/* ship warp speed update sql file
 *   -allan 19may16
 *   this file contains the correct warp attributes for (i think) all ships ingame
 *
 *    for the crucible expansion, warp speed multipliers (attributeID 600)
 *  are as follows and based on ship group.
 *      Freighter       =  0.25
 *      Capital         =  0.5
 *      I/C/BC/BS/D/MB  =  1.0
 *      T2 Cruiser      =  1.25
 *      Frigate         =  2.0
 *      Inty #1         =  3.0
 *      Inty/CovOps     =  4.5
 */

/* fix base warp speeds */
UPDATE dgmTypeAttributes SET valueFloat = 3 WHERE attributeID = 1281;

-- warp speed update for groupID 28,380,902 (Freighter)
UPDATE dgmTypeAttributes
SET valueFloat = 0.25
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID IN (28,380,902));

-- warp speed update for groupID 30,485,547,659,883,941 (Capital)
UPDATE dgmTypeAttributes
SET valueFloat = 0.5
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID IN (30,485,547,659,883,941));

-- warp speed update for groupID 26,27,237,381,419,420,463,540,543,963 (C/BC/BS/D/MB)
UPDATE dgmTypeAttributes
SET valueFloat = 1.0
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID IN (26,27,237,381,419,420,463,540,543,963));

-- warp speed update for groupID 29,358 (T2 Cruiser)
UPDATE dgmTypeAttributes
SET valueFloat = 1.25
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID IN (29,358));

-- warp speed update for groupID 25 (frigate)
UPDATE dgmTypeAttributes
SET valueFloat = 2.0
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID = 25);

-- warp speed update for groupID 31,541,832,894 (Inty #1)
UPDATE dgmTypeAttributes
SET valueFloat = 3.0
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID IN (31,541,832,894));

-- warp speed update for groupID 324,830,831,900 (Inty #2/CovOps)
UPDATE dgmTypeAttributes
SET valueFloat = 4.5
WHERE attributeID = 600
AND typeID IN (SELECT typeID FROM invTypes WHERE groupID IN (324,830,831,900));

-- warp speed correction for Sigil
UPDATE dgmTypeAttributes
SET valueFloat = 1.5
WHERE attributeID = 600
AND typeID = 19744;

-- warp speed correction for Orca
UPDATE dgmTypeAttributes
SET valueFloat = 0.75
WHERE attributeID = 600
AND typeID = 28606;
