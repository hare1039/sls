#!/bin/bash

set -e;

rm -f exec hello.zip;

docker build -t hare1039/slsfs-datafunction:nightly --build-arg CPPBUILDARGS="$CPPBUILDARGS" -f Dockerfile ..

docker run --rm hare1039/slsfs-datafunction:nightly cat /action/exec > exec

chmod +x exec
