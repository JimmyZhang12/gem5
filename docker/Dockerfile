FROM ubuntu:16.04
MAINTAINER "Andrew Smith <atsmith3@illinois.edu>"
RUN apt-get update && \
    apt-get install -y gcc-multilib make valgrind git gdb build-essential && \
    apt-get install -y flex bison autoconf zlib1g-dev libgoogle-perftools-dev

RUN apt-get update && \
    apt-get install -y gcc make python python-pip && \
    pip install --upgrade pip && \
    pip install six scons pybind11 file_read_backwards

WORKDIR /build/
