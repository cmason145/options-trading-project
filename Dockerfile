# Base image
FROM ubuntu:22.04

# Avoid interactive dialog during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Create developer user and group but don't switch to it yet
RUN mkdir -p /home/developer && \
    echo "developer:x:1000:1000:Developer,,,:/home/developer:/bin/bash" >> /etc/passwd && \
    echo "developer:x:1000:" >> /etc/group && \
    chown developer:developer /home/developer

# Install basic build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    wget \
    software-properties-common \
    curl \
    libssl-dev \
    libcurl4-openssl-dev\
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Vulkan Dependencies
RUN apt-get update && apt-get install -y \
    libvulkan-dev \
    && rm -rf /var/lib/apt/lists/*

# Install OpenGL dependencies
RUN apt-get update && apt-get install -y \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxkbcommon-x11-0 \
    libvulkan1 \
    && rm -rf /var/lib/apt/lists/*

# Add Qt repository and install Qt6
RUN apt-get update && apt-get install -y software-properties-common
RUN add-apt-repository ppa:oibaf/graphics-drivers

# Install Qt6 and components
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-base-private-dev \
    qt6-declarative-dev \
    libqt6charts6-dev \
    libqt6charts6 \
    qt6-tools-dev \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6opengl6-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Boost
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Install ZeroMQ
RUN apt-get update && apt-get install -y \
    libzmq3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install TBB
RUN apt-get update && apt-get install -y \
    libtbb-dev \
    && rm -rf /var/lib/apt/lists/*

# Install QuantLib
RUN apt-get update && apt-get install -y \
    libquantlib0v5 \
    libquantlib0-dev \
    && rm -rf /var/lib/apt/lists/*

# Install X11 dependencies for GUI applications
RUN apt-get update && apt-get install -y \
    libx11-xcb1 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    xauth \
    x11-apps \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

# Copy CMakeLists.txt and source files
COPY . .

RUN mkdir -p /tmp/.X11-unix && \
    chmod 1777 /tmp/.X11-unix

# Create build directory and set permissions
RUN mkdir -p build && \
    chown -R developer:developer /app && \
    chmod -R 777 /app/build

# Now switch to developer user
USER developer

# Set environment variables
ENV QT_QPA_PLATFORM=xcb
ENV CMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake

CMD ["bash"]