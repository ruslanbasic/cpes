/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unity.h"
#include "headers.h"

static const char TAG[] = "[test lws handlers]";

const struct
{
  const char *request;
  const char *response;
} tests[] =
{
  {
    NULL, //request points to NULL
    NULL, //no response
  },
  {
    "", //empty request
    NULL,
  },
  {
    "not base64", //incorrect request
    NULL,
  },
  {
    "{\"\":\"\"}", //request with empty topic and value
    NULL,
  },
  {
    "{\"0.0.0\"\"0\"}", //request without colon in json
    NULL,
  },
  {
    "{0.0.0:0}", //request without quotation marks
    NULL,
  },
  {
    "\"0.0\":\"0\"", //request without message_id
    NULL,
  },
  {
    "\"0\":\"0\"", //request without method_id and message_id
    NULL,
  },
  {
    "\"0.0.0\":\"0\"", //request without brackets
    NULL,
  },
  {
    "{\"0.0.0\":\"0\"}", //invalid method
    NULL,
  },
  {
    "{\"0.500.0\":\"0\"}", //ignore echo from server
    NULL,
  },
  {
    "{\"0.1500.4242\":\"\"}", //ignore echo from server
    NULL,
  },
  {
    "{\"0.1.0\":\"0\"}", //valid request
    "{\"0.1.0\":\"\"}",
  },
  {
    "{\"0.2.4242\":\"42\"}", //valid request
    "{\"0.2.4242\":\"\"}",
  },
  {
    "{\"0.42.4242\":\"1\"}", //error out of range
    "{\"6.42.4242\":\"\"}",
  },
};

/* dummy functions ***********************************************************/

device_error_t device_control_super(device_method_t method, char *parameters)
{
  return method == 42 ? DEVICE_ERROR_OUT_OF_RANGE : DEVICE_OK;
}

/* mock functions ***********************************************************/

static char* mock_buf;

void publish_context_id(const uint32_t message_id, const publish_context_t* const con)
{
  static char buf[WEBSOCKET_CLIENT_TX_BUF_SIZE];
  assert(mock_buf == NULL); // assert publish is called only once per request
  mock_buf = buf;
  snprintf(buf, sizeof buf,
           "{\"%d.%d.%d\":\"%s\"}", con->error_id, con->method_id, message_id, con->value);
}

/*****************************************************************************/

static void send_request(const char * const data)
{
  mock_buf = NULL;
  const int datalen = data != NULL ? strlen(data) : 0;
  lws_client_receive_handler(NULL, (unsigned char*)data, datalen);
}

static void check_response(const char * const response)
{
  TEST_ASSERT_EQUAL_STRING(response, mock_buf);
}

TEST_CASE("all", TAG)
{
  static uint8_t buf[WEBSOCKET_CLIENT_TX_BUF_SIZE];

  for(int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
  {
    size_t out_len;
    unsigned char *request = NULL;
    if(tests[i].request != NULL)
    {
      mbedtls_base64_encode(buf, sizeof buf, &out_len,
                            (const unsigned char *)tests[i].request,
                            strlen(tests[i].request));
      request = buf;
      request[out_len - 1] = '\0'; //overwrite '\n' symbol
    }

    send_request((char*)request);
    check_response(tests[i].response);
  }
}

/*****************************************************************************/
