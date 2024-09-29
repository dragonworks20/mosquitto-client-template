#include "mosquittoclient.hpp"
#include "utils.hpp"
#include <cassert>
#include <iostream>
#include <thread>
int main(int, char **) {
  std::cout << "Hello Mosquitto!!" << std::endl;

  BrokerInfo broker{"localhost", 1883};
  MosquittoClient client(broker);
  std::thread client_thread([&client]() {
    client.Init();
    client.Connect();
  });

  while (1) {
    std::cout << "[1]publish [2]subscribe [3]exit" << std::endl;
    int input = 0;
    std::cin >> input;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if(std::cin.fail()){
        std::cout<<"cin fail..."<<std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
    }
    try {
      if (input == 1) {
        std::string topic = "";
        std::string message = "";
        mosquitto_property *props = nullptr;
        std::cout << "topic:";
        // std::getline(std::cin, topic);
        topic = "test/topic";
        std::cout << "recv :" << topic << std::endl;
        std::cout << "message:";
        std::getline(std::cin, message);
        std::cout << "recv :" << message << std::endl;
        assert(!topic.empty());
        assert(!message.empty());
        client.PublishV5(topic, message, props);
      } else if (input == 2) {
        std::cout << "please input topic for subscribe" << std::endl;
        std::string topic;
        int qos;
        topic = "test/topic";
        qos = 1;
        std::cout<<"topic : ";
        // std::getline(std::cin, topic);
        std::cout<<"recv : "<<topic<<std::endl;
        // std::cin>>qos;
        std::cout<<"qos : ";
        std::cout<<qos<<std::endl;
        client.SubscribeV5(topic, qos);
      } else if (input == 3) {
        std::cout << "client exit" << std::endl;
        break;
      } else {
        std::cout << "input error" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
    } catch (const std::exception &e) {
      std::cout << e.what() << std::endl;
    }
  }
  client.Disconnect();
  client_thread.join();
  std::cout << "GoodBye!" << std::endl;
  return 0;
}
