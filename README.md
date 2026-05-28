# C++ integration with the Verbio Speech Center

This repository contains a C++ example of how to use the Verbio Technologies Speech Center cloud.

## Requirements
### Starting requirements

Before you start you will need:
1. Dashboard user account (provided to you by Verbio Technologies), please contact our sales team at info@verbio.com to get one.
2. Speech center CSR endpoint - us.speechcenter.verbio.com

## Step by step

### Docker
You can pull a docker image from our public repository at Docker Hub, or alternatively, create a docker image and run transcriptions from the respective container. 
You need to have docker installed in your system ([Docker installation](https://docs.docker.com/engine/install/)).
#### Pull docker image
Pull docker image from Docker Hub with:
```shell
docker pull verbio/speechcenter-stt-streaming-cpp-client:0.1.0
```
#### Or create docker image
Alternatively, you can build the docker image. From the root of the project run :
```shell
docker build -t asr-cpp-client:1.1.0 .
```

#### Run container
Create a directory and add any wav audio files in it. 
Also add the token file, this token file can be a blank file (if the client credentials issued by the dashboard will be used) or a valid token. 
This directory can be placed anywhere on the computer.
Run:
```shell
docker run --rm -v <audio-and-token-dir>:/asr-client -it asr-cpp-client:1.1.0 /bin/bash
```
The audios and token directory mounts to /asr-client directory inside the container, 
and you can run transcriptions from the container with the:
```text
cli_client
```
command like:
```shell
cli_client -a audiofile.wav -T GENERIC -t my.token -l en-US --asr-version V1  -H us.speechcenter.verbio.com -s 16000 --client-id my-client-id --client-secret my-client-secret
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

Use:
```text
cli_client --help
```
for more options.

### Build from source
In order to compile the source code you will need g++>=7 and cmake-3.>= 20
#### Install Conan dependency manager

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


#### Install conan dependencies

It is recommended to create a `/build` subdirectory in the project root, and run all the following commands from there:

They are already writen in the `conanfile.txt` file in this repository.
The grp and protobuf packages are necessary to automatically generate from the .proto specification all the necessary code that the main code will use to connect with the gRPC server in the cloud.

To install, (from `/build`) run:
```shell
conan install ..
```

This will also set up the necessary files for CMake build inside the `/build` folder.

#### Generate the gRPC code for C++

There is already a step in the CMake configuration of this project that will generate the C++ code for gRPC. You do not have to worry about it. 

#### Configuration
From `/build`:
```shell
cmake ..
```

This will generate all the configuration files needed and will create all the necessary C++ from the .proto file that will allow your code to communicate with the Speech Center platform.

#### Compile

You can use cmake to compile the code.

From `/build`:
```shell
cmake --build . --target all 
```
Once the project is compiled you can check that everything went as expected by executing the unit tests:
```shell
ctest
```
#### Run the client
The cli_client will be using the generated C++ code to connect to the Speech Center cloud to process you speech file. If you have run the commands from a <project-root>/build directory then the executable is located at <project-root>/build/src directory.

#### 

**Example**

From `/build/src`:
```shell
./cli_client -a audiofile.wav -T generic -t my.token -l en-US --asr-version V1  -H us.speechcenter.verbio.com -s 16000 --client-id my-client-id --client-secret my-client-secret
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

## Speech Center integration information

### Speech Center important information

Speech Center website: https://speechcenter.verbio.com

Speech Center dashboard: https://dashboard.speechcenter.verbio.com

Speech Center authorization service endpoint: https://auth.speechcenter.verbio.com:444/api/v1/token

### Dashboard 

Speech Center features a dashboard at https://dashboard.speechcenter.verbio.com. 
Dashboard serves two main purposes:

1. Allow users to follow up historical usage data of the service, separated by projects or as a whole.
2. Allow users to retrieve client_id and client_secret credentials necessary to integrate with Speech Center, please look at authentication section to learn more about how this is performed. 


### Customer Credentials

All speech requests sent to Speech Center must have a valid authorization token in order for the system to successfully process the request. These tokens have a validity period of 1 hour and can be generated on demand using your customer credentials.

Your customer credentials can be retrieved through the [Speech Center Dashboard](https://dashboard.speechcenter.verbio.com) by logging in with the customer account supplied to you by Verbio.

#### Authentication flow


To acquire a valid token submit an HTTP POST request to the authentication service at `https://auth.speechcenter.verbio.com:444`.

**Token expiration management**

![Authentication flow](img/OAuth_diagram.png)

As part of the JWT specification, we fully support the token expiration claim, so generated tokens will be valid for only a finite period of time of 1 hour to up to 1 day. It is the responsibility of the calling client to manage this token expiration time and, in a best case scenario, anticipate the refresh by a couple of minutes so the streaming session attempt never fails because of token expiration.

In order to refresh the token, the token refresh endpoint can be called with the same client_id and client_secret, and it will respond with a new JWT token with a renewed expiration time.


#### Authentication API

**Method**
POST

**Resource**
```
/api/v1/token
```
**Request body**

```
{
 "client_id":"your_client_id",
 "client_secret":"your_client_secret"
}
```

**Response body**

Content type: application/json
Body:
```
{
 "access_token": "new_access_token",
 "expiration_time": 1678453328
}
```
*expiration_time field contains the expiration time for the JWT token so there is no need to decode the token on the client request to know the token expiration time.

Status codes:
```
HTTP 200: OK
HTTP 400: KO - Provided request format was invalid.
HTTP 401: KO - There was a problem with the provided credentials.
HTTP 5XX: KO - There was a problem with the server.
```

#### Testing calls using curl

**Example request**
```
curl --header "Content-Type: application/json"   --request POST   --data '{"client_id":"YOUR_CLIENT_ID","client_secret":"YOUR_CLIENT_SECRET"}'   'https://auth.speechcenter.verbio.com:444/api/v1/token'
```

**Example response**

{
  "access_token": "EXAMPLE_ACCESS_TOKEN",
  "expiration_time": 1678453328
}


### Client flags


#### Audio file

Argument: 
```
-a, --audio file
```

This argument is required, stating a path to a .wav audio in 8kHz or 16kHz sampling rate and PCM16 encoding to use for the recognition.

#### Topic

```
-T, --topic arg
```

 Topic to use for the recognition when a grammar is not provided. Must be `generic`


#### Grammar

There are three options available to provide a grammar:

```
-I, --inline-grammar arg
-G, --grammar-uri arg
-C, --compiled-grammar arg
```

- The inline grammar option expects a grammar passed inline as a string.
- The grammar URI option expects a URI, either pointing to a built-in grammar or to a grammar that is being hosted externally.
- The compiled grammar expects a filename (a .tar.xz file) of the previously compiled grammar.

> **THE INLINE GRAMMAR OPTION IS NOT IMPLEMENTED YET.**

#### Language

```
-l, --language arg
```

Language to use for the recognition: `en-US`, `en-GB`, `pt-BR`, `es`, `ca-ES`, `es-419`, `tr`, `ja`, `fr`, `fr-CA`, `de`, `it` (default: `en-US`).


#### Sample rate


```
-s, --sample-rate arg
```

The sampling rate for the audio recognition: 8000, 16000. (default: 8000).

#### Token


```
-t, --token arg 
```

Path to the authentication token file. This file needs to have a valid token in order for the Speech Center to work. 

In order for the client to work, the token argument is required in the following situations:

- The client will authenticate just by using the available token file. The file provided in this argument **needs to be a valid SpeechCenter JWT token so the transcription service can work**.
- The client will authenticate by providinf their client credentials through the `--client_id` and `--client_secret` program arguments. In this case **a token file must also be supplied even if it is a blank file**. Client will check file to see if it is a valid token, if it isn't it will refresh automatically the token and fill the file with a valid token. In this case, client_id and client_secret fields are also required.
     
     
#### Client id and secret


```
--client-id arg      Client id for token refresh (default: "")
```    

``` 
--client-secret arg  Client secret for token refresh (default: "")
``` 

`client-id` and `client-secret` fields are required for automatic token refresh. The arguments need to be written inline with no quotes for each field.

#### Host


```
-H, --host arg
```

URL of the host where the request will be sent. Main endpoints will be expanded as the product is deployed in different regions. Please use **us.speechcenter.verbio.com** as the host.


#### Diarization

```
-d, --diarization
```

This option enables diarization.

> This option is oriented towards batch transcription only and its use for streaming and call automation is not recommended.


#### Formatting

```
-f, --formatting
```

This option will enable formatting on the speech transcription.

#### ASR version

```
-A, --asr-version arg
```

This will select the asr version the speech center will use for transcriptions.

> Please follow Verbio's sales department recommendation on which version to use.

#### Labels

```
-L, --label arg
```

This option allows for a one word argument to be sent so that the speech transcription request is billed as part of a particular project for the customer. The argument will be a one word name that will classify the request under that project.

- **Project name must only consist of one word.**
- **Argument must be the same each time for the same project. If there is a typo another project will be created.**
- **There is no limit on the amount of projects that can be created.**
