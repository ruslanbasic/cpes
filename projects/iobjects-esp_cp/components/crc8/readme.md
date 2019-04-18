# Component: crc8

##Description:

Modified version of crc8 from linux kernel (github.com/torvalds/linux/blob/master/include/linux/crc8.h).

##Example:

    uint8_t buf[4] = {0x55,0x2f,0x5b,0xc6};
    uint8_t crc8 = crc8(buf, sizeof(buf));

    printf("crc8: $%2.2X\n", crc8);

## License

Copyright (c) A1 Company LLC. All rights reserved.