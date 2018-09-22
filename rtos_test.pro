TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

###########################################################
IDF_PATH = $$(HOME)/projects/esp/ESP8266_RTOS_SDK

###########################################################
DEFINES += ESP_PLATFORM
DEFINES += WOLFSSL_USER_SETTINGS

# FreeRTOS settings for better auto completion
DEFINES += configSTACK_DEPTH_TYPE=uint16_t
DEFINES += configSUPPORT_DYNAMIC_ALLOCATION=1
DEFINES += configUSE_NEWLIB_REENTRANT=0
DEFINES += configUSE_PREEMPTION=1
DEFINES += configUSE_IDLE_HOOK=1
DEFINES += configUSE_TICK_HOOK=0
DEFINES += configUSE_TICKLESS_IDLE=1
DEFINES += configUSE_TRACE_FACILITY=0
DEFINES += configUSE_STATS_FORMATTING_FUNCTIONS=0
DEFINES += configUSE_16_BIT_TICKS=0
DEFINES += configIDLE_SHOULD_YIELD=1
DEFINES += INCLUDE_xTaskGetIdleTaskHandle=1
DEFINES += INCLUDE_xTimerGetTimerDaemonTaskHandle=1
DEFINES += configCHECK_FOR_STACK_OVERFLOW=2
DEFINES += configUSE_MUTEXES=1
DEFINES += configUSE_RECURSIVE_MUTEXES=1
DEFINES += configUSE_COUNTING_SEMAPHORES=1
DEFINES += configUSE_TIMERS=1
DEFINES += CONFIG_NEWLIB_LIBRARY_LEVEL_NORMAL=1

# LWIP
DEFINES += LWIP_COMPAT_SOCKETS
DEFINES += LWIP_DNS
DEFINES += LWIP_SOCKET
DEFINES += LWIP_DHCP=1
DEFINES += MEM_LIBC_MALLOC=1
DEFINES += MEMP_OVERFLOW_CHECK=1

###########################################################
INCLUDEPATH += main/include \
               build/include \
               components/mqtt \
               components/mqtt/lwip/apps


###########################################################
INCLUDEPATH += $${IDF_PATH}/components/newlib/newlib/include \
               $${IDF_PATH}/components/esp8266/include \
               $${IDF_PATH}/components/cjson/cJSON \
               $${IDF_PATH}/components/espos/include \
               $${IDF_PATH}/components/freertos/include \
               $${IDF_PATH}/components/freertos/port/esp8266/include \
               $${IDF_PATH}/components/freertos/include/freertos/private \
               $${IDF_PATH}/components/log/include \
               $${IDF_PATH}/components/lwip/include \
               $${IDF_PATH}/components/lwip/lwip/src/include \
               $${IDF_PATH}/components/lwip/lwip/src/include/posix \
               $${IDF_PATH}/components/nvs_flash/include \
               $${IDF_PATH}/components/mqtt/paho/MQTTClient-C/src \
               $${IDF_PATH}/components/mqtt/paho/MQTTClient-C/src/FreeRTOS \
               $${IDF_PATH}/components/spiffs/include \
               $${IDF_PATH}/components/tcpip_adapter/include \
               $${IDF_PATH}/components/heap/include

###########################################################
INCLUDEPATH += $$(HOME)/projects/iot/libraries/secretdata


###########################################################
SOURCES += \ 
    components/mqtt/mqtt.c \
    main/mqtt_client.c \
    main/main.c

DISTFILES += \
    Makefile \
    main/component.mk \
    sdkconfig \
    main/Kconfig.projbuild

HEADERS += \ 
    components/mqtt/lwip/apps/iana.h \
    components/mqtt/lwip/apps/mqtt.h \
    components/mqtt/lwip/apps/mqtt_opts.h \
    components/mqtt/lwip/apps/mqtt_priv.h \
    components/mqtt/lwip/altcp.h \
    components/mqtt/lwip/altcp_tcp.h \
    components/mqtt/lwip/altcp_tls.h \
    main/include/mqtt_client.h
