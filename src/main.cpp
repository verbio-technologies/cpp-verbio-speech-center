
#include "Configuration.h"
#include "RecognitionClient.h"
#include "gRpcExceptions.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    try {
        Configuration configuration(argc, argv);
        RecognitionClient client(configuration);
        client.connect(configuration);
    } catch (GrpcException &e) {
        ERROR(e.what());
        return -1;
    }
    return 0;
}