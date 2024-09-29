#pragma once
#include "../3rdparty/mosquitto/include/mosquitto.h"
#include "../3rdparty/mosquitto/include/mqtt_protocol.h"
#include "types.hpp"
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

/// TODO LISTS
/*
1. publish topic according to publish's timeout, retry
 */
class MosquittoClient {
public:
  MosquittoClient(BrokerInfo);
  virtual ~MosquittoClient();

  MosquittoClient() = delete;
  MosquittoClient(const MosquittoClient &) = delete;
  MosquittoClient &operator=(const MosquittoClient &) = delete;
  MosquittoClient(MosquittoClient &&) = delete;
  MosquittoClient &operator=(MosquittoClient &&) = delete;
  int Init();
  void Connect();
  void Disconnect();
  void PublishV5(const std::string &topic, const std::string &message, const mosquitto_property *props);
  void SubscribeV5(const std::string &topic, const int qos);

protected:
  void SetConnectionStatus(MqttConnectionStatus status);
  MqttConnectionStatus GetConnectionStatus();
  void StartMosquittoLoop();
  void Reconnect();
  void RemoveTopicFromPubQueue(int mid);
  void MosquittoClient::StopMosquittoLoop();
  void PrintPubLists();
  void PrintSubLists();
  static void ConnectV5Callback(struct mosquitto *mosq, void *obj, int rc,
                                int flags, const mosquitto_property *props);
  static void MessageV5Callback(struct mosquitto *mosq, void *obj,
                                const struct mosquitto_message *msg,
                                const mosquitto_property *props);
  static void DisconnectV5Callback(mosquitto *mosq, void *obj, int rc,
                                   const mosquitto_property *props);
  static void PublishV5Callback(mosquitto *mosq, void *obj, int mid, int rc,
                                const mosquitto_property *props);
  static void SubscribeV5Callback(mosquitto *mosq, void *obj, int mid,
                                  int qos_count, const int *granted_qos,
                                  const mosquitto_property *props);

private:
  std::list<TopicInfo> publish_lists_;
  std::list<TopicInfo> subscribe_lists_;
  std::thread mosq_loop_thread;
  std::mutex list_mutex;
  mosquitto *mosquitto_ = nullptr;
  MosquittoVersion mosq_ver_;
  BrokerInfo broker_;
  MosquittoOptions mosq_options_;
  uint32_t retry_count_ = 0;
  MqttConnectionStatus connection_status_ = MqttConnectionStatus::DISCONNECTED;

};