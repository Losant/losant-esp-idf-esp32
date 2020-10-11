#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
# This file is modeled after https://github.com/espressif/esp-idf/blob/master/examples/protocols/mqtt/tcp/Makefile
# with updated values for this project.
#
PROJECT_NAME := getting_started_with_esp32

EXTRA_COMPONENT_DIRS = $(IDF_PATH)/examples/common_components/protocol_examples_common

include $(IDF_PATH)/make/project.mk
