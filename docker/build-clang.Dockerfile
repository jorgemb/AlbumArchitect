FROM ubuntu:24.04 AS build

ENV TZ=UTC \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    bison \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    codespell \
    cppcheck \
    curl \
    git \
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
    python3-jinja2 \
    python3-setuptools \
    tar \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

# Get VCPKG and install dependencies
ENV CC="/usr/bin/clang"
ENV CXX="/usr/bin/clang++"


# Set alternatives to programs
#RUN update-alternatives --install \
#    /usr/bin/clang-tidy clang-tidy \
#    /usr/bin/clang-tidy-14 140

ENV VCPKG_ROOT="/vcpkg"
ARG VCPKG_LOCAL_CACHE="/cache/vcpkg/"
RUN git clone https://github.com/microsoft/vcpkg

# Binary sources dir helps with caching values on an external site
ARG VCPKG_EXTERNAL_BINARY_SOURCES
ENV VCPKG_BINARY_SOURCES="clear;files,${VCPKG_LOCAL_CACHE},readwrite;${VCPKG_EXTERNAL_BINARY_SOURCES}"

WORKDIR $VCPKG_ROOT
COPY vcpkg.json vcpkg.json
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    ./bootstrap-vcpkg.sh \
    && ./vcpkg install --x-feature=test

# Configure and build project
WORKDIR /app
COPY cmake cmake
COPY source source
COPY test test
COPY CMakeLists.txt CMakePresets.json vcpkg.json .codespellrc .clang-format .clang-tidy ./

# Lint and spelling
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    cmake -D FORMAT_COMMAND=clang-format -P cmake/lint.cmake -B build/ . \
    && cmake -P cmake/spell.cmake

# Sanitize and test
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    cmake --preset=ci-sanitize . \
    && cmake --build build/sanitize -j 4

# .. perform build and tests
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    cmake --preset=ci-ubuntu . && \
    cmake --build build --config Release -j 4

WORKDIR /app/build
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    ctest --output-on-failure --no-tests=error -C Release -j 4


# Build
WORKDIR /app
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    cmake --preset=ci-ubuntu . \
    && cmake --build build --config Release -j 4

# .. create distributable
WORKDIR /app
RUN --mount=type=cache,sharing=locked,target=${VCPKG_LOCAL_CACHE} \
    cmake --install build --config Release --prefix dist
#    && for file in $(ldd dist/bin/AlbumArchitect | grep "=>" | cut -d" " -f3); do cp $file dist/bin/ ; done


FROM ubuntu:24.04 AS runtime

# Copy application
WORKDIR /app
COPY --from=build /app/dist/bin/ .

ENTRYPOINT ["/app/AlbumArchitect"]
SHELL ["--help"]