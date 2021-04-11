#!/bin/bash

echo "Downloading latest EVEDBTool..."

# Check if curl is available or not
if ! command -v curl &> /dev/null
then
    echo "Wget or curl not found, please install one of these."
    exit
fi

# Download with curl
DOWNLOAD_URL=$(curl -s https://api.github.com/repos/EvEmu-Project/EVEDBTool/releases/latest \
        | grep browser_download_url \
        | grep evedbtool \
        | cut -d '"' -f 4)
curl -O -s -L "$DOWNLOAD_URL"
chmod +x evedbtool
