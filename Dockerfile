# This script constitutes the reproducible build environment for EVEmu.
# Always build EVEmu binaries using docker in a reproducible fashion for issue reporting.

# This is a multi Stage Build, we start by makeing the base image we will use.
FROM quay.io/fedora/fedora:33-x86_64 as base
RUN dnf groupinstall -y "Development Tools" && dnf install -y cmake git zlib-devel mariadb-devel boost-devel tinyxml-devel utf8cpp-devel mariadb shadow-utils gdb

# Now we use the base image to build the project
FROM base as app-build
# We Add what we need for the build, if you need to add more; remember you may need to update .dockerignore
ADD CMakeLists.txt /src/
ADD config.h.in /src/
ADD /cmake/ /src/cmake
ADD /dep/ /src/dep
ADD /src/ /src/src
ADD /utils/ /src/utils

# make some folders we need for the build
RUN mkdir -p /src/build /app /app/logs /app/server_cache /app/image_cache
# set our default path for the build
WORKDIR /src/build
# and run the build
RUN cmake -DCMAKE_INSTALL_PREFIX=/app -DCMAKE_BUILD_TYPE=Debug .. 
# we can pull the # of cores on the system and change the build to match the system
RUN make -j$(nproc)
RUN make install

# Now we switch to makeing the image that will run the code that we have build
FROM base as app
LABEL description="EVEmu Server"
# copy our utils to this image
COPY --from=app-build /src/utils/ /src/utils
# copy our compiled code to this image
COPY --from=app-build /app/ /app
# install the configuration files
RUN cp /src/utils/config/log.ini /app/etc/; cp /src/utils/config/eve-server.xml /app/etc/
# add in the files to load the database
ADD /sql/ /src/sql

RUN cd /src/sql && ./get_evedbtool.sh

# Expose the port the server is on.
EXPOSE 26000
EXPOSE 26001

# Start the app via the script.
CMD /src/utils/container-scripts/start.sh
