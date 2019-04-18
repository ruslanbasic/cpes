###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

COMPONENT_DEPENDS += libwebsockets

HEATER := heater/hw-v0.0.5 \
          heater/hw-v0.0.6 \
          heater/hw-v0.0.7 \
          heater/hw-v0.0.8 \
          heater/hw-v0.0.9

LAMP := lamp/hw-v0.0.1 \
        lamp/hw-v0.0.2 \
        lamp/hw-v0.0.4 \
        lamp/hw-v0.0.5

SOCKET := socket/hw-v0.0.2 \
          socket/hw-v0.0.4 \
          socket/hw-v0.0.5

WATER_SENSOR := water_sensor/hw-v0.0.2

WATER_TAP := water_tap/hw-v0.0.2

COMPONENT_ADD_INCLUDEDIRS := errors \
                             $(HEATER) \
                             $(LAMP) \
                             $(SOCKET) \
                             $(WATER_SENSOR) \
                             $(WATER_TAP)

COMPONENT_SRCDIRS := $(COMPONENT_ADD_INCLUDEDIRS)

COMPONENT_EMBED_FILES := socket/hw-v0.0.5/atmel.usb.bin

###############################################################################