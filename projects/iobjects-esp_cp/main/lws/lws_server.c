/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/
/*
 * Copyright (C) 2017 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The person who associated a work with this deed has dedicated
 * the work to the public domain by waiving all of his or her rights
 * to the work worldwide under copyright law, including all related
 * and neighboring rights, to the extent allowed by law. You can copy,
 * modify, distribute and perform the work, even for commercial purposes,
 * all without asking permission.
 *
 * The test apps are intended to be adapted for use in your code, which
 * may be proprietary.  So unlike the library itself, they are licensed
 * Public Domain.
 *
 */

#include "lws.h"
#include "headers.h"

#ifndef CURRENT_SETUP_MODE_TYPE
#error forgot #include configuration.h ?
#endif

#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_AP

#include "protocol_lws_meta.c"
#include "protocol_esp32_lws_reboot_to_factory.c"
#include "protocol_dumb_increment.c"
#include "protocol_lws_mirror.c"
#include "protocol_lws_status.c"
#include "protocol_post_demo.c"

static const struct lws_protocols protocols_station[] =
{
  {
    "http-only",
    //lws_callback_http_dummy,
    lws_callback,
    0,
    1024, 0, NULL, 900
  },
  LWS_PLUGIN_PROTOCOL_DUMB_INCREMENT, /* demo... */
  LWS_PLUGIN_PROTOCOL_MIRROR,     /* replace with */
  LWS_PLUGIN_PROTOCOL_POST_DEMO,      /* your own */
  LWS_PLUGIN_PROTOCOL_LWS_STATUS,     /* plugin protocol */
  LWS_PLUGIN_PROTOCOL_ESPLWS_RTF, /* helper protocol to allow reset to factory */
  LWS_PLUGIN_PROTOCOL_LWS_META,     /* protocol multiplexer */
  { NULL, NULL, 0, 0, 0, NULL, 0 } /* terminator */
};

static const struct lws_protocol_vhost_options pvo_headers =
{
  NULL,
  NULL,
  "Keep-Alive:",
  "timeout=5, max=20",
};

/* reset to factory mount */
static const struct lws_http_mount mount_station_rtf =
{
  .mountpoint   = "/esp32-rtf",
  .origin     = "esplws-rtf",
  .origin_protocol  = LWSMPRO_CALLBACK,
  .mountpoint_len   = 10,
};

/*
 * this makes a special URL "/formtest" which gets passed to
 * the "protocol-post-demo" plugin protocol for handling
 */
static const struct lws_http_mount mount_station_post =
{
  .mount_next   = &mount_station_rtf,
  .mountpoint   = "/formtest",
  .origin     = "protocol-post-demo",
  .origin_protocol  = LWSMPRO_CALLBACK,
  .mountpoint_len   = 9,
};

/*
 * this serves "/station/..." in the romfs at "/" in the URL namespace
 */
static const struct lws_http_mount mount_station =
{
  .mount_next   = &mount_station_post,
  .mountpoint   = "/",
  .origin     = "/station",
  .def      = "test.html",
  .origin_protocol  = LWSMPRO_FILE,
  .mountpoint_len   = 1,
};

void lws_server_task(void *pvParameters)
{
  static struct lws_context_creation_info server_context_info;
  struct lws_context *context;
  struct lws_vhost *vhost;
  int next_dump_secs;

  memset(&server_context_info, 0, sizeof(struct lws_context_creation_info));
  server_context_info.port = 80;
  server_context_info.fd_limit_per_thread = 30;
  server_context_info.max_http_header_pool = 3;
  server_context_info.max_http_header_data = 512;
  server_context_info.pt_serv_buf_size = 900;
  server_context_info.keepalive_timeout = 5;
  server_context_info.options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
  server_context_info.vhost_name = "station";
  server_context_info.protocols = protocols_station;
  server_context_info.mounts = &mount_station;
  server_context_info.headers = &pvo_headers;

  wifi_ap_initialize();
  context = lws_esp32_init(&server_context_info, &vhost);

  next_dump_secs = lws_now_secs();

  while(!lws_service(context, 100))
  {
    if(lws_now_secs() > next_dump_secs)
    {
      info("Settings mode. Please enter the settings: ssid and password of your router");
      next_dump_secs = lws_now_secs() + 1; //5
    }
    taskYIELD();
  }
  vTaskDelete(NULL);
}

#endif // SETUP_MODE_TYPE_AP

/*****************************************************************************/
