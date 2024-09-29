#include <iostream>
#include "../3rdparty/mosquitto/include/mosquitto.h"
struct MosquittoVersion {
  int maj;
  int min;
  int rev;
  void Print() {
    std::cout << "mosquitto ver : " << maj << "." << min << "." << rev
              << std::endl;
  }
};

struct BrokerInfo {
  std::string host;
  int port;
};
struct TopicInfo{
  int mid;
  int qos;
  std::string topic;
  std::string message;
  const mosquitto_property *props;
};
struct MosquittoOptions{
  int sub_options;
};
enum class MqttConnectionStatus : uint8_t {
  DISCONNECTED = 0,
  CONNECTING = 1,
  CONNECTED = 2,
  RECONNECTING = 3
};
constexpr const char *
MqttConnectionStatusStrting(MqttConnectionStatus conn_status) {
  switch (conn_status) {
  case MqttConnectionStatus::DISCONNECTED:
    return "DISCONNECTED";
  case MqttConnectionStatus::CONNECTING:
    return "CONNECTING";
  case MqttConnectionStatus::CONNECTED:
    return "CONNECTED";
  case MqttConnectionStatus::RECONNECTING:
    return "RECONNECTING";
  default:
    return "UNKNOWN";
  }
}