#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/queue.h>

#include <esp_system.h>
#include <esp_log.h>
#include <lwip/dns.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "mqtt_client.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"

#define QOS            1U /* 0 1 or 2, see MQTT specification */
#define DO_NOT_RETAIN  0U /* No don't retain such crappy payload... */

static const char MQTT_TAG[] = "MQTTCLIENT";
static const char MQTT_CLIENT_ID[] = "loadclient";
static const char MQTT_SERVER_NAME[] = CONFIG_MQTT_SERVER;

typedef struct {
    struct {
        uint8_t       isConnected:1;
        uint8_t       hasConnected:1;  // was first connect done
        uint8_t       mustReconnect:1; // wifi has lost connection
    };
    SemaphoreHandle_t   mutexDnsResolve;
    ip_addr_t           ipOfMqttServer;
    mqtt_client_t       client;
} mqtt_t;
static mqtt_t  mqtt;

/**
 * @brief mqttIncomingPublishCallback
 * @param data
 * @param topic
 * @param tot_len
 */
static void mqttIncomingPublishCallback(mqtt_t *data, const char *topic, u32_t tot_len)
{
    ESP_LOGD(MQTT_TAG, "Incoming mqtt message on topic '%s' (len: %d)", topic, tot_len);
}

/**
 * @brief mqttIncomingDataCallback
 * @param dataModel
 * @param data
 * @param len
 * @param flags
 */
static void mqttIncomingDataCallback(mqtt_t *unused, const u8_t *data, u16_t len, u8_t flags)
{
    ESP_LOGD(MQTT_TAG, "Incoming mqtt message flag: %d", flags);

    if (flags & MQTT_DATA_FLAG_LAST) {
        char *payload = calloc(len + 2, sizeof(char));
        snprintf(payload, len + 1, (const char*)data);
        ESP_LOGD(MQTT_TAG, "Got mqtt message %s", payload);
        free(payload);
    }
}

static void mqttConnectCallback(mqtt_client_t *client, mqtt_t *data, mqtt_connection_status_t status)
{
    switch (status) {
        case MQTT_CONNECT_ACCEPTED:
            data->hasConnected = 1;
            data->isConnected = 1;
            mqtt_set_inpub_callback(client, (mqtt_incoming_publish_cb_t)mqttIncomingPublishCallback, (mqtt_incoming_data_cb_t)mqttIncomingDataCallback, data);
            break;
        case MQTT_CONNECT_DISCONNECTED:
        default:
            ESP_LOGW(MQTT_TAG, "mqtt connect failed or broken.");
            data->isConnected = 0;
            break;
    }
}

static void dnsFoundCallback(const char *name, const ip_addr_t *ipaddr, mqtt_t *data)
{
    ESP_LOGD(MQTT_TAG, "DNS callback ...");
    if (ip_addr_isany(ipaddr)) {
        ESP_LOGE(MQTT_TAG, "Host %s not found.", name);
        ip_addr_set_any(0, &(mqtt.ipOfMqttServer));
    }
    else {
        ESP_LOGD(MQTT_TAG, "DNS lookup for %s => %s", name, ipaddr_ntoa(ipaddr));
        ip_addr_copy(mqtt.ipOfMqttServer, *ipaddr);
    }
    xSemaphoreGive(mqtt.mutexDnsResolve);
}

void mqttDisconnect()
{
    ESP_LOGD(MQTT_TAG, "Disconnecting from MQTT[conn:%d]", mqtt.isConnected);
    if (mqtt.isConnected) {
        mqtt_disconnect(&(mqtt.client));
        mqtt.hasConnected = 0;
        mqtt.isConnected = 0;
    }
}

/**
 * 
 */
err_t mqttConnect()
{
    struct mqtt_connect_client_info_t ci;
    ip_addr_t resolved;
    err_t err;

    ESP_LOGD(MQTT_TAG, "connecting to MQTT server");
    
    if (mqtt.hasConnected) {
        mqttDisconnect();
    }

    ESP_LOGD(MQTT_TAG, "MQTT resolve hostname %s", MQTT_SERVER_NAME);

    ip_addr_set_any(0, &mqtt.ipOfMqttServer);
    err = dns_gethostbyname(MQTT_SERVER_NAME, &resolved, (dns_found_callback)dnsFoundCallback, &mqtt);
    switch (err) {
        case ERR_OK:
            ip_addr_copy(mqtt.ipOfMqttServer, resolved);
            break;
        case ERR_INPROGRESS:
            if (xSemaphoreTake(mqtt.mutexDnsResolve, 10 * 1000 / portTICK_RATE_MS) != pdPASS) {
                ESP_LOGE(MQTT_TAG, "Timeout while resolving mqtt server %s", MQTT_SERVER_NAME);
                return ESP_FAIL;
            }
            if (ip_addr_isany_val(mqtt.ipOfMqttServer)) {
                ESP_LOGE(MQTT_TAG, "Failed to resolve IP address of %s", MQTT_SERVER_NAME);
                return ESP_FAIL;
            }
            break;
        default:
            ESP_LOGE(MQTT_TAG, "Failed to resolve mqtt server %s. Error %d", MQTT_SERVER_NAME, err);
            return ESP_FAIL;
    }
    
    ESP_LOGD(MQTT_TAG, "MQTT connecting to %s", ipaddr_ntoa(&(mqtt.ipOfMqttServer)));
    
    /* Setup an empty client info structure */
    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_CLIENT_ID;
    ci.keep_alive = 90;
    err = mqtt_client_connect(&(mqtt.client), &(mqtt.ipOfMqttServer), MQTT_PORT, (mqtt_connection_cb_t)mqttConnectCallback, &mqtt, &ci);
    if (err != ERR_OK) {
        ESP_LOGE(MQTT_TAG, "mqtt_client_connect failed with %d", err);
    }
    
    return err;
}

static void mqttPublishCallback(mqtt_t *data, err_t err)
{
    ESP_LOGD(MQTT_TAG, "MQTT publish state is %d", err);
}

/**
 * @brief mqttSendMessage
 * @param data
 * @param channel
 * @param message
 * @return 
 */
err_t mqttSendMessage(const char* channel, const char* message)
{
    err_t err;

    if (mqtt.isConnected) {
        ESP_LOGD(MQTT_TAG, "MQTT sending on %s message: %s", channel, message);
        err = mqtt_publish(&(mqtt.client), channel, message, strlen(message), QOS, DO_NOT_RETAIN, (mqtt_request_cb_t)mqttPublishCallback, &mqtt);
        if (err != ERR_OK) {
            ESP_LOGE(MQTT_TAG, "MQTT publish error: %d", err);
        }
    }
    else
        err = ERR_CONN;
    
    return err;
}

/**
 * @brief mqttInit
 * @param data
 */
err_t mqttInit()
{
    ESP_LOGD(MQTT_TAG, "init MQTT.");
    
    memset(&mqtt, 0x00, sizeof(mqtt));
    
    mqtt.mutexDnsResolve = xSemaphoreCreateMutex();
    xSemaphoreTake(mqtt.mutexDnsResolve, 0);
    
    return ERR_OK;
}
