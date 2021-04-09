#!/bin/bash

host="127.0.0.1"    #Database Host
port="3306"     #Database Port
user="evemu"     #Database Username
pass="evemu"     #Database Password

database="evemutest"   #Database name

# Print usage
if [[ $# -eq 0 ]] ; then
    echo 'Usage: ./create-migration.sh <migration name>'
    exit 0
fi

# Check if mysql client is available or not
if ! command -v mysql &> /dev/null
then
    echo "Client not found, please install the latest MariaDB client"
    exit
fi

# Check if golang is available or not
if ! command -v go &> /dev/null
then
    echo "Golang not found, please install golang from your package manager"
    exit
fi

# Set path for go binaries
export PATH=$PATH:$HOME/go/bin

# Check if sql-migrate is available or not
if ! command -v sql-migrate &> /dev/null
then
    echo "Sql-migrate not found, installing using go get..."
    go get -v github.com/rubenv/sql-migrate/...
fi

# Write the dbconfig.yml file based upon above variables
cat > dbconfig.yml<<EOF
development:
    dialect: mysql
    datasource: $user:$pass@tcp(127.0.0.1:3306)/$database?parseTime=true
    dir: migrations
    table: migrations
EOF

sql-migrate new $1