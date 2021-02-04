# This script constitutes the reproducible build environment for EVEmu.
# Always build EVEmu binaries using docker in a reproducible fashion for issue reporting.
# Author: jdhirst1

FROM debian:buster AS builder

RUN apt update && apt install -y build-essential cmake git zlib1g-dev libmariadbclient-dev libboost-all-dev libutfcpp-dev libtinyxml-dev libmariadb-dev-compat
ADD . /src
WORKDIR /src/build
RUN cmake ../
RUN make -j12

FROM debian:buster
LABEL description="EVEmu Server container"

RUN apt update && apt install -y libtinyxml2.6.2v5 zlib1g libmariadb3 libboost-system1.67.0
COPY --from=builder /src/utils/eve-server.xml /opt/evemu/etc/eve-server.xml
COPY --from=builder /src/utils/log.ini /opt/evemu/etc/log.ini
COPY --from=builder /src/utils/devtools.raw /opt/evemu/etc/devtools.raw
COPY --from=builder /src/build/src/eve-server/eve-server /opt/evemu/bin/eve-server
RUN mkdir -p /opt/evemu/log /opt/evemu/server_cache /opt/evemu/image_cache

WORKDIR /opt/evemu/bin
CMD /opt/evemu/bin/eve-server
