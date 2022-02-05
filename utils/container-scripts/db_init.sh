#!/bin/bash

# This is a script to populate the db container with the sql table data.
# Author: James, Gwen

MARIADB_HOST="${MARIADB_HOST:-db}"
MARIADB_DATABASE="${MARIADB_DATABASE:-evemu}"
MARIADB_PASSWORD="${MARIADB_PASSWORD:-evemu}"
MARIADB_USER="${MARIADB_USER:-evemu}"
MARIADB_PORT="${MARIADB_PORT:-3306}"


# Get script path:
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

# Function to determine health of MariaDB container
function waitContainer {
    #Checking if we can actually connect to the container
    while ! mysql -h $MARIADB_HOST -u $MARIADB_USER -p$MARIADB_PASSWORD -e "use evemu;show tables;" >/dev/null 2>&1; do
        printf .
        sleep 1
    done
}

# Wait for container to be up and running...
echo "Waiting for DB to start..."
waitContainer evemu_db

#Write the eve-server.xml variables
sed -i "s/database_host/$MARIADB_HOST/" /src/utils/config/eve-server.xml
sed -i "s/database_username/$MARIADB_USER/" /src/utils/config/eve-server.xml
sed -i "s/database_password/$MARIADB_PASSWORD/" /src/utils/config/eve-server.xml
sed -i "s/database_name/$MARIADB_DATABASE/" /src/utils/config/eve-server.xml
sed -i "s/database_port/$MARIADB_PORT/" /src/utils/config/eve-server.xml
cp /src/utils/config/eve-server.xml /app/etc/eve-server.xml

# Write evedb.yaml based upon above variables
cd /src/sql
cat >/src/sql/evedb.yaml <<EOF
base-dir: /src/sql/base
db-database: $MARIADB_DATABASE
db-host: $MARIADB_HOST
db-pass: $MARIADB_PASSWORD
db-port: $MARIADB_PORT
db-user: $MARIADB_USER
log-level: Info
migrations-dir: /src/sql/migrations
EOF

echo "Running EVEDBTool..."
/src/sql/evedbtool install

if [ "$SEED_MARKET" == "TRUE" ]
then
    IFS=',' read -r -a array <<< "$SEED_REGIONS"
    echo "seed-saturation: $SEED_SATURATION" >> /src/sql/evedb.yaml
    echo "seed-regions: " >> /src/sql/evedb.yaml
    for i in "${array[@]}"
    do
        echo "- $i" >> /src/sql/evedb.yaml
    done
    /src/sql/evedbtool seed
fi