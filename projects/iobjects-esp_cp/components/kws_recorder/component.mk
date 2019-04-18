###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

COMPONENT_PRIV_INCLUDEDIRS := . mem
COMPONENT_SRCDIRS := recorder mem

recorder/recorder.o: CFLAGS += -Wno-maybe-uninitialized

CFLAGS += -include mem_esp32.h

###############################################################################