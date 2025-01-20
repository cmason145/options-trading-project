# Build stage
FROM ubuntu:22.04 as build

# Environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV GRPC_RELEASE_TAG v1.16.0
ENV TRADING_BUILD_PATH /usr/local/trading

# Install basic dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    wget \
    autoconf \
    automake \
    libtool \
    curl \
    libssl-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libboost-all-dev \
    libzmq3-dev \
    libtbb-dev \
    libquantlib0v5 \
    libquantlib0-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone and build gRPC
RUN git clone -b ${GRPC_RELEASE_TAG} https://github.com/grpc/grpc /var/local/git/grpc && \
    cd /var/local/git/grpc && \
    git submodule update --init --recursive

# Install protobuf
RUN cd /var/local/git/grpc/third_party/protobuf && \
    ./autogen.sh && \
    ./configure --enable-shared && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Install gRPC
RUN cd /var/local/git/grpc && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Copy source code
COPY . $TRADING_BUILD_PATH/src/trading/

# Build trading platform
RUN mkdir -p $TRADING_BUILD_PATH/out/trading && \
    cd $TRADING_BUILD_PATH/out/trading && \
    cmake -DCMAKE_BUILD_TYPE=Release $TRADING_BUILD_PATH/src/trading && \
    make -j$(nproc) && \
    mkdir -p bin && \
    ldd trading_server | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' bin/ && \
    mv trading_server bin/trading_server && \
    echo "LD_LIBRARY_PATH=/opt/trading/:\$LD_LIBRARY_PATH ./trading_server" > bin/start.sh && \
    chmod +x bin/start.sh

# Runtime stage
FROM ubuntu:22.04 as runtime

# Copy built binaries and libraries
COPY --from=build /usr/local/trading/out/trading/bin/ /opt/trading/

# Expose gRPC port
EXPOSE 50051

# Set working directory
WORKDIR /opt/trading/

# Start trading server
ENTRYPOINT ["/bin/bash", "start.sh"]