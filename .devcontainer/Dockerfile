
FROM centos:centos7.9.2009

# yum
RUN yum -y install which zlib zlib-devel curl perl-ExtUtils-CBuilder perl-ExtUtils-MakeMaker gcc gcc-c++ curl-devel expat-devel gettext-devel openssl-devel expect make wget gettext zip unzip scl-utils

# cmake
RUN mkdir -p /data/App \
    && cd /data/App \
    && wget https://cmake.org/files/v3.24/cmake-3.24.0-rc4-linux-x86_64.tar.gz --no-check-certificate \
    && tar zxvf cmake-3.24.0-rc4-linux-x86_64.tar.gz \
    && mv cmake-3.24.0-rc4-linux-x86_64 cmake

# gcc
RUN yum -y install centos-release-scl scl-utils \
    && yum -y install devtoolset-7-gcc* 

# git
RUN cd /data/App \
    && wget https://github.com/git/git/archive/refs/tags/v2.37.1.tar.gz \
    && tar -zxvf v2.37.1.tar.gz \
    && rm *.tar.gz \
    && cd git-2.37.1 \
    && source /opt/rh/devtoolset-7/enable \
    && gcc --version \
    && make prefix=/usr/local/git all \
    && make prefix=/usr/local/git install

# ENV
ENV LANG=en_US.UTF-8 \
    CMAKE_HOME=/data/App/cmake \
    GCC_HOME=/opt/rh/devtoolset-7/root/usr/ \
    CC=/opt/rh/devtoolset-7/root/usr/bin/gcc \
    CXX=/opt/rh/devtoolset-7/root/usr/bin/g++ \
    LD_LIBRARY_PATH=/opt/rh/devtoolset-7/root/usr/lib64:$LD_LIBRARY_PATH \
    PATH=/usr/local/git/bin:/opt/rh/devtoolset-7/root/usr/bin:/data/App/cmake/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
