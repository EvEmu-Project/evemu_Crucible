#!/bin/bash

echo "Initializing EVEmu..."

#Initialize the database
/src/utils/container-scripts/db_init.sh

#Initialize configuration files
if [ ! -f "/app/etc/eve-server.xml" ]; then
    echo "eve-server.xml not found, installing..."
    cp /src/utils/config/eve-server.xml /app/etc/
fi
if [ ! -f "/app/etc/log.ini" ]; then
    echo "log.ini not found, installing..."
    cp /src/utils/config/log.ini /app/etc/
fi
if [ ! -f "/app/etc/devtools.raw" ]; then
    echo "devtools.raw not found, installing..."
    cp /src/utils/config/devtools.raw /app/etc/
fi

#Start eve-server
echo "Starting eve-server..."
cd /app/bin/
./eve-server
