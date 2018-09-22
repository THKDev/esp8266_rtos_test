#pragma once

#include "sdkconfig.h"

#define MQTT_DOMAIN          "/test/8266/1/"
#define MQTT_TOPIC_PUBLISH   MQTT_DOMAIN "/publish"
#define MQTT_TOPIC_RECEIVE   MQTT_DOMAIN "/receive"

/**
 * @brief mqttInit
 * @return 
 */
err_t mqttInit();

/**
 * @brief mqttConnect
 * @return 
 */
err_t mqttConnect();

/**
 * @brief mqttDisconnect
 */
void mqttDisconnect();

/**
 * @brief mqttSendMessage
 * @param channel
 * @param message
 * @return 
 */
err_t mqttSendMessage(const char* channel, const char* message);
