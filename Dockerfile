# This script constitutes the reproducible build environment for EVEmu.
# Always build EVEmu binaries using docker in a reproducible fashion for issue reporting.
# Author: jdhirst

FROM quay.io/evemu/build-image:latest
LABEL description="EVEmu Server"

ADD . /src
RUN mkdir -p /src/build /app /app/logs /app/server_cache /app/image_cache
WORKDIR /src/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/app -DCMAKE_BUILD_TYPE=Debug ..
RUN make -j12
RUN make install

CMD /src/utils/container-scripts/start.sh
