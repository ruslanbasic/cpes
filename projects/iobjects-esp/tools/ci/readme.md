##Build->execute shell

    ssh -i /home/esp/ci.pem -o StrictHostKeyChecking=no -T git@bitbucket.org

    rm -rf iobjects-esp

    GIT_SSH_COMMAND="ssh -i /home/esp/ci.pem -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" \
    git clone git@bitbucket.org:a1company/iobjects-esp.git -b alpha --depth 1

    cd iobjects-esp
    export SDKCONFIG_DEFAULTS=main/device/plug/hw-v5/sdkconfig.defaults
    make defconfig
    CFLAGS="-DDEVICE=DEVICE_PLUG -DHW_VERSION=5 -DSW_VERSION=$BUILD_NUMBER" make
    cp build/bootloader/bootloader.bin build/bootloader.bin 
    md5name=`md5sum build/esp32-smart-device.bin | awk '{ print $1 }'`
    mkdir ota
    cp build/esp32-smart-device.bin ota/${md5name}.bin
    echo "{
      \"version\":$BUILD_NUMBER,
      \"interval\":3600,
      \"path\":\"/firmware/plug/hw-v5/${md5name}.bin\",
      \"checksum\":\"$md5name\"
    }" > ota/info.json

##Post-build Actions->Files to archive

    iobjects-esp/ota/*, iobjects-esp/build/esp32-smart-device.bin, iobjects-esp/build/partitions.bin,  iobjects-esp/build/bootloader.bin