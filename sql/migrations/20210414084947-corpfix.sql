

-- +migrate Up
ALTER TABLE crpCorporation
  MODIFY `allianceMemberStartDate` bigint(20) NOT NULL DEFAULT '0';

-- +migrate Down
ALTER TABLE crpCorporation
  MODIFY `allianceMemberStartDate` bigint(20) NOT NULL;
