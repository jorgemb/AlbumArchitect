# BUILD STAGE
FROM ubuntu:23.10 as configure
LABEL authors="Jorge L. Martínez"

WORKDIR /app
RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    gcc-13 \
    g++-13 \
    bison \
    cmake \
    curl \
    zip \
    unzip \
    tar \
    clang-tidy \
    cppcheck \
    pkg-config \
    jq \
    # OpenCV packages
    python3 \
    python-is-python3 \
    python3-distutils \
    libdbus-1-dev \
    libxi-dev \
    libxtst-dev \
    libx11-dev \
    libxft-dev \
    libxext-dev \
    libxrandr-dev \
    linux-libc-dev \
    gfortran \
    # OpenImageIO packages \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-common-dev \
    libxrandr-dev \
    libxxf86vm-dev \
    libsystemd-dev \
    nasm \
    # DLIB
    libdlib-dev \
    libdlib-data \
    && rm -rf rm -rf /var/lib/apt/lists/*

# Get vcpkg and install packages
ENV CC=/usr/bin/gcc-13
ENV CXX=/usr/bin/g++-13
ENV VCPKG_ROOT=/vcpkg

WORKDIR ${VCPKG_ROOT}
RUN git clone https://github.com/microsoft/vcpkg . \
    && sh bootstrap-vcpkg.sh
COPY vcpkg.json .
RUN ./vcpkg install --clean-after-build

# Copy source code files
WORKDIR /app
COPY . .

# Prepare for build
RUN --mount=type=cache,target=/app/build/vcpkg_installed \
    cmake -Bbuild/ -DDLIB_BUILD=OFF -DDLIB_DOWNLOAD_DATA=OFF --preset=ci-ubuntu .\
    && cmake --build build/

