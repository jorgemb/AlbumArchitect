# BUILD STAGE
FROM ubuntu:23.10 as configure
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
    clang-tidy \
    clang-15 \
    cppcheck \
    pkg-config \
    linux-headers-generic \
    libx11-dev \
    libxft-dev \
    libxext-dev \
    libdbus-1-dev \
    libxi-dev \
    libxtst-dev \
    libxrandr-dev \
    python3 \
    python3-distutils \
    python-is-python3 \
    libcereal-dev \
    libgoogle-glog-dev \
    liblcms2-dev \
    libopenimageio-dev \
    openimageio-tools \
    libopencv-dev \
    libopencv-contrib-dev \
    libdlib-dev \
    libdlib-data \
    libavdevice-dev \
    libavfilter-dev \
    libavformat-dev \
    libavcodec-dev \
    libswresample-dev \
    libswscale-dev \
    libavutil-dev \
    libtesseract-dev \
    tesseract-ocr-eng \
    libboost1.81-all-dev \
    libopenblas-dev \
    liblapack-dev \
    && rm -rf rm -rf /var/lib/apt/lists/*

# Get vcpkg
ENV VCPKG_ROOT=/vcpkg
RUN --mount=type=cache,target=${VCPKG_ROOT},sharing=locked \
    git clone https://github.com/microsoft/vcpkg ${VCPKG_ROOT}

# Copy source code files
COPY . .
COPY vcpkg.ubuntu.json vcpkg.json

# Prepare for build
ENV CC=/usr/bin/clang-15
ENV CXX=/usr/bin/clang++-15
WORKDIR /app/build
RUN --mount=type=cache,target=${VCPKG_ROOT},sharing=locked \
    cmake ../ -DDLIB_BUILD=OFF -DDLIB_DOWNLOAD_DATA=OFF --preset=ci-ubuntu

RUN --mount=type=cache,target=${VCPKG_ROOT},sharing=locked \
    cmake --build . -j8
