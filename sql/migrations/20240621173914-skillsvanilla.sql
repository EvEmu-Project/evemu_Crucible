--  Fixed starting skills, job skills did not exist in Crucible

-- +migrate Up

-- UPDATE BASE SKILLS
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3300; -- Gunnery
UPDATE sklBaseSkills
    SET level = 3 WHERE skillTypeID = 3327; -- Spaceship Command
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3386; -- Mining
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3393; -- Mechanics
UPDATE sklBaseSkills
    SET level = 3 WHERE skillTypeID = 3402; -- Science
UPDATE sklBaseSkills
    SET level = 3 WHERE skillTypeID = 3413; -- Engineering
UPDATE sklBaseSkills
    SET level = 3 WHERE skillTypeID = 3416; -- Shield Operation
UPDATE sklBaseSkills
    SET level = 3 WHERE skillTypeID = 3426; -- Electronics
UPDATE sklBaseSkills
    SET level = 3 WHERE skillTypeID = 3449; -- Navigation

-- UPDATE RACE SKILLS
DROP TABLE sklRaceSkills;
CREATE TABLE sklRaceSkills(
    id INT(10) DEFAULT NULL,
    raceID INT(10) DEFAULT NULL,
    skillTypeID INT(10) DEFAULT NULL,
    level TINYINT(3) DEFAULT NULL
);

INSERT INTO sklRaceSkills(id, raceID, skillTypeID, level) VALUES
    (1,1,3301,3),
    (2,1,3330,2),
    (3,2,3302,3),
    (4,2,3329,2),
    (5,4,3303,3),
    (6,4,3331,2),
    (7,8,3301,3),
    (8,8,3328,2);

-- +migrate Down

-- RESTORE BASE SKILLS
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3300; -- Gunnery
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3327; -- Spaceship Command
UPDATE sklBaseSkills
    SET level = 1 WHERE skillTypeID = 3386; -- Mining
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3393; -- Mechanics
UPDATE sklBaseSkills
    SET level = 1 WHERE skillTypeID = 3402; -- Science
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3413; -- Engineering
UPDATE sklBaseSkills
    SET level = 1 WHERE skillTypeID = 3416; -- Shield Operation
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3426; -- Electronics
UPDATE sklBaseSkills
    SET level = 2 WHERE skillTypeID = 3449; -- Navigation

-- RESTORE RACE SKILLS
DROP TABLE sklRaceSkills;
CREATE TABLE sklRaceSkills(
    id INT(10) DEFAULT NULL,
    raceID INT(10) DEFAULT NULL,
    skillTypeID INT(10) DEFAULT NULL,
    level TINYINT(3) DEFAULT NULL
);

INSERT INTO sklRaceSkills(id, raceID, skillTypeID, level) VALUES
    (1,1,3301,2),
    (2,1,3330,2),
    (3,1,3319,2),
    (4,1,3321,3),
    (5,1,3413,2),
    (6,1,3432,1),
    (7,1,3416,1),
    (8,1,3426,2),
    (9,1,21059,2),
    (10,1,3425,1),
    (11,2,3302,2),
    (12,2,3329,2),
    (13,2,3416,2),
    (14,2,3413,2),
    (15,2,3426,2),
    (16,2,3300,2),
    (17,2,3393,3),
    (18,2,3394,4),
    (19,4,3303,2),
    (20,4,3331,2),
    (21,4,3392,2),
    (22,4,3393,3),
    (23,4,3394,3),
    (24,4,3417,2),
    (25,4,3418,2),
    (26,8,3301,2),
    (27,8,3328,2),
    (28,8,3436,4),
    (29,8,3437,3),
    (30,8,3442,2),
    (31,8,12305,2),
    (32,8,3392,2);


