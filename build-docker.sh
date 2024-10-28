#!/usr/bin/env bash

# Fail on any error
set -e

# Build the docker files
echo "Building base build image"
BASE_TAG=albumarchitect:clang
docker build -t $BASE_TAG -f docker/clang-base.Dockerfile .
