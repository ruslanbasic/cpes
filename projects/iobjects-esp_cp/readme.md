# ESP32 Intelligent Object

## esp-idf 3.1 (New)
  * git clone --recursive -b release/v3.1 https://github.com/espressif/esp-idf.git

## Components:
  * github.com/espressif/esp-idf - `bae9709a7950e2ee08e14c65be27831bcb547105` `11.04.2018`
  * github.com/warmcat/libwebsockets - `19627b812e97c2aedee61a64b0c69a89e2199a8e` `13.04.2018`
  * github.com/lws-team/mbedtls - `4efb69fc0c5c7a007395d55044fafb86a97ec4ed` `03.02.2018`

You can force component to that commit by cloning / pulling / fetching
the latest version and then doing `git reset --hard bae9709a7950e2ee08e14c65be27831bcb547105`
in the component directory.

## Configure automatic style checking on commit

  * cd .git/hooks
  * nano pre-commit

      #!/bin/sh
      tools/checkstyle/check_all.sh

  * chmod +x pre-commit

## Unit Testing Example

Add partition "flash_test, data, fat, , 528K" into partitions.csv then `make test TEST_COMPONENTS=lws`

## Development tools:
  * OS        - Ubuntu v17.10, Kernel v.4.13.0-16-generic
  * IDE       - Eclipse IDE for C/C++ Developers v4.7.0 (Oxygen)
  * Buildtool - GNU Make v4.1
  * Toolchain - xtensa esp32 elf linux64 v1.22.0-73-ge28a011-5.2.0

## License

Copyright (c) A1 Company LLC. All rights reserved.