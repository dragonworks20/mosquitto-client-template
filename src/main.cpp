#include <iostream>
#include "../3rdparty/mosquitto/include/mosquitto.h"
#include "../3rdparty/mosquitto/include/mqtt_protocol.h"

int main(int, char **)
{
    std::cout << "Hello, from vscode_mosquitto!\n";

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq)
    {
        std::cerr << "Failed to create Mosquitto client instance" << std::endl;
        return 1;
    }
    else{
        std::cout<<"success init"<<std::endl;
    }
}
