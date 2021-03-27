#/bin/bash
MYSQL_USER=evemu
MYSQL_PASS=evemu
DB_NAME=evemu

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
TABLES=$(echo $SQL_STRING | mysql -u$MYSQL_USER -p$MYSQL_PASS $DB_NAME -Bs)

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
		mysqldump --add-drop-table -d -u $MYSQL_USER -p$MYSQL_PASS $DB_NAME $i | gzip > "dump/${i}.sql.gz"
	else
		echo "Dumping $i"
		mysqldump --add-drop-table -u $MYSQL_USER -p$MYSQL_PASS $DB_NAME $i | gzip > "dump/${i}.sql.gz"
	fi
done
