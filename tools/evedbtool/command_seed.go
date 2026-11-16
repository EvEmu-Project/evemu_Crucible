package main

import (
	"database/sql"
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"

	migrate "github.com/rubenv/sql-migrate"
	"github.com/spf13/viper"
	"gopkg.in/jcmturner/rpc.v1/mstypes"
)

type SeedCommand struct{}

func (c *SeedCommand) Help() string {
	return strings.TrimSpace(`
Usage: evedbtool seed [options] ...
  Seeds the configured EVEmu regions with NPC market data.
Options:
  -dryrun                Print SQL without applying it.
  -force                 Rebuild seeded NPC sell orders in each configured region.
                         This replaces existing NPC corporation sell orders for
                         the seeded market item categories in those regions.
`)
}

func (c *SeedCommand) Synopsis() string {
	return "Seeds or rebuilds the configured EVEmu regional markets."
}

func (c *SeedCommand) Run(args []string) int {
	var dryrun bool
	var force bool

	cmdFlags := flag.NewFlagSet("seed", flag.ContinueOnError)
	cmdFlags.Usage = func() { ui.Output(c.Help()) }
	cmdFlags.BoolVar(&dryrun, "dryrun", false, "Print SQL without applying it.")
	cmdFlags.BoolVar(&force, "force", false, "Replace seeded NPC market orders before reseeding.")

	if err := cmdFlags.Parse(args); err != nil {
		return 1
	}

	regionArray := viper.GetStringSlice("seed-regions")
	if len(regionArray) == 0 {
		log.Error("No seed-regions configured in evedb.yaml.")
		return 1
	}
	saturation := viper.GetInt("seed-saturation")
	if saturation < 1 || saturation > 100 {
		log.Error("seed-saturation must be between 1 and 100.")
		return 1
	}

	seedQueries := make([]string, 0, len(regionArray))
	for _, regionID := range regionArray {
		log.Info("Preparing market seed for region ", regionID)
		seedQueries = append(seedQueries, getQuery(regionID, saturation, force))
	}

	if dryrun {
		log.Info("Dry run; no database changes will be made.")
		for i, query := range seedQueries {
			log.Info("SQL for region ", regionArray[i], ":\n---\n", query)
		}
		return 0
	}

	tables := GetNumberOfTables()
	if tables == 0 {
		log.Error("Database is not initialized; install the DB before seeding.")
		return 1
	}

	migrationID := "SEED_MARKET"
	if force {
		// A new migration ID makes an explicit reseed repeatable without
		// changing or deleting the migration history from earlier runs.
		migrationID += "_" + strconv.FormatInt(time.Now().UTC().UnixNano(), 10)
	}

	db := getDB()
	defer db.Close()
	if err := db.Ping(); err != nil {
		log.Error("Unable to connect to the EVEmu database: ", err)
		return 1
	}

	migrate.SetTable("seed_migrations")
	// This dedicated seed history may contain IDs from older EVEDBTool builds.
	// Each force run uses a unique ID and only moves forward; do not block it
	// because a legacy seed ID is absent from this in-memory migration source.
	migrate.SetIgnoreUnknown(true)
	migrations, err := buildSeedMigrations(db, migrationID, seedQueries)
	if err != nil {
		log.Error("Unable to prepare market seed migration history: ", err)
		return 1
	}
	source := &migrate.MemoryMigrationSource{Migrations: migrations}
	log.Info("Executing market seed migration ", migrationID, "...")
	n, err := migrate.Exec(db, "mysql", source, migrate.Up)
	if err != nil {
		log.Error("Market seed failed: ", err)
		return 1
	}
	if n == 0 {
		log.Info("Market seed was already applied. Use 'evedbtool seed -force' to rebuild the configured regions.")
	} else {
		log.Info("Successfully applied market seed to ", len(regionArray), " configured region(s).")
	}

	// Force runs use a unique timestamped migration ID. Retain only the latest
	// successful force marker so seed_migrations does not grow indefinitely.
	// This also removes stale IDs left by earlier EVEDBTool versions.
	if force {
		result, cleanupErr := db.Exec("DELETE FROM seed_migrations WHERE LEFT(id, 12) = 'SEED_MARKET_' AND id <> ?", migrationID)
		if cleanupErr != nil {
			log.Warn("Market seed succeeded, but old force migration markers could not be cleaned up: ", cleanupErr)
		} else if removed, rowsErr := result.RowsAffected(); rowsErr == nil && removed > 0 {
			log.Info("Removed ", removed, " obsolete market seed migration marker(s).")
		}
	}
	return 0
}

