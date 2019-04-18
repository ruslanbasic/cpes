###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

CFLAGS += -ffunction-sections 
CFLAGS += -fdata-sections

COMPONENT_SRCDIRS := device # socket/hw-v0.0.5

COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive -l$(COMPONENT_NAME) -Wl,--no-whole-archive
COMPONENT_ADD_LDFLAGS += -Wl,--wrap=device_control_super

###############################################################################
