# http://esp-idf.readthedocs.io/en/latest/api-guides/unit-tests.html

COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive -l$(COMPONENT_NAME) -Wl,--no-whole-archive