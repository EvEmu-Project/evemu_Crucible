-- Example Dungeon (copy this and modify to add a new dungeon to the game)
-- 
-- Process:
-- Step 1. Add a new dungeon definition to the database
-- Step 2. Edit the dungeon using the dungeonEditor
-- Step 3. Add the table changes to this migration
-- 
-- +migrate Up

-- Create the dungeon itself
INSERT INTO dunDungeons
VALUES (
    1200000000,        -- Dungeon ID (MUST BE UNIQUE and start from 1.2b. Check dunDungeons first before creating.)
    "Example Dungeon", -- Dungeon Name (used for selecting within the dungeon editor)
    3,                 -- Dungeon Status (1=Release, 2=Testing, 3=Working Copy)
    500001,            -- FactionID of the dungeon (this can be obtained from the facFactions)
    7                  -- Type of the dungeon (for example, Mission, Anomaly, Wormhole, etc.
                       -- this can be obtained from the dunArchetypes table
);

-- Create dungeon rooms (create as many room entries as are needed for your dungeon)
INSERT INTO dunRooms
VALUES (
    10000,              -- ID of the room (MUST BE UNIQUE. Check dunRooms AND dunRoomData for collisions)
    "First Room",       -- Name of the room
    1200000000          -- ID of the dungeon
);
INSERT INTO dunRooms
VALUES (
    10001,              -- ID of the room (MUST BE UNIQUE. Check dunRooms AND dunRoomData for collisions)
    "Second Room",      -- Name of the room
    1200000000          -- ID of the dungeon
);

-- Create example room object (this is not needed if you are using the room editor)
INSERT INTO dunRoomObjects
VALUES (
    NULL,               -- ID of the room object (MUST BE UNIQUE. Check dunRoomObjects AND dunRoomObjectData for collisions)
    10001,              -- ID of the room
    9886,               -- TypeID of the item to be inserted
    10000,              -- GroupID of the item
    15000,              -- x relative position
    3000,               -- y relative position
    1000,               -- z relative position
    0,                  -- yaw value
    0,                  -- pitch value
    0,                  -- roll value
    10000               -- radius in meters
),
(
    NULL,               -- ID of the room object (MUST BE UNIQUE. Check dunRoomObjects AND dunRoomObjectData for collisions)
    10000,              -- ID of the room
    17831,              -- TypeID of the item to be inserted
    10000,              -- GroupID of the item
    100,                -- x relative position
    100,                -- y relative position
    100,                -- z relative position
    0,                  -- yaw value
    0,                  -- pitch value
    0,                  -- roll value
    10000               -- radius in meters
);

-- Create a group for the object to be in
INSERT INTO dunGroups
VALUES (
    10000,
    "Example Group",
    0,
    0,
    0
);

-- +migrate Down
DELETE FROM dunDungeons
    WHERE dungeonID=1200000000; -- Use the same Dungeon ID which was created in the 'migrate Up' section
DELETE FROM dunRooms
    WHERE dungeonID=1200000000;
DELETE FROM dunRoomObjects
    WHERE roomID=10001;
DELETE FROM dunGroups
    WHERE groupID=10000;