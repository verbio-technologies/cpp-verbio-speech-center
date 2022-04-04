# C++ integration with the Verbio Speech Center

This repository contains a C++ example of how to use the Verbio Technologies Speech Center cloud.

## Requirements
### Starting requirements
In order to compile the source code you will need g++>=7, cmake-3.>= 20 and conan >= 1.3

Before you start you will need: 
1. Speech center proto file (provided in this repository)
2. Platform access token (provided to you by Verbio Technologies) if you do not have one, ask for one [here](https://www.speechcenter.verbio.com/contact)
3. Speech center endpoint (https://www.speech-center.verbio.com:2424)

### Conan requirements
```shell
protobuf/3.19.2
grpc/1.43.0
cxxopts/3.0.0
spdlog/1.9.2
libsndfile/1.0.31
```
they are already writen in the `conanfile.txt` file in this repository.

The grp and protobuf packages are necessary to automatically generate from the .proto specification all the necessary code that the main code will use to connect with the gRPC server in the cloud.

## Step by step
The steps to compile the code are standard to any other conan-cmake project programmed in C++

### Install dependencies
The repository is already configure with conan dependencies that can be installed just by executing the standard conan install command:
```
conant install .
```

### Generate the gRPC code for C++
The are is already a step in the cmake configuration of this project that will generate the gRPC C++ code for. You do not have to worry about it. 

### Configuration
You can use a standard cmake call to configure the project:
```
cmake .
```
this will generate all the configuration files needed and will create all the necessary C++ from the .proto file that will allow your code to communicate with the Speech Center platform.

### Compile
You can use cmake to compile the code:
```
cmake --build . --target all 
```
Once the project is compile you can check that everything went as expected by executing the unit tests:
```
ctest
```
### Run the client
The cli_client will be using the generated C++ code to connect to the Speech Center cloud to process you speech file.
```shell
./>$ ./cli_client --help
Verbio Technlogies S.L. - Speech Center client example
Usage:
./cli_client [OPTION...]

-a file     Path to a .wav audio in 8kHz and PCM16 encoding to use for the recognition
-b file     Path to the ABNF grammar file to use for the recognition (optional)
-t arg      Topic to use for the recognition when a grammar is not provided. Must be GENERIC | BANKING | TELCO (default: generic)
-l arg      Language to use for the recognition: es-ES, en-US or pt-BR. (default: en-US)
-T arg      Path to the authentication token file
-e arg      End point to send requests (The URL of the host or server trying to reach) (default: speechcenter.verbio.com:2424)
-h, --help  this help message
```

 #### Example:
 ```
 ./cli_client -a audio.wav -t generic -l en-US -T your-speech-center.token 
 [2022-02-24 18:16:16.908] [info] [SpeechCenterClient.cpp:88] FINAL RESPONSE: -Recognition results comes here- 
 ```







