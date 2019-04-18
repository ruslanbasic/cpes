# ESP32 Intelligent Object

## Get Started

### ESP32 Get Started:
```
  https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html
```
### Install necessary packages:
```
  $ sudo apt install cmake
  $ sudo apt install genromfs
```
### Create the working directory for ESP projects:
```
  $ mkdir ~/esp
```
### Setup SSH key for the project GIT:
##### Note: create ~/.ssh if need; it must be read/write for the user only
```
  $ cd ~/.ssh
  $ ssh-keygen -b 4096 -t rsa
```
##### Enter file in which to save the key (~/.ssh/id_rsa): a1company and passphrase
```
  $ ssh-add ~/.ssh/a1company
  $ cd .ssh
  $ cat a1company.pub
```
##### Copy public key and paste it into your bitbucket account
  
##### Enable auto-launcing of the ssh-agent:
    https://help.github.com/en/articles/working-with-ssh-key-passphrases
##### or start manually:
```
  $ eval `ssh-agent`
```
##### Add the private key to the agent
```
  $ ssh-add ~/.ssh/a1company
```
### Get Espressif IoT Development Framework:
```
  $ cd ~/esp
  $ git clone --recursive -b release/v3.1 https://github.com/espressif/esp-idf.git
  $ cd esp-idf
  $ git checkout 3567b1d0ce110aa17aec897fc4cbbbeda749b397
  $ git submodule init
  $ git submodule update --recursive
```
### Get IO project:
```
  $ cd ~/esp
  $ git clone git@bitbucket.org:a1company/iobjects-esp.git
```
##### Apply patches:
```
  $ cd iobjects-esp
  $ (cd $IDF_PATH && patch -p1) < patches/esp.idf.3.2.smartconfig.ack.task.stack.size.patch
  $ (cd $IDF_PATH && patch -p1) < patches/idf.3.1.bootloader.wdt.patch
  $ (cd $IDF_PATH && patch -p1) < patches/idf.3.1.bootloader.factory.reset.invert.gpio.patch
```
##### Configure automatic style checking on commit:
```
  $ cd .git/hooks
  $ nano pre-commit

  #!/bin/sh
  tools/checkstyle/check_all.sh

  $ chmod +x pre-commit
```
##### Build and Flash:
```
  $ make menuconfig
  $ make -j2
  $ make erase_flash flash monitor
```
## Setup Eclipse IDE:
```
  https://www.eclipse.org/downloads/packages/release/2018-12/r/eclipse-ide-cc-developers

  iobjects-esp/eclipse/readme.md
```

## Custom Device Build
```
  $ export SDKCONFIG_DEFAULTS=main/device/plug/hw-v5/sdkconfig.defaults
  $ make defconfig && make -j2
```
## Components:
```
  github.com/espressif/esp-idf - `bae9709a7950e2ee08e14c65be27831bcb547105` `11.04.2018`
  github.com/warmcat/libwebsockets - `19627b812e97c2aedee61a64b0c69a89e2199a8e` `13.04.2018`
  github.com/lws-team/mbedtls - `4efb69fc0c5c7a007395d55044fafb86a97ec4ed` `03.02.2018`
```
  You can force component to that commit by cloning / pulling / fetching
  the latest version and then doing `git reset --hard bae9709a7950e2ee08e14c65be27831bcb547105`
  in the component directory.
## Remote Terminal
```
  ESP32 has standart UART terminal. Intelligent Objects implement additional terminal via TCP.

  ~$ ip=`sudo arp-scan --localnet | grep 30:ae:a4:45:f3:60 | awk '{print $1}'`
  ~$ echo $ip
  ~$ nc $ip 3333

  Where 30:ae:a4:45:f3:60 is a MAC address of device to connect to.
```
## Unit Test
```
  Add partition "flash_test, data, fat, , 528K" into partitions.csv

  $ make test
  $ make test TEST_COMPONENTS=lws
```
## Development tools:
```
  OS        - Ubuntu v18.04, Kernel v4.15
  IDE       - Eclipse IDE for C/C++ Developers v4.10 (2018-12)
  Buildtool - GNU Make v4.1
  Toolchain - linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
```
##### for Centos 7.x need to install latest stable GCC, CMAKE and VIM from sources
## License
  Copyright (c) A1 Company LLC. All rights reserved.