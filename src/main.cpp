
#include "Configuration.h"
#include "RecognitionClient.h"


int main(int argc, char* argv[]) {
    Configuration configuration(argc, argv);
    RecognitionClient client(configuration);
    client.connect(configuration);
}