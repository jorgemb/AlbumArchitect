FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    bison \
    build-essential \
    clang-tidy \
    cmake \
    cppcheck \
    curl \
    git \
    gcc-14 \
    g++-14 \
    libtool \
    libx11-dev \
    libxext-dev \
    libxft-dev \
    libxi-dev \
    libxrandr-dev \
    libxtst-dev \
    nasm \
    ninja-build \
    openimageio-tools \
    pkg-config \
    python3 \
    python3-setuptools \
    python3-jinja2 \
    tar \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

# Get VCPKG and install dependencies
ENV VCPKG_ROOT="/vcpkg"
ENV CC="/usr/bin/gcc-14"
ENV CXX="/usr/bin/g++-14"

WORKDIR $VCPKG_ROOT
COPY vcpkg.json vcpkg.json

RUN VCPKG_BASELINE=$(cat vcpkg.json | grep "builtin-baseline" | cut -d"\"" -f4) \
    && git init . \
    && git remote add origin https://github.com/microsoft/vcpkg \
    && git fetch origin $VCPKG_BASELINE \
    && git reset --hard FETCH_HEAD \
    && ./bootstrap-vcpkg.sh \
    && ./vcpkg install --clean-after-build --x-feature=test

# Configure and build project
WORKDIR /app
COPY cmake cmake
COPY source source
COPY test test
COPY CMakeLists.txt CMakePresets.json vcpkg.json ./

RUN mkdir build \
    && cmake -B build/ --preset=ci-ubuntu . \
    && cmake --build build --config Release -j 4

# .. perform tests
WORKDIR /app/build
RUN ctest --output-on-failure --no-tests=error -C Release -j 2

# .. create distributable
WORKDIR /app
RUN cmake --install build --config Release --prefix dist
#    && for file in $(ldd dist/bin/AlbumArchitect | grep "=>" | cut -d" " -f3); do cp $file dist/bin/ ; done



FROM ubuntu:24.04 AS runtime

# TODO: Create non-root user

WORKDIR /app
COPY --from=build /app/dist/bin/ .

ENTRYPOINT ["/app/AlbumArchitect"]
SHELL ["--help"]