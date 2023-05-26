FROM ubuntu:22.10

RUN apt update && \
    apt install -y build-essential python3 cmake python3-pip

RUN pip3 install conan==1.57.0

WORKDIR /asr-client

COPY . .

RUN mkdir build-asr-cpp-client \
    && cd build-asr-cpp-client \
    && conan install .. --build=missing

RUN cd build-asr-cpp-client \
    && cmake .. \
    && cmake --build . \
    && cp src/cli_client /usr/local/bin/ \
    && cd .. \
    && rm -rf *