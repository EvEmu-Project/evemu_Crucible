#!/bin/bash

set -Eeuo pipefail

: "${MYSQL_USER:?MYSQL_USER must be set}"
: "${MYSQL_PASSWORD:?MYSQL_PASSWORD must be set}"
: "${DB_NAME:?DB_NAME must be set}"
MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_PORT="${MYSQL_PORT:-3306}"

[[ "$MYSQL_HOST" =~ ^[A-Za-z0-9_.:-]+$ ]] || exit 2
[[ "$MYSQL_PORT" =~ ^[0-9]+$ ]] || exit 2
(( MYSQL_PORT > 0 && MYSQL_PORT <= 65535 )) || exit 2

unset blacklist

_() { blacklist="${blacklist}${blacklist+}${*}"; }

# Add any tables that should be dumped with no data.
_ "agtOffers alnAlliance alnContacts alnLabels avatars avatar_colors"
_ "avatar_modifiers avatar_sculpts billsPayable billsReceivable bookmarks"
_ "bookmarkFolders cacheOwners channelChars channelMods channels"
_ "chrCertificates chrCharacters chrCharacterAttributes chrContacts chrEmployment"
_ "chrKillTable chrLabels chrNotes chrOwnerNote chrPausedSkillQueue"
_ "chrShipFittings chrSkillHistory chrSkillQueue chrVisitedSystems crpAdRegistry"
_ "crpApplications crpBulletins crpContacts crpLabels crpLockedItems"
_ "crpItemEvent crpRecruiters crpRoleHistroy crpRoleTitles crpShares"
_ "crpVoteItems crpWalletDivisons droneState dunActive entity"
_ "entity_attributes eveMail eveMailDetails invBlueprints jnlCharacters"
_ "jnlCorporations mailLabel mailList mailListUsers mailMessage"
_ "mailStatus mapDynamicData mktHistory mktOrders mktTransactions"
_ "piCCPin piECUHeads piLaunches piLinks piPinContents"
_ "piPins piPlanets piRoutes posCustomOfficeData posJumpBridgeData"
_ "posStructureData posTowerData ramJobs rentalInfo repStandingChanges"
_ "shipInsurance srvStatisticData srvStatisticHistory staOffices sysAsteroids"
_ "sysSignatures webBounties"

SQL_STRING="SHOW TABLES;"
# Pipe the SQL into mysql
TABLES=$(MYSQL_PWD="$MYSQL_PASSWORD" mysql \
    --skip-auto-rehash \
    --host="$MYSQL_HOST" \
    --port="$MYSQL_PORT" \
    --user="$MYSQL_USER" \
    --database="$DB_NAME" \
    --batch --skip-column-names --execute="$SQL_STRING")

mkdir -p dump

# Check if mysql client is available or not
if ! command -v mysqldump &> /dev/null
then
    echo "Client not found, please install the latest MariaDB client"
    exit
fi

#Updated to use gzipped .sql files to reduce size of repository and not require use of Git LFS.
for i in ${TABLES} ; do
	if [[ ${blacklist} == *${i}* ]]; then
		echo "Dumping $i without data"
		MYSQL_PWD="$MYSQL_PASSWORD" mysqldump --add-drop-table -d \
			--host="$MYSQL_HOST" --port="$MYSQL_PORT" \
			--user="$MYSQL_USER" "$DB_NAME" "$i" | gzip > "dump/${i}.sql.gz"
	else
		echo "Dumping $i"
		MYSQL_PWD="$MYSQL_PASSWORD" mysqldump --add-drop-table \
			--host="$MYSQL_HOST" --port="$MYSQL_PORT" \
			--user="$MYSQL_USER" "$DB_NAME" "$i" | gzip > "dump/${i}.sql.gz"
	fi
done
