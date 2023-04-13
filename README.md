# C++ integration with the Verbio Speech Center

This repository contains a C++ example of how to use the Verbio Technologies Speech Center cloud.

## Requirements
### Starting requirements
In order to compile the source code you will need g++>=7, cmake-3.>= 20 and conan >= 1.3

Before you start you will need: 
1. Speech center proto file (provided in this repository)
2. Platform access token (provided to you by Verbio Technologies) if you do not have one, check our [documentation](https://speechcenter.verbio.com/documentation)
3. Speech center CSR endpoint

## Step by step

### Check conan version

Run:
```shell
conan --version
```
if the version is < 1.3 or >= 2.* you need to check the documentation at:

[https://docs.conan.io/1/installation.html]()

on how to install conan with virtualenvs, and install a compatible version with a command like:
```shell
pip install conan==1.54.0
```

### Create build directory

It is recommended to create a `/build` subdirectory in the project root, and run all the following commands from this build directory.

### Install conan dependencies

Conan dependencies are specified in `conanfile.txt` file in this repository.

To install those dependencies (from `/build`) run:
```
conan install ..
```
This will also set up the necessary files for CMake build inside the `/build` folder.


### Configuration
From `/build`:
```
cmake ..
```

This will generate all the configuration files needed.

### Compile

You can use cmake to compile the code:
```
cmake --build . --target all 
```
Once the project is compiled you can check that everything went as expected by executing the unit tests:
```
ctest
```
### Run the client
The cli_client will be using the generated C++ code to connect to the Speech Center cloud to process you speech file.

**Example**

```shell
./cli_client -a audio.wav -T GENERIC -t token_file -l en-US --asr-version V1  -H us.speechcenter.verbio.com -s 16000 --labels 'project1 project2'
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

You can also run:
```shell
./cli_client --help
```
to list all options.


## Automatically Refresh Service Token
This repository optionally implements an automatic token update. To do so, you must specify your client-id and client-secret (find them in the Client Credentials section of the [user dashboard](https://dashboard.speechcenter.verbio.com)).

You must also specify a token file, where the token will be stored and updated in case it is invalid or expired.

**Example**
```console
./cli_client -a audio.wav -T GENERIC -t token_file -l en-US --asr-version V2  -H us.speechcenter.verbio.com -s 16000 --labels --client-id 'your-client-id' --client-secret 'your-client-secret'

```