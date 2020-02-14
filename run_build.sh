#!/bin/bash

pushd docker && docker build -t gem5:build . && popd
docker run --rm -t -i --user $(id -u):$(id -g) --name=m5 -v $PWD:/build gem5:build ./build.sh