func buildSeedMigration(id string, seedQueries []string) *migrate.Migration {
	return &migrate.Migration{
		Id:   id,
		Up:   seedQueries,
		Down: []string{},
	}
}

// buildSeedMigrations includes no-op representations of previously applied
// seed migrations. sql-migrate rejects a database containing migration IDs
// that are absent from its source; force runs use unique IDs, so without these
// placeholders the next run would fail on the previous SEED_MARKET_* entry.
func buildSeedMigrations(db *sql.DB, currentID string, seedQueries []string) ([]*migrate.Migration, error) {
	// Ensure the custom migration table exists before reading its history.
	// This matches sql-migrate's MySQL table layout.
	_, err := db.Exec("CREATE TABLE IF NOT EXISTS seed_migrations (id VARCHAR(255) NOT NULL PRIMARY KEY, applied_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP)")
	if err != nil {
		return nil, fmt.Errorf("create seed_migrations table: %w", err)
	}

	rows, err := db.Query("SELECT id FROM seed_migrations")
	if err != nil {
		return nil, fmt.Errorf("read seed_migrations history: %w", err)
	}
	defer rows.Close()

	migrations := make([]*migrate.Migration, 0)
	for rows.Next() {
		var id string
		if err := rows.Scan(&id); err != nil {
			return nil, fmt.Errorf("read seed migration ID: %w", err)
		}
		if id == currentID {
			continue
		}
		migrations = append(migrations, &migrate.Migration{
			Id:   id,
			Up:   []string{},
			Down: []string{},
		})
	}
	if err := rows.Err(); err != nil {
		return nil, fmt.Errorf("iterate seed_migrations history: %w", err)
	}

	// Keep the real SQL only on the migration being applied in this run.
	migrations = append(migrations, buildSeedMigration(currentID, seedQueries))
	return migrations, nil
}

func getQuery(regionValue string, saturation int, force bool) string {
	regionMap := map[string]int{
		"Derelik": 10000001, "The Forge": 10000002, "Vale of the Silent": 10000003,
		"UUA-F4": 10000004, "Detorid": 10000005, "Wicked Creek": 10000006,
		"Cache": 10000007, "Scalding Pass": 10000008, "Insmother": 10000009,
		"Tribute": 10000010, "Great Wildlands": 10000011, "Curse": 10000012,
		"Malpais": 10000013, "Catch": 10000014, "Venal": 10000015,
		"Lonetrek": 10000016, "J7HZ-F": 10000017, "The Spire": 10000018,
		"A821-A": 10000019, "Tash-Murkon": 10000020, "Outer Passage": 10000021,
		"Stain": 10000022, "Pure Blind": 10000023, "Immensea": 10000025,
		"Etherium Reach": 10000027, "Molden Heath": 10000028, "Geminate": 10000029,
		"Heimatar": 10000030, "Impass": 10000031, "Sinq Laison": 10000032,
		"The Citadel": 10000033, "The Kalevala Expanse": 10000034, "Deklein": 10000035,
		"Devoid": 10000036, "Everyshore": 10000037, "The Bleak Lands": 10000038,
		"Esoteria": 10000039, "Oasa": 10000040, "Syndicate": 10000041,
		"Metropolis": 10000042, "Domain": 10000043, "Solitude": 10000044,
		"Tenal": 10000045, "Fade": 10000046, "Providence": 10000047,
		"Placid": 10000048, "Khanid": 10000049, "Querious": 10000050,
		"Cloud Ring": 10000051, "Kador": 10000052, "Cobalt Edge": 10000053,
		"Aridia": 10000054, "Branch": 10000055, "Feythabolis": 10000056,
		"Outer Ring": 10000057, "Fountain": 10000058, "Paragon Soul": 10000059,
		"Delve": 10000060, "Tenerifis": 10000061, "Omist": 10000062,
		"Period Basis": 10000063, "Essence": 10000064, "Kor-Azor": 10000065,
		"Perrigen Falls": 10000066, "Genesis": 10000067, "Verge Vendor": 10000068,
		"Black Rise": 10000069, "A-R00001": 11000001, "A-R00002": 11000002,
		"A-R00003": 11000003, "B-R00004": 11000004, "B-R00005": 11000005,
		"B-R00006": 11000006, "B-R00007": 11000007, "B-R00008": 11000008,
		"C-R00009": 11000009, "C-R00010": 11000010, "C-R00011": 11000011,
		"C-R00012": 11000012, "C-R00013": 11000013, "C-R00014": 11000014,
		"C-R00015": 11000015, "D-R00016": 11000016, "D-R00017": 11000017,
		"D-R00018": 11000018, "D-R00019": 11000019, "D-R00020": 11000020,
		"D-R00021": 11000021, "D-R00022": 11000022, "D-R00023": 11000023,
		"E-R00024": 11000024, "E-R00025": 11000025, "E-R00026": 11000026,
		"E-R00027": 11000027, "E-R00028": 11000028, "E-R00029": 11000029,
		"F-R00030": 11000030,
	}

	regionID := regionValue
	if _, err := strconv.Atoi(regionValue); err != nil {
		value, ok := regionMap[regionValue]
		if !ok {
			log.Error("Invalid seed region name: ", regionValue)
			os.Exit(1)
		}
		regionID = strconv.Itoa(value)
	}

	timestamp := mstypes.GetFileTime(time.Now()).MSEpoch()
	var cleanup string
	if force {
		cleanup = `
-- Remove prior NPC corporation sell orders for seeded market categories.
-- Player-owned orders are retained because ownerID must match the station corporation.
DELETE oldOrder
FROM mktOrders AS oldOrder
INNER JOIN staStations AS station
    ON station.stationID = oldOrder.stationID
   AND station.regionID = oldOrder.regionID
   AND station.corporationID = oldOrder.ownerID
WHERE oldOrder.regionID = @regionid
  AND oldOrder.bid = 0
  AND (
      oldOrder.typeID IN (
          SELECT seededType.typeID
          FROM invTypes AS seededType
          INNER JOIN invGroups AS seededGroup ON seededType.groupID = seededGroup.groupID
          WHERE (
              seededGroup.categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46)
              -- Buyable containers are Celestial-category items. Restrict
              -- to the four historical container inventory groups and only
              -- published, market-listed types; do not seed wreck/loot cans.
              OR (seededGroup.groupID IN (12, 340, 448, 649)
                  AND seededType.published = 1
                  AND seededType.marketGroupID IS NOT NULL)
          )
      )
      OR oldOrder.typeID IN (
          SELECT rareType.typeID
          FROM invTypes AS rareType
          INNER JOIN invGroups AS rareGroup ON rareType.groupID = rareGroup.groupID
          INNER JOIN invMetaTypes AS metaType ON rareType.typeID = metaType.typeID
          INNER JOIN invMetaGroups AS metaGroup ON metaType.metaGroupID = metaGroup.metaGroupID
          WHERE LOWER(COALESCE(metaGroup.metaGroupName, '')) IN ('officer', 'deadspace', 'faction', 'storyline')
      )
  );
`
	}

	query := `
SET @regionid = RVAL;
SET @saturation = SVAL;

CREATE TEMPORARY TABLE IF NOT EXISTS tStations (
    stationId INT NOT NULL PRIMARY KEY,
    solarSystemID INT,
    regionID INT,
    corporationID INT,
    security FLOAT
) ENGINE=InnoDB;
TRUNCATE TABLE tStations;

-- Select each station independently so the configured saturation works
-- without relying on server-specific LIMIT user-variable behaviour.
INSERT INTO tStations (stationId, solarSystemID, regionID, corporationID, security)
SELECT stationID, solarSystemID, regionID, corporationID, security
FROM staStations
WHERE regionID = @regionid
  AND RAND() < @saturation;

CLEANUP

-- Materialize existing sell orders for the selected stations once. The primary
-- key makes the repeated station/type existence checks cheap, instead of
-- running a correlated lookup against the full mktOrders table per candidate.
CREATE TEMPORARY TABLE IF NOT EXISTS tExistingSellOrders (
    stationID INT NOT NULL,
    typeID INT NOT NULL,
    PRIMARY KEY (stationID, typeID)
) ENGINE=InnoDB;
TRUNCATE TABLE tExistingSellOrders;

INSERT IGNORE INTO tExistingSellOrders (stationID, typeID)
SELECT existing.stationID, existing.typeID
FROM tStations AS stations
INNER JOIN mktOrders AS existing
    ON existing.stationID = stations.stationId
WHERE existing.bid = 0;

-- Standard published market items, including 550,000 units of charges/ammunition.
INSERT INTO mktOrders
    (typeID, ownerID, regionID, stationID, price, volEntered, volRemaining,
     issued, minVolume, duration, solarSystemID, jumps)
SELECT
    it.typeID,
    stations.corporationID,
    stations.regionID,
    stations.stationId,
    -- NPC market prices are exactly 100% of invTypes.basePrice.
    -- No security-based multiplier: 0.5 security used to double prices.
    -- Types with no positive basePrice use the existing 100 ISK fallback.
    CASE
        WHEN it.basePrice IS NULL OR it.basePrice <= 0 THEN 100
        ELSE GREATEST(0.01, ROUND(it.basePrice, 2))
    END,
    CASE WHEN ig.categoryID = 8 THEN 550000 ELSE 550 END,
    CASE WHEN ig.categoryID = 8 THEN 550000 ELSE 550 END,
    TSTAMP,
    1,
    250,
    stations.solarSystemID,
    1
FROM tStations AS stations
CROSS JOIN invTypes AS it
INNER JOIN invGroups AS ig ON it.groupID = ig.groupID
LEFT JOIN tExistingSellOrders AS existing
    ON existing.stationID = stations.stationId
   AND existing.typeID = it.typeID
WHERE it.published = 1
  AND (
      ig.categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 22, 23, 24, 25, 32, 34, 35, 39, 40, 41, 42, 43, 46)
      -- Crucible market containers are Celestial-category groups:
      -- ordinary (12), secure (340), audit log (448), freight (649).
      OR (ig.groupID IN (12, 340, 448, 649)
          AND it.marketGroupID IS NOT NULL)
  )
  AND existing.stationID IS NULL;

-- Add newly seeded orders to the lookup table before considering rare modules.
INSERT IGNORE INTO tExistingSellOrders (stationID, typeID)
SELECT existing.stationID, existing.typeID
FROM tStations AS stations
INNER JOIN mktOrders AS existing
    ON existing.stationID = stations.stationId
WHERE existing.bid = 0;

-- Officer, Deadspace, Faction and Storyline modules.
INSERT INTO mktOrders
    (typeID, ownerID, regionID, stationID, price, volEntered, volRemaining,
     issued, minVolume, duration, solarSystemID, jumps)
SELECT
    it.typeID,
    stations.corporationID,
    stations.regionID,
    stations.stationId,
    -- NPC market prices are exactly 100% of invTypes.basePrice.
    -- No security-based multiplier: 0.5 security used to double prices.
    -- Types with no positive basePrice use the existing 100 ISK fallback.
    CASE
        WHEN it.basePrice IS NULL OR it.basePrice <= 0 THEN 100
        ELSE GREATEST(0.01, ROUND(it.basePrice, 2))
    END,
    10,
    10,
    TSTAMP,
    1,
    250,
    stations.solarSystemID,
    1
FROM tStations AS stations
CROSS JOIN invTypes AS it
INNER JOIN invGroups AS ig ON it.groupID = ig.groupID
INNER JOIN invMetaTypes AS imt ON it.typeID = imt.typeID
INNER JOIN invMetaGroups AS mg ON imt.metaGroupID = mg.metaGroupID
LEFT JOIN tExistingSellOrders AS existing
    ON existing.stationID = stations.stationId
   AND existing.typeID = it.typeID
WHERE LOWER(COALESCE(mg.metaGroupName, '')) IN ('officer', 'deadspace', 'faction', 'storyline')
  AND existing.stationID IS NULL;
`

	replacer := strings.NewReplacer(
		"SVAL", fmt.Sprintf("%.2f", float32(saturation)/100),
		"RVAL", regionID,
		"TSTAMP", strconv.FormatInt(timestamp, 10),
		"CLEANUP", cleanup,
	)
	return replacer.Replace(query)
}
