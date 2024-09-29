#include "MosquittoClient.hpp"
#include <cassert>
#include <chrono>
#include <thread>
MosquittoClient::MosquittoClient(BrokerInfo broker) : broker_(broker) {
  std::cout << "MosquittoClient::MosquittoClient()" << std::endl;
}
MosquittoClient::~MosquittoClient() {
  std::cout << "MosquittoClient::~MosquittoClient()" << std::endl;
  StopMosquittoLoop();
}

int MosquittoClient::Init() {

  mosq_options_.sub_options = 0;
  // Initialize the Mosquitto library
  int rc = mosquitto_lib_init();
  if (rc != MOSQ_ERR_SUCCESS) {
    std::cout << "mosquitto init fail..." << std::endl;
    return -1;
  }

  // Create a new Mosquitto client instance
  mosquitto_ = mosquitto_new(NULL, true, this);
  if (!mosquitto_) {
    std::cout << "Failed to create Mosquitto client instance" << std::endl;
    return -1;
  }

  // set mqtt version
  mosquitto_int_option(mosquitto_, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
  mosquitto_connect_v5_callback_set(mosquitto_, ConnectV5Callback);
  mosquitto_disconnect_v5_callback_set(mosquitto_, DisconnectV5Callback);
  mosquitto_publish_v5_callback_set(mosquitto_, PublishV5Callback);
  mosquitto_subscribe_v5_callback_set(mosquitto_, SubscribeV5Callback);
  mosquitto_message_v5_callback_set(mosquitto_, MessageV5Callback);

  int mos_ver =
      mosquitto_lib_version(&mosq_ver_.maj, &mosq_ver_.min, &mosq_ver_.rev);
  mosq_ver_.Print();
  std::cout << "mosquittolib : " << mos_ver << std::endl;

  return 0;
}

void MosquittoClient::Connect() {
  int rc = 0;
  // connect to server
  rc = mosquitto_connect(mosquitto_, broker_.host.c_str(), broker_.port, 60);
  if (rc != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "Error: Could not connect to broker (%s)\n",
            mosquitto_strerror(rc));
  } else {
    SetConnectionStatus(MqttConnectionStatus::CONNECTING);
    StartMosquittoLoop();
  }
}
void MosquittoClient::Disconnect() {
  int reason_code = 0;
  int rc = mosquitto_disconnect_v5(mosquitto_, MQTT_RC_NORMAL_DISCONNECTION,
                                   nullptr);
  std::cout << "disconnect rc : " << rc << std::endl;
  std::cout << mosquitto_strerror(rc) << std::endl;
  retry_count_ = 0;
}
void MosquittoClient::SubscribeV5(const std::string &topic, const int qos) {

  std::lock_guard<std::mutex> lock(list_mutex);
  static int sub_mid = 0;
  int rc = 0;
  sub_mid++;
  TopicInfo topicinfo{sub_mid, qos, topic, ""};
  mosquitto_property *props = nullptr;
  subscribe_lists_.push_back(topicinfo);
  rc = mosquitto_subscribe_v5(mosquitto_, &sub_mid, topicinfo.topic.c_str(),
                              topicinfo.qos, mosq_options_.sub_options, props);
  std::cout << "subscribe rc : " << rc << ", mid:" << sub_mid << std::endl;
  if (rc != MOSQ_ERR_SUCCESS) {
    std::cout << "sub fail... " << mosquitto_strerror(rc) << std::endl;
  }
}
void MosquittoClient::PublishV5(const std::string &topic,
                                const std::string &message,
                                const mosquitto_property *props) {
  int rc = 0;
  static int pub_mid = 0;
  int qos = 1;
  bool retain = false;
  std::lock_guard<std::mutex> lock(list_mutex);
  pub_mid++;

  TopicInfo topicinfo{pub_mid, qos, topic, message, props};

  publish_lists_.push_back(topicinfo);
  rc = mosquitto_publish_v5(mosquitto_, &pub_mid, topicinfo.topic.c_str(),
                            static_cast<int>(topicinfo.message.length()),
                            topicinfo.message.c_str(), qos, retain, props);
  std::cout << "publish rc : " << rc << " mid:" << pub_mid << std::endl;
  if (rc != MOSQ_ERR_SUCCESS) {
    std::cout << "publish fail... ";
    std::cout << mosquitto_strerror(rc) << std::endl;
  }
}
void MosquittoClient::RemoveTopicFromPubQueue(int mid) {
  std::lock_guard<std::mutex> lock(list_mutex);
  publish_lists_.remove_if(
      [mid](const TopicInfo &topic) { return topic.mid == mid; });
  PrintPubLists();
}

