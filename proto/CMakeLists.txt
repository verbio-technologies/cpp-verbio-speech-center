#
# Protobuf/Grpc source files
#
set(PROTO_FILES
    recognition_streaming_request.proto
    recognition_streaming_response.proto
    recognition.proto
    )

#
# Add Library target with protobuf sources
#
add_library(speech-center-grpc ${PROTO_FILES})
target_link_libraries(speech-center-grpc
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(speech-center-grpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in speech-center-grpc target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET speech-center-grpc LANGUAGE cpp)
protobuf_generate(TARGET speech-center-grpc LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
