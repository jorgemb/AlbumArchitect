FROM ubuntu:24.04 AS base

ENV TZ=UTC \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    bison \
    build-essential \
    clang-tidy \
    clang-format \
    clang \
    cmake \
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
    python3-setuptools \
    python3-jinja2 \
    tar \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

# Get VCPKG and install dependencies
ENV CC="/usr/bin/clang"
ENV CXX="/usr/bin/clang++"


FROM base AS build

# Set alternatives to programs
#RUN update-alternatives --install \
#    /usr/bin/clang-tidy clang-tidy \
#    /usr/bin/clang-tidy-14 140

ENV VCPKG_ROOT="/vcpkg"
WORKDIR $VCPKG_ROOT
COPY vcpkg.json vcpkg.json

RUN --mount=type=cache,target=$VCPKG_ROOT/packages \
    VCPKG_BASELINE=$(cat vcpkg.json | grep "builtin-baseline" | cut -d"\"" -f4) \
    && git init . \
    && git remote add origin https://github.com/microsoft/vcpkg \
    && git fetch origin $VCPKG_BASELINE \
    && git reset --hard FETCH_HEAD \
    && ./bootstrap-vcpkg.sh \
    && ./vcpkg install --clean-after-build --x-feature=test

FROM build AS configure

# Configure and build project
WORKDIR /app
COPY cmake cmake
COPY source source
COPY test test
COPY CMakeLists.txt CMakePresets.json vcpkg.json ./

# Lint and spelling
RUN --mount=type=cache,target=/app/build/ \
    cmake -D FORMAT_COMMAND=clang-format -P cmake/lint.cmake \
    && cmake -P cmake/spell.cmake

RUN --mount=type=cache,target=/app/build/ \
    cmake -B build/ --preset=ci-sanitize . \
    && cmake --build build/sanitize -j 4

# .. perform tests
WORKDIR /app/build
RUN --mount=type=cache,target=/app/build/ \
    ctest --output-on-failure --no-tests=error -j 4 \

FROM build as release

# Build
RUN --mount=type=cache,target=/app/build/ \
    cmake -B build/ --preset=ci-ubuntu . \
    && cmake --build build --config Release -j 4

# .. create distributable
WORKDIR /app
RUN --mount=type=cache,target=/app/build/ \
    cmake --install build --config Release --prefix dist
#    && for file in $(ldd dist/bin/AlbumArchitect | grep "=>" | cut -d" " -f3); do cp $file dist/bin/ ; done


FROM ubuntu:24.04 AS runtime

# Copy application
WORKDIR /app
COPY --from=configure /app/dist/bin/ .

ENTRYPOINT ["/app/AlbumArchitect"]
SHELL ["--help"]