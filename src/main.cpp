#include <iostream>
#include "../3rdparty/mosquitto/include/mosquitto.h"
#include "../3rdparty/mosquitto/include/mqtt_protocol.h"

int main(int, char **)
{
    std::cout<<"Hello Mosquitto!!"<<std::endl;
    // Initialize the Mosquitto library
    int rc = mosquitto_lib_init();
    if(rc != MOSQ_ERR_SUCCESS){
        std::cout<<"mosquitto init fail..."<<std::endl;
        return EXIT_FAILURE;    
    }

    // Create a new Mosquitto client instance
    mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq)
    {
        std::cout << "Failed to create Mosquitto client instance" << std::endl;
        return EXIT_FAILURE;
    }
    else{
        std::cout<<"success init"<<std::endl;
    }
    // set mqtt version
    mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);

    int ver_maj, ver_min, ver_rev;
    int mos_ver = mosquitto_lib_version(&ver_maj, &ver_min, &ver_rev);
    std::cout<<"mosquitto ver : "<<ver_maj<<"."<<ver_min<<"."<<ver_rev<<std::endl;
    std::cout<<"mos_ver : "<<mos_ver<<std::endl;

    // connect to server
    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: Could not connect to broker (%s)\n", mosquitto_strerror(rc));
        return EXIT_FAILURE;
    }

    std::string topic = "test/topic";
    int mid = 1;
    int qos = 1;
    bool retain = false;
    std::string payload = "hello mosquitto!!";
    rc = mosquitto_publish_v5(mosq, &mid, topic.c_str(), payload.length(), payload.c_str(), qos, retain, nullptr);
    std::cout<<"publish rc : "<<rc<<std::endl;

    // disconnect server
    mosquitto_disconnect(mosq);
    // destroy mosquitto instance
    mosquitto_destroy(mosq);
    // clean resources of libs
    mosquitto_lib_cleanup();
    return 0;

}
