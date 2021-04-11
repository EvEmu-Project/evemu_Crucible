#!/bin/bash

# This is a script to populate the db container with the sql table data.
# Author: James

MARIADB_HOST=db
MARIADB_DATABASE=evemu
MARIADB_PASSWORD=evemu
MARIADB_USER=evemu

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

# Write evedb.yaml based upon above variables
cd /src/sql
cat >/src/evedb.yaml <<EOF
base-dir: /src/sql/base
db-database: evemu
db-host: db
db-pass: evemu
db-port: 3306
db-user: evemu
log-level: Info
migrations-dir: /src/sql/migrations
EOF

# Download latest version of evedbtool
/src/sql/get_evedbtool.sh

echo "Running EVEDBTool..."
/src/sql/evedbtool install