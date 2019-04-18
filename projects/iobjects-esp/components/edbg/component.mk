###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

COMPONENT_SRCDIRS := . dap-hal mem

COMPONENT_PRIV_INCLUDEDIRS := mem

CFLAGS += -include mem_esp32.h -Werror

$(call compile_only_if,$(CONFIG_EDBG_ENABLED),dap-hal/dap.o dap-hal/utils.o)

###############################################################################