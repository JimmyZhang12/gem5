FROM ubuntu:16.04
MAINTAINER "Andrew Smith <atsmith3@illinois.edu>"

ARG wd
ARG gid
ARG uid
ARG user

RUN apt-get update && \
    apt-get install -y gcc-multilib make valgrind git gdb build-essential && \
    apt-get install -y flex bison autoconf zlib1g-dev libgoogle-perftools-dev

RUN apt-get update && \
    apt-get install -y gcc make python python-pip && \
    pip install --upgrade pip && \
    pip install six scons pybind11 file_read_backwards

RUN groupadd -g ${gid} sim
RUN useradd -rm -d /home/${user} -s /bin/bash -g root -G sim -u ${uid} ${user}

WORKDIR ${wd}
