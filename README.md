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

**Example**

```commandline
./>$ ./cli_client -a audiofile.wav -l en-US -t my.token -T generic -s 16000 -H eu.speechcenter.verbio.com -V V1
```

Which will give an output along these lines:

 #### Example:
 ```
[2022-12-15 11:43:39.022] [info] [RecognitionClient.cpp:88] Channel is ready. State 2
[2022-12-15 11:43:39.022] [info] [RecognitionClient.cpp:89] Channel configuration: {}
[2022-12-15 11:43:39.022] [info] [RecognitionClient.cpp:98] Stream CREATED. State 2
[2022-12-15 11:43:39.024] [info] [RecognitionClient.cpp:103] WRITE: STARTING...
[2022-12-15 11:43:39.024] [info] [RecognitionClient.cpp:105] Sending config:
(...)
 ```

Use the `--help` command for more options.
