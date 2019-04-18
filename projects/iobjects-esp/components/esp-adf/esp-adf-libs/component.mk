#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS :=    esp_audio/include \
                                esp_codec/include/codec \
                                esp_codec/include/processing

COMPONENT_SRCDIRS := . esp_codec

LIBS := esp_codec esp_audio esp-amr esp-amrwbenc esp-aac esp-ogg-container esp-opus esp-tremor esp-flac

COMPONENT_ADD_LDFLAGS +=  -L$(COMPONENT_PATH)/esp_audio/lib \
                          -L$(COMPONENT_PATH)/esp_codec/lib \
                           $(addprefix -l,$(LIBS)) \

ALL_LIB_FILES += $(patsubst %,$(COMPONENT_PATH)/%/lib/lib%.a,$(LIBS))