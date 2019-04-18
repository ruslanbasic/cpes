###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################

COMPONENT_DEPENDS += audio_hal \
                     audio_pipeline \
                     audio_sal \
                     bme280 \
                     button \
                     encoder \
                     kws_recorder \
                     libwebsockets \
                     lmt01 \
                     mlx90614 \
                     ota \
                     rgbled \
                     stpm3x

COMPONENT_ADD_INCLUDEDIRS := . \
                             autoupdate \
                             cmdline \
                             cmdline/commands \
                             device \
                             events \
                             kws \
                             logger \
                             lws \
                             nvs \
                             pubsub \
                             wifi

COMPONENT_SRCDIRS := $(COMPONENT_ADD_INCLUDEDIRS)

COMPONENT_EMBED_TXTFILES := server_root_cert.pem

###############################################################################
