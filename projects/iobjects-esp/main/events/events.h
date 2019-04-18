/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

#include "headers.h"

bool event_initialize();

void event_set_websocket_connected_to_server();
void event_set_websocket_disconnected_from_server();

void event_wait_websocket_client_connected_to_server();
bool event_wait_websocket_client_connected_to_server_timeout(TickType_t);

void event_set_urgent_ota_update();
void event_clear_urgent_ota_update();
bool event_wait_urgent_ota_update_or_timeout(TickType_t xTicksToWait);

/*****************************************************************************/
