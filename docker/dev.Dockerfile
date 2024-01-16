FROM nvidia/cuda:12.3.1-devel-ubuntu22.04

RUN apt-get update && apt-get install -y \
    bison \
    build-essential \
    clang \
    clang-tidy \
    curl \
    cmake \
    cppcheck \
    gdb \
    git \
    pkg-config \
    tar \
    unzip \
    zip \
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
    libsystemd-dev \
    # OpenImageIO packages
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-common-dev \
    libxrandr-dev \
    libxxf86vm-dev \
    nasm  \
    # dlib
    libcudnn8-dev \
    nvidia-cuda-dev \
    gfortran \
    p7zip-full \
    && rm -rf /var/lib/apt/lists/*

# Add build user

ARG UID=1000
ENV VCPKG_ROOT=/data/vcpkg
RUN mkdir -p ${VCPKG_ROOT}

RUN useradd -m -u ${UID} -s /bin/bash builder \
    && chown -R builder:builder ${VCPKG_ROOT}
USER builder


# Clone vcpkg
RUN git clone https://github.com/microsoft/vcpkg ${VCPKG_ROOT}
