ALTER TABLE ramJobs
 ADD COLUMN pauseProductionTime bigint(20) unsigned default NULL AFTER beginProductionTime;
