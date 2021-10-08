-- +migrate Up
ALTER TABLE ramJobs
ADD COLUMN eventID INT(10) NOT NULL DEFAULT 0 AFTER licensedProductionRuns;
ALTER TABLE sysCalendarEvents
ADD COLUMN autoEventType SMALLINT(3) NULL AFTER eventText;

-- +migrate Down
ALTER TABLE ramJobs
DROP COLUMN eventID;
ALTER TABLE sysCalendarEvents
DROP COLUMN autoEventType;
