/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include <stddef.h>

#include "libwebsockets.h"

void lws_client_task(void *pvParameters);
void lws_server_task(void *pvParameters);

int lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                 void *user, void *in, size_t len);

void lws_server_receive_handler(struct lws *wsi, uint8_t data[], uint16_t size);
void lws_client_receive_handler(struct lws *wsi, uint8_t data[], uint16_t size);
void lws_free_wsi(struct lws *wsi);

void lws_client_set_login_host_port(const char* host_port);
void lws_client_set_websocket_host_port(const char* host_port);

/*****************************************************************************/
