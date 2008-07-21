# useless tables, we can pull their content from elsewhere
DROP TABLE IF EXISTS ramALTypes;
DROP TABLE IF EXISTS ramALTypeGroup;
DROP TABLE IF EXISTS ramALTypeCategory;

# change datetime to bigint(20) and reset it ... I see no reason why store datetime instead of timestamp
ALTER IGNORE TABLE ramAssemblyLines MODIFY nextFreeTime bigint(20);
UPDATE ramAssemblyLines SET nextFreeTime = 0;

# fix wrong default values in dgmAttributes
UPDATE dgmAttributes SET defaultValue = 100 WHERE attributeID IN (365, 366);

# create table for jobs
DROP TABLE IF EXISTS ramJobs;
CREATE TABLE ramJobs ( 
		jobID int(10) unsigned NOT NULL AUTO_INCREMENT,
		ownerID int(10) unsigned NOT NULL,
		installerID int(10) unsigned NOT NULL,
		assemblyLineID int(10) unsigned NOT NULL,
		installedItemID int(10) unsigned NOT NULL,
		installTime bigint(20) unsigned NOT NULL,
		beginProductionTime bigint(20) unsigned NOT NULL,
		endProductionTime bigint(20) unsigned NOT NULL,
		description varchar(250) NOT NULL default 'blah',
		runs int(10) unsigned NOT NULL,
		outputFlag int(10) unsigned NOT NULL,
		completedStatusID int(10) unsigned NOT NULL,
		installedInSolarSystemID int(10) unsigned NOT NULL,
		licensedProductionRuns int(10) default NULL,
		PRIMARY KEY  (jobID),
		KEY RAMJOBS_ASSEMBLYLINES (assemblyLineID)  
);

# create table for blueprints
DROP TABLE IF EXISTS invBlueprints;
CREATE TABLE invBlueprints (
		blueprintID int(10) unsigned NOT NULL,
		copy tinyint(1) unsigned NOT NULL DEFAULT 0,
		materialLevel int(10) unsigned NOT NULL DEFAULT 0,
		productivityLevel int(10) unsigned NOT NULL DEFAULT 0,
		licensedProductionRunsRemaining int(10) NOT NULL DEFAULT -1,
		PRIMARY KEY (blueprintID)
);

# delete details for groups which are unknown
DELETE FROM ramAssemblyLineTypeDetailPerGroup
WHERE groupID NOT IN (SELECT groupID FROM invGroups);

