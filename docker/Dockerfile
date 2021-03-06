FROM ubuntu:16.04
MAINTAINER "Andrew Smith <atsmith3@illinois.edu>"

ARG wd
ARG gid
ARG uid
ARG user

RUN apt-get update && \
    apt-get install -y gcc-multilib make valgrind git gdb build-essential && \
    apt-get install -y flex bison autoconf zlib1g-dev libgoogle-perftools-dev && \
    apt-get install -y libboost-all-dev

RUN apt-get update && \
    apt-get install -y gcc make python python-pip && \
    pip install --upgrade pip && \
    pip install six scons pybind11 file_read_backwards

RUN apt-get update && \
    apt-get install -y g++ git wget

# Install Boost:
# https://github.com/pblischak/boost-docker-test/blob/master/Dockerfile
RUN cd /home && wget \
    http://downloads.sourceforge.net/project/boost/boost/1.73.0/boost_1_73_0.tar.gz \
    && tar xfz boost_1_73_0.tar.gz \
    && rm boost_1_73_0.tar.gz \
    && cd boost_1_73_0 \
    && ./bootstrap.sh --prefix=/usr/local \
    && ./b2 install \
    && cd /home \
    && rm -rf boost_1_73_0

RUN groupadd -g ${gid} sim
RUN useradd -rm -d /home/${user} -s /bin/bash -g root -G sim -u ${uid} ${user}

WORKDIR ${wd}
