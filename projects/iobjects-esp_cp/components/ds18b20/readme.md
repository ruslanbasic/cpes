# Component: ds18b20

##Description:

    The DS18B20 digital thermometer provides 9-bit to 12-bit
    Celsius temperature measurements and has an alarm
    function with nonvolatile user-programmable upper and
    lower trigger points. The DS18B20 communicates over a
    1-Wire bus that by definition requires only one data line
    (and ground) for communication with a central micro-
    processor. In addition, the DS18B20 can derive power
    directly from the data line (“parasite power”), eliminating
    the need for an external power supply.
    Each DS18B20 has a unique 64-bit serial code, which
    allows multiple DS18B20s to function on the same 1-Wire
    bus. Thus, it is simple to use one microprocessor to
    control many DS18B20s distributed over a large area.
    Applications that can benefit from this feature include
    HVAC environmental controls, temperature monitoring
    systems inside buildings, equipment, or machinery, and
    process monitoring and control systems.

##Example:

    ds18b20_initialize(GPIO_NUM_4);
    float temp = ds18b20_get_temperature(GPIO_NUM_4);

    printf("ds18b20: $%2.1f\n", temp);

## License

Copyright (c) A1 Company LLC. All rights reserved.