FROM ubuntu:20.04

WORKDIR /opt

RUN apt-get update \
    # Dev dependencies
    && apt-get install -y git python3 pip build-essential wget \
    #
    # Install latest Cmake version
    #
    && wget -O /opt/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.19.2/cmake-3.19.2-Linux-x86_64.tar.gz \
    && tar -zxvf /opt/cmake.tar.gz -C /opt/ \
    && find /opt/cmake* -wholename '*/bin/cmake' -exec update-alternatives --install /usr/bin/cmake cmake {} 25 \; \
    && find /opt/cmake* -wholename '*/bin/ctest' -exec update-alternatives --install /usr/bin/ctest ctest {} 25 \; \
    && find /opt/cmake* -wholename '*/bin/cpack' -exec update-alternatives --install /usr/bin/cpack cpack {} 25 \; \
    && rm /opt/cmake*.tar.gz \
    # Install conan C++ package manager
    && pip3 install -I conan==1.45.0 conan_package_tools \
    && rm -rf /var/lib/apt/lists/*