#!/bin/bash
ORIGINAL_DIR=$(pwd)

mkdir -p ../build
cd ../build
cmake .. -DLOCAL_DEBUG=ON
make -j 4

cd "$ORIGINAL_DIR"

