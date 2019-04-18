###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

COMPONENT_DEPENDS += libwebsockets

LAMP := lamp/hw-v4 \
        lamp/hw-v7

PLUG := plug/hw-v4 \
        plug/hw-v5 \
        plug/hw-v6

HEATER := heater/hw-v8 \
          heater/hw-v9

WATER_SENSOR := water_sensor/hw-v2

WATER_TAP := water_tap/hw-v2

COMPONENT_ADD_INCLUDEDIRS := errors \
                             $(LAMP) \
                             $(PLUG) \
                             $(HEATER) \
                             $(WATER_SENSOR) \
                             $(WATER_TAP)

COMPONENT_SRCDIRS := $(COMPONENT_ADD_INCLUDEDIRS)

COMPONENT_EMBED_FILES := plug/hw-v5/atmel.usb.bin

###############################################################################