#!/bin/bash

echo "Downloading latest EVEDBTool..."

# Check if curl is available or not
if ! command -v curl &> /dev/null
then
    echo "Wget or curl not found, please install one of these."
    exit
fi

arch=$(uname -i)

if [[ $arch == aarch64* ]];
then
echo "Using aarch64 build..."
# Download with curl
DOWNLOAD_URL=$(curl -s https://api.github.com/repos/EvEmu-Project/EVEDBTool/releases/latest \
        | grep browser_download_url \
        | grep evedb_aarch64 \
        | cut -d '"' -f 4)
fi

if [[ $arch == x86_64* ]];
then
echo "Using x86_64 build..."
# Download with curl
DOWNLOAD_URL=$(curl -s https://api.github.com/repos/EvEmu-Project/EVEDBTool/releases/latest \
        | grep browser_download_url \
        | grep evedbtool | grep -v exe \
        | cut -d '"' -f 4)
fi

curl --output evedbtool -s -L "$DOWNLOAD_URL"
chmod +x evedbtool
