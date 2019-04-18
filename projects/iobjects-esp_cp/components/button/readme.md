# Component: button

##Description:

Button actions creator. You can create actions such as one or more short press,
long press. Button component receive action to queue for then handling.

##Example:

    void buttons_task(void *parameters)
    {
      button_handle_t action;
      
      button_initialize();
    
      QueueHandle_t xQueue = xQueueCreate(10, sizeof(button_handle_t));
      const button_handle_t user_one_short_press   = button_create(xQueue, GPIO_NUM_4, 1, 100);
      const button_handle_t user_three_short_press = button_create(xQueue, GPIO_NUM_4, 3, 100);
      const button_handle_t user_one_long_press    = button_create(xQueue, GPIO_NUM_4, 1, 5000);
    
      for(;;)
      {
        BaseType_t xStatus = xQueueReceive(xQueue, &action, portMAX_DELAY);
        if(xStatus == pdPASS)
        {
          if(action == user_one_short_press)
          {
            //one short press action handler
          }
          if(action == user_three_short_press)
          {
            //three short press action handler
          }
          if(action == user_one_long_press)
          {
            //one long press action handler
          }
        }
      }
      vTaskDelete(NULL);
    }

## License

Copyright (c) A1 Company LLC. All rights reserved.