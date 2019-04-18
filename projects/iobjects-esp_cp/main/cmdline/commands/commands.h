/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#pragma once /****************************************************************/

/* common commands ***********************************************************/
void cmd_deep_sleep();
void cmd_free();
void cmd_info();
void cmd_mic();
void cmd_restart();
void cmd_get_chip_temperature();
void cmd_wifi_disconnect();
void cmd_stt_set_host_port();
void cmd_lws_set_host_port();
void cmd_ota_set_host_port();

/* specific commands *********************************************************/
void cmd_lamp();
void cmd_socket();

/*****************************************************************************/
