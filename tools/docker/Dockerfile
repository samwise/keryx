FROM ubuntu:18.04
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y \
 autoconf \
 automake \
 bsdmainutils \
 ccache \
 clang-format \
 clang \
 cmake \
 curl \
 emacs \
 flex \
 g++ \
 gcc \
 gdb \
 git \
 glibc-source \
 htop  \
 iputils-ping \
 libc6-dbg \
 libclang-dev \
 liblttng-ust-dev \
 libncurses5-dev \
 libreadline-dev \
 libtool \
 libz-dev \
 llvm \
 locales \
 lttng-modules-dkms \
 lttng-tools \
 make \
 net-tools \
 ninja-build \
 openssh-server \
 pkg-config \
 python-pip \
 screen \
 silversearcher-ag \
 sudo \
 tcl \
 tcsh \
 texinfo \
 tmux \
 tree \
 unzip \
 uuid-dev \
 vim \
 sqlite3 \
 libsqlite3-dev

# fix locale so we can run tmux
run sed -i 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen
run locale-gen

# expand libc sources
run cd /usr/src/glibc ; tar xvf glibc-*.tar.xz

# get and build rtags
run git clone --recursive https://github.com/Andersbakken/rtags.git && \
cd rtags && \
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 . && \
make -j$(nproc) install && \
cd .. && \
rm -rf rtags




    
