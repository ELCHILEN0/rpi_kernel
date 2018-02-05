FROM centos

SHELL ["/bin/bash", "-c"]

ARG CT_NG_VERSION=crosstool-ng-1.23.0

# Fetch dependencies
RUN yum install -y wget which make file
RUN yum install -y autoconf gperf bison flex texinfo help2man gcc-c++ patch \
		   ncurses-devel python-devel perl-Thread-Queue bzip2 git
RUN yum install -y bc

# Prepare cross compiler
WORKDIR /root
RUN wget http://crosstool-ng.org/download/crosstool-ng/$CT_NG_VERSION.tar.xz
RUN tar -xvf $CT_NG_VERSION.tar.xz

# Build cross compiler scripts
WORKDIR $CT_NG_VERSION
RUN ./configure
RUN make
RUN make install

# Build cross compiler - 64 bit
WORKDIR /root
RUN ulimit -n 2048
RUN ct-ng aarch64-rpi3-linux-gnueabi
RUN echo "CT_ALLOW_BUILD_AS_ROOT_SURE=y" >> .config
RUN ct-ng build

# Build cross compiler - 32 bit
RUN ct-ng armv8-rpi3-linux-gnueabihf
RUN echo "CT_ALLOW_BUILD_AS_ROOT_SURE=y" >> .config
RUN ct-ng build

# Cleanup
RUN rm -rf config
RUN rm -rf config.gen
RUN rm -rf $CT_NG_VERSION
RUN rm -rf $CT_NG_VERSION.tar.xz
