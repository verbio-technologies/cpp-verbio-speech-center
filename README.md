# C++ integration with the Verbio Speech Center

This repository contains a C++ example of how to use the Verbio Technologies Speech Center cloud.

## Requirements
### Starting requirements
In order to compile the source code you will need g++>=7, cmake-3.>= 20 and conan >= 1.3

Before you start you will need: 
1. Speech center proto file (provided in this repository)
2. Platform access token (provided to you by Verbio Technologies) if you do not have one, register on our [website](https://www.speechcenter.verbio.com)
3. Speech center CSR endpoint (csr.api.speechcenter.verbio.com)

## Step by step

It is recommended to create a `/build` subdirectory in the project root, and run all the following commands from there:

### Install conan dependencies

They are already writen in the `conanfile.txt` file in this repository.

The grp and protobuf packages are necessary to automatically generate from the .proto specification all the necessary code that the main code will use to connect with the gRPC server in the cloud.

To install, (from `/build`) run:
```
conan install ..
```

This will also set up the necessary files for CMake build inside the `/build` folder.

### Generate the gRPC code for C++

There is already a step in the CMake configuration of this project that will generate the C++ code for gRPC. You do not have to worry about it. 

### Configuration
From `/build`:
```
cmake .
```

This will generate all the configuration files needed and will create all the necessary C++ from the .proto file that will allow your code to communicate with the Speech Center platform.

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
```commandline
./>$ ./cli_client --help
Verbio Technlogies S.L. - Speech Center client example
Usage:
./cli_client [OPTION...]

-a file     Path to a .wav audio in 8kHz and PCM16 encoding to use for the recognition
-b file     Path to the ABNF grammar file to use for the recognition (optional)
-t arg      Topic to use for the recognition when a grammar is not provided. Must be GENERIC | BANKING | TELCO | INSURANCE (default: generic)
-l arg      Language to use for the recognition: es-ES, en-US or pt-BR. (default: en-US)
-T arg      Path to the authentication token file
-r arg      The sample rate in hz of the audio file (default: 16000, and currently the only one supported)
-e arg      End point to send requests (The URL of the host or server trying to reach) (default: csr.api.speechcenter.verbio.com)
-h, --help  this help message
```

 #### Example:
 ```
 ./cli_client -a audio.wav -t generic -l en-US -T your-speech-center.token 
 [2022-02-24 18:16:16.908] [info] [SpeechCenterClient.cpp:88] FINAL RESPONSE: -Recognition results comes here- 
 ```







