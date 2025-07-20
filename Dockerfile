# Base image for building EVEmu using Debian 12
FROM debian:12 AS base

# Install build dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    zlib1g-dev \
    libmariadb-dev \
    libboost-all-dev \
    libtinyxml-dev \
    ca-certificates \
    g++ \
    gdb \
    libutfcpp-dev \
    mariadb-client \
    passwd \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Build stage
FROM base AS app-build

# Add project files
COPY CMakeLists.txt /src/
COPY config.h.in /src/
COPY /cmake/ /src/cmake
COPY /dep/ /src/dep
COPY /src/ /src/src
COPY /utils/ /src/utils
# Included for cmake to read git rev-hash
COPY /.git/ /src/.git

# Create necessary directories
RUN mkdir -p /src/build /app /app/logs /app/server_cache /app/image_cache

ENV MYSQL_INCLUDE_DIR="/usr/include/mariadb"
ENV MYSQL_LIBRARIES="/usr/lib/x86_64-linux-gnu/libmariadbclient.so"

# Set working directory
WORKDIR /src/build

# Configure and build the project
RUN cmake -DCMAKE_INSTALL_PREFIX=/app -DCMAKE_BUILD_TYPE=Debug .. 
RUN make -j$(nproc)
RUN make install

# Final runtime image
FROM base AS app

LABEL description="EVEmu Server"

# Copy built assets
COPY --from=app-build /src/utils/ /src/utils
COPY --from=app-build /app/ /app

# Add SQL loading tools
COPY /sql/ /src/sql

# Run SQL tool script
RUN cd /src/sql && ./get_evedbtool.sh

# Expose server ports
EXPOSE 26000
EXPOSE 26001

# Default command
CMD ["/src/utils/container-scripts/start.sh"]
