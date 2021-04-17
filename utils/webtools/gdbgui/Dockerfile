# This script constitutes the reproducible build environment for EVEmu.
# Always build EVEmu binaries using docker in a reproducible fashion for issue reporting.

# This is a multi Stage Build, we start by makeing the base image we will use.
FROM quay.io/fedora/fedora:33-x86_64 as base
RUN dnf groupinstall -y "Development Tools" && dnf install -y cmake git zlib-devel mariadb-devel boost-devel tinyxml-devel utf8cpp-devel mariadb shadow-utils gdb pip python3-devel netcat net-tools
RUN pip3 install --user gdbgui

# Now we use the base image to build the project
FROM base as gdbgui
# We Add what we need for the build, if you need to add more; remember you may need to update .dockerignore
ADD start.sh /
# ADD ../../config.h.in /src/
# ADD ../../cmake/ /src/cmake
# ADD ../../dep/ /src/dep
# ADD ../../src/ /src/src
# ADD ../../utils/ /src/utils

# make some folders we need for the build
RUN mkdir -p /src/build /app /app/logs /app/server_cache /app/image_cache
# set our default path for the build
WORKDIR /src/
EXPOSE 8080

# Start the app via the script.
CMD /start.sh