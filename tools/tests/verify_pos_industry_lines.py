"""Exercise the migration and the server's exact provisioning statement in MariaDB."""
import ast
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
MARIADB = ["mariadb", "--batch", "--skip-column-names", *sys.argv[1:]]


def run(sql):
    return subprocess.run(MARIADB, input=sql, text=True, check=True,
                          stdout=subprocess.PIPE).stdout.strip()


def provision_sql():
    source = (ROOT / "src/eve-server/manufacturing/FactoryDB.cpp").read_text()
    helper = source.split("bool EnsurePosAssemblyLines(", 1)[1].split(
        "PyRep *FactoryDB::AssemblyLinesSelectCorporation", 1)[0]
    literal = r'"(?:[^"\\]|\\.)*"'
    digits_block = helper.split("constexpr const char* digits =", 1)[1].split(";", 1)[0]
    query_block = helper.split("sDatabase.RunQuery(err,", 1)[1].split(
        "EvERam::RestrictionMask::ByCorp", 1)[0]
    digits = "".join(map(ast.literal_eval, re.findall(literal, digits_block)))
    query = "".join(map(ast.literal_eval, re.findall(literal, query_block)))
    return query % (4, digits, digits, 98000000, 0, 0)


run("""
CREATE TABLE ramAssemblyLines (
    assemblyLineID INT NOT NULL PRIMARY KEY, containerID INT, assemblyLineTypeID TINYINT UNSIGNED,
    ownerID INT, activityID TINYINT UNSIGNED, nextFreeTime BIGINT NOT NULL DEFAULT 0,
    UIGroupingID TINYINT UNSIGNED, costInstall DOUBLE, costPerHour DOUBLE, restrictionMask TINYINT,
    discountPerGoodStandingPoint DOUBLE, surchargePerBadStandingPoint DOUBLE,
    minimumStanding DOUBLE, minimumCharSecurity DOUBLE, maximumCharSecurity DOUBLE,
    minimumCorpSecurity DOUBLE, maximumCorpSecurity DOUBLE
) ENGINE=MyISAM;
CREATE TABLE posStructureData (itemID INT PRIMARY KEY, towerID INT, state INT);
CREATE TABLE posTowerData (itemID INT PRIMARY KEY, harmonic INT);
CREATE TABLE entity (itemID INT PRIMARY KEY, typeID INT, locationID INT, ownerID INT);
CREATE TABLE invTypes (typeID INT PRIMARY KEY, groupID INT);
CREATE TABLE ramInstallationTypeContents (
    installationTypeID INT, assemblyLineTypeID TINYINT UNSIGNED, quantity TINYINT,
    PRIMARY KEY (installationTypeID, assemblyLineTypeID)
);
CREATE TABLE ramAssemblyLineTypes (assemblyLineTypeID TINYINT UNSIGNED PRIMARY KEY, activityID INT);
INSERT INTO ramAssemblyLines (assemblyLineID, containerID, assemblyLineTypeID, nextFreeTime)
VALUES (99, 60000976, 35, 12345);
INSERT INTO invTypes VALUES (16213,365), (28351,413), (24574,397);
INSERT INTO entity VALUES (140003744,16213,30004759,98000000),
    (140004212,28351,30004759,98000000), (140004213,24574,30004759,98000000),
    (140004214,28351,30004759,98000000), (140004215,24574,30004759,98000001);
INSERT INTO posTowerData VALUES (140003744,1);
INSERT INTO posStructureData VALUES (140003744,140003744,4),
    (140004212,140003744,4), (140004213,140003744,4),
    (140004214,140003744,2), (140004215,140003744,4);
INSERT INTO ramAssemblyLineTypes VALUES (146,5), (147,4), (149,8), (17,1);
INSERT INTO ramInstallationTypeContents VALUES (28351,146,3), (28351,147,2), (28351,149,2), (24574,17,10);
""")
migration = (ROOT / "sql/migrations/20261118120000-pos-industry-lines.sql").read_text()
run(migration.split("-- +migrate Down", 1)[0])
provision = provision_sql()
run(provision)
assert run("SELECT containerID,assemblyLineTypeID,COUNT(*) FROM ramAssemblyLines "
           "WHERE posSlot IS NOT NULL GROUP BY containerID,assemblyLineTypeID ORDER BY 1,2") == (
               "140004212\t146\t3\n140004212\t147\t2\n140004213\t17\t10")
assert run("SELECT assemblyLineID,nextFreeTime FROM ramAssemblyLines WHERE posSlot IS NULL") == "99\t12345"
before = run("SELECT assemblyLineID FROM ramAssemblyLines ORDER BY assemblyLineID")
run(provision)
assert run("SELECT assemblyLineID FROM ramAssemblyLines ORDER BY assemblyLineID") == before
run("UPDATE posStructureData SET state=1 WHERE itemID=140003744")
assert run("SELECT COUNT(*) FROM ramOnlinePosLineTypes") == "0"
assert run("SELECT COUNT(*) FROM ramAssemblyLines WHERE posSlot IS NOT NULL") == "15"
print("PASS: POS slot migration, counts, ownership, online filtering and repeat provisioning")
