###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

PROJECT_NAME := esp32-smart-device
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/components/esp-adf

include $(IDF_PATH)/make/project.mk
include sdkconfig

# 0 = OTA Application, 1 = Factory (Recovery) Application
LWS_IS_FACTORY_APPLICATION=0
export LWS_IS_FACTORY_APPLICATION
export A
export F

include ${PWD}/components/libwebsockets/scripts/esp32.mk

CFLAGS+= -I$(PROJECT_PATH)/components/libwebsockets/plugins \
         -I$(PROJECT_PATH)/components/libwebsockets/lib \
         -DLWS_IS_FACTORY_APPLICATION=$(LWS_IS_FACTORY_APPLICATION) \

test:
	$(MAKE) CFLAGS+=-DUNIT_TEST --makefile=unittest.mk flash monitor

###############################################################################
