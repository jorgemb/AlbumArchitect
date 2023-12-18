# BUILD STAGE
FROM nvidia/cuda:12.3.1-devel-ubuntu22.04 as configure
LABEL authors="Jorge L. Martínez"

WORKDIR /app
RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    bison \
    cmake \
    curl \
    zip \
    unzip \
    tar \
    clang-15 \
    clang-tidy \
    libstdc++-12-dev \
    cppcheck \
    pkg-config \
    jq \
    libcudnn8-dev \
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
ENV CC=/usr/bin/clang-15
ENV CXX=/usr/bin/clang++-15
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

