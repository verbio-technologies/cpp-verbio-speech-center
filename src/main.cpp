
#include "Configuration.h"
#include "SpeechCenterClient.h"


int main(int argc, char* argv[]) {
    Configuration configuration(argc, argv);
    SpeechCenterClient client;
    client.connect(configuration);
    client.process(configuration.getAudioPath());
}