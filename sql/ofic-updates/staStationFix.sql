/* 
 * Some databases seem to have a problem with the default conversation between null and 0. This fixes the issue for the problematic entries.
 */

UPDATE staStations SET corporationID = 0 WHERE stationID IN (60014839, 60014842, 60014845, 60014848);