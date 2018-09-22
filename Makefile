#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := simple_wifi

###################################################
PRJDIR    ?= $(PWD)
TOOLCHAIN ?= $(HOME)/programme/toolchains/xtensa-lx106-elf-4.8.5
IDF_PATH  ?= $(HOME)/projects/esp/ESP8266_RTOS_SDK
BIN_PATH  ?= $(PRJDIR)/out

###################################################
PATH  := $(TOOLCHAIN)/bin:$(PATH)

CFLAGS += -I$(PRJDIR)/components/mqtt/

include $(IDF_PATH)/make/project.mk