void MosquittoClient::PrintPubLists() {
  std::cout << "PUBLIST SIZE : " << publish_lists_.size() << std::endl;
  for (auto topic : publish_lists_) {
    std::cout << "PUBLIST mid:" << topic.mid << std::endl;
  }
}
void MosquittoClient::PrintSubLists() {
  std::cout << "SUBSCRIBE SIZE : " << subscribe_lists_.size() << std::endl;
  for (auto topic : publish_lists_) {
    std::cout << "SUBSCIRBE topic:" << topic.topic << ", qos:" << topic.qos
              << std::endl;
  }
}
void MosquittoClient::SetConnectionStatus(MqttConnectionStatus status) {
  connection_status_ = status;
  std::cout << "connection status : "
            << MqttConnectionStatusStrting(connection_status_) << std::endl;
}
MqttConnectionStatus MosquittoClient::GetConnectionStatus() {
  return connection_status_;
}
void MosquittoClient::ConnectV5Callback(struct mosquitto *mosq, void *obj,
                                        int rc, int flags,
                                        const mosquitto_property *props) {
  assert(obj);
  std::cout << "ConnectV5Callback" << std::endl;
  if (obj) {
    MosquittoClient *userdata = static_cast<MosquittoClient *>(obj);
    userdata->SetConnectionStatus(MqttConnectionStatus::CONNECTED);
    userdata->retry_count_ = 0;
  }
}
void MosquittoClient::DisconnectV5Callback(mosquitto *mosq, void *obj, int rc,
                                           const mosquitto_property *props) {
  assert(obj);
  std::cout << "DisconnectV5Callback" << std::endl;
  if (obj) {
    MosquittoClient *userdata = static_cast<MosquittoClient *>(obj);
    userdata->SetConnectionStatus(MqttConnectionStatus::DISCONNECTED);
    // userdata->Disconnect();
    userdata->Reconnect();
  }
}
void MosquittoClient::MessageV5Callback(struct mosquitto *mosq, void *obj,
                                        const struct mosquitto_message *msg,
                                        const mosquitto_property *props) {
  assert(obj);
  std::cout << "Received message: " << static_cast<char *>(msg->payload)
            << " from topic: " << msg->topic << std::endl;
}

void MosquittoClient::PublishV5Callback(mosquitto *mosq, void *obj, int mid,
                                        int rc,
                                        const mosquitto_property *props) {
  assert(obj);
  if (obj) {
    MosquittoClient *userdata = static_cast<MosquittoClient *>(obj);
    std::cout << "pub callback mid:" << mid << ", rc:" << rc << std::endl;
    userdata->RemoveTopicFromPubQueue(mid);
  }
}
void MosquittoClient::SubscribeV5Callback(mosquitto *mosq, void *obj, int mid,
                                          int qos_count, const int *granted_qos,
                                          const mosquitto_property *props) {
  assert(obj);
  if (obj) {
    MosquittoClient *userdata = static_cast<MosquittoClient *>(obj);
    std::cout << "sub callback mid:" << mid << ", qos_count:" << qos_count
              << std::endl;
  }
}
void MosquittoClient::Reconnect() {
  ++retry_count_;
  std::cout << "try reconnect to the broker... retry count : " << retry_count_
            << std::endl;
  mosquitto_reconnect_async(mosquitto_);
}
void MosquittoClient::StartMosquittoLoop() {

  mosq_loop_thread = std::thread([this]() {
    int rc = mosquitto_loop_forever(mosquitto_, 0, 1);
    std::cout << "start mosquitto loop rc:" << rc << std::endl;
  });
}

void MosquittoClient::StopMosquittoLoop() {
  // disconnect server
  int rc = mosquitto_disconnect_v5(mosquitto_, MQTT_RC_NORMAL_DISCONNECTION,
                                   nullptr);
  std::cout << "StopMosquittoLoop disconnect rc : " << rc << " ";
  std::cout << mosquitto_strerror(rc) << std::endl;

  mosquitto_destroy(mosquitto_);
  // clean resources of libs
  mosquitto_lib_cleanup();
  if (mosq_loop_thread.joinable())
    mosq_loop_thread.join();
  std::cout << "StopMosquittoLoop end" << std::endl;
}