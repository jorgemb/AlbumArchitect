FROM ubuntu:22.04 AS build

ENV TZ=US \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    autoconf \
    automake \
    bison \
    build-essential \
    clang-tidy-14 \
    clang-14 \
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
    libstdc++-12-dev \
    tar \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

# Get VCPKG and install dependencies
ENV VCPKG_ROOT="/vcpkg"
ENV CC="/usr/bin/clang-14"
ENV CXX="/usr/bin/clang++-14"

# Set alternatives to programs
RUN update-alternatives --install \
    /usr/bin/clang-tidy clang-tidy \
    /usr/bin/clang-tidy-14 140

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

# Configure and build project
WORKDIR /app
COPY cmake cmake
COPY source source
COPY test test
COPY CMakeLists.txt CMakePresets.json vcpkg.json ./

RUN --mount=type=cache,target=/app/build/ \
    cmake -B build/ --preset=ci-ubuntu . \
    && cmake --build build --config Release -j 4

# .. perform tests
WORKDIR /app/build
RUN --mount=type=cache,target=/app/build/ \
    ctest --output-on-failure --no-tests=error -C Release -j 4

# .. create distributable
WORKDIR /app
RUN --mount=type=cache,target=/app/build/ \
    cmake --install build --config Release --prefix dist
#    && for file in $(ldd dist/bin/AlbumArchitect | grep "=>" | cut -d" " -f3); do cp $file dist/bin/ ; done



FROM ubuntu:22.04 AS runtime

# Copy application
WORKDIR /app
COPY --from=build /app/dist/bin/ .

ENTRYPOINT ["/app/AlbumArchitect"]
SHELL ["--help"]