FROM conanio/gcc11-ubuntu18.04

USER root

RUN apt update && \
    pip install conan==1.57.0 --upgrade

WORKDIR /asr-client

COPY . .

RUN mkdir build-asr-cpp-client \
    && cd build-asr-cpp-client \
    && conan install .. --build=libcurl \
    && cmake .. -DUSE_CXX11_ABI_0=OFF \
    && cmake --build . \
    && cp src/cli_client /usr/local/bin/