###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive -l$(COMPONENT_NAME) -Wl,--no-whole-archive

###############################################################################
