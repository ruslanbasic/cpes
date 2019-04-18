/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "actions.h"
#include "unicode.h"
#include "headers.h"

typedef enum
{
  eAction_NONE = -1,
  eAction_OFF = 0,
#if(DEVICE == DEVICE_PLUG)
  eActionSocket_ON,
#endif
} eAction_t;

typedef struct
{
  const char* value;
  eAction_t action;
} element_t;

static element_t dictionary[] =
{
#if(DEVICE == DEVICE_PLUG)
  {"розетк", eActionSocket_ON},
#endif
  {"ключ", eAction_OFF},
};

/*****************************************************************************/

bool actions_execute(const char* const transcript)
{
  eAction_t action = eAction_NONE;

  for(int i = 0; i < sizeof(dictionary) / sizeof(dictionary[0]); i++)
  {
    if(unicode_stristr(transcript, dictionary[i].value))
    {
      action = dictionary[i].action;
      break;
    }
  }

  switch(action)
  {
#if(DEVICE == DEVICE_PLUG)
    case eActionSocket_ON:
      device_control(TO_DEVICE_ONOFF, "1");
      break;
#endif
    case eAction_OFF:
      device_control(TO_DEVICE_ONOFF, "0");
      break;
    case eAction_NONE:
      return false;
  }

  return true;
}


/*****************************************************************************/
