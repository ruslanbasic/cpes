# Component: encoder

##Description:

    A rotary encoder, also called a shaft encoder, is an
    electro-mechanical device that converts the angular position
    or motion of a shaft or axle to analog or digital output signals.

    This is a special component for smart heater device.

##Example:

    void encoder_task(void *parameters)
    {
      const encoder_options_t encoder_options =
      {
        .gpio_a = GPIO_NUM_4,
        .gpio_b = GPIO_NUM_5,
        .min = 0, /* deg C */
        .max = 32, /* deg C */
      };
      encoder_initialize(&encoder_options);

      for(;;)
      {
        encoder_value_t item = encoder_get_value();

        printf("encoder: %2.1f\n", (float)item.value);
      }
      vTaskDelete(NULL);
    }

## License

Copyright (c) A1 Company LLC. All rights reserved.