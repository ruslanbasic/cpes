/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lws.h"
#include "headers.h"

static const char *TAG = "lws_callbacks";

void lws_server_receive_handler(struct lws *wsi, uint8_t data[], uint16_t size)
{

}

int lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                 void *user, void *in, size_t len)
{
  const char *header = "lws callback:";
  switch(reason)
  {
    case LWS_CALLBACK_ESTABLISHED:
      ESP_LOGI(TAG, "%s established", header);
      break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
      ESP_LOGE(TAG, "%s client connection error", header);
      if((in != NULL) && (len > 0))
        ESP_LOGE(TAG, "%s %s", header, (char *)in);
      else
        ESP_LOGE(TAG, "%s no error messages", header);
      break;
    }
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
      ESP_LOGI(TAG, "%s client filter pre establish", header);
      break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
      ESP_LOGI(TAG, "%s connected to server '%s:%d'", header,
               WEBSOCKET_SERVER_ADDR,
               WEBSOCKET_SERVER_PORT);
      event_set_websocket_connected_to_server();
      device_set_state(eConnectedToServer);
      break;
    }
    case LWS_CALLBACK_CLOSED:
    {
      ESP_LOGW(TAG, "%s closed", header);
      event_set_websocket_disconnected_from_server();
      device_set_state(eDisconnectedFromServer);
      break;
    }
    case LWS_CALLBACK_CLOSED_HTTP:
      ESP_LOGI(TAG, "%s closed http", header);
      break;

    case LWS_CALLBACK_RECEIVE:
    {
#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_AP
      ((char *)in)[len] = '\0';
      ESP_LOGI(TAG, "%s server receive '%s' length %d", header, (char *)in, (int)len);
      lws_server_receive_handler(wsi, (uint8_t *)in, (uint16_t)len);
#else
      ESP_LOGE(TAG, "FIXME! This was supposed to be AP only !!!!");
#endif
      break;
    }
    case LWS_CALLBACK_RECEIVE_PONG:
      ESP_LOGI(TAG, "%s receive pong", header);
      break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
      if((in != NULL) && (len > 0))
      {
        ((char *)in)[len] = '\0';
        lws_client_receive_handler(wsi, (uint8_t *)in, (uint16_t)len);
      }
      break;
    }
    case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
      ESP_LOGI(TAG, "%s client receive pong", header);
      break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
      ESP_LOGI(TAG, "%s client writeable", header);
      char *p = (char *)lws_get_protocol(wsi)->user;
      char buf[LWS_PRE + 128];
      memcpy(&buf[LWS_PRE], p, strlen(p));
      lws_write(wsi, (unsigned char *)&buf[LWS_PRE], strlen(p), LWS_WRITE_TEXT);
      break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE:
      ESP_LOGI(TAG, "%s server writeable", header);
      break;

    case LWS_CALLBACK_HTTP:
      ESP_LOGI(TAG, "%s http", header);
      break;

    case LWS_CALLBACK_HTTP_BODY:
      ESP_LOGI(TAG, "%s http body", header);
      break;

    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
      ESP_LOGI(TAG, "%s http body completion", header);
      break;

    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
      ESP_LOGI(TAG, "%s http file completion", header);
      break;

    case LWS_CALLBACK_HTTP_WRITEABLE:
      ESP_LOGI(TAG, "%s http writeable", header);
      break;

    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
      ESP_LOGI(TAG, "%s filter network connection", header);
      break;

    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
      ESP_LOGI(TAG, "%s filter http connection", header);
      break;

    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
      ESP_LOGI(TAG, "%s server new client instantiated", header);
      break;

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
      ESP_LOGI(TAG, "%s filter protocol connection", header);
      break;

    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
      ESP_LOGI(TAG, "%s openssl load extra client verify certs", header);
      break;

    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
      ESP_LOGI(TAG, "%s openssl load extra server verify certs", header);
      break;

    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
      ESP_LOGI(TAG, "%s openssl perform client cert verification", header);
      break;

    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
    {
      ESP_LOGI(TAG, "%s client append handshake header", header);
      break;
    }
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
      ESP_LOGI(TAG, "%s confirm extension okay", header);
      break;

    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
      ESP_LOGI(TAG, "%s client confirm extension supported", header);
      break;

    case LWS_CALLBACK_PROTOCOL_INIT:
      ESP_LOGI(TAG, "%s protocol init", header);
      break;

    case LWS_CALLBACK_PROTOCOL_DESTROY:
      ESP_LOGI(TAG, "%s protocol destroy", header);
      break;

    case LWS_CALLBACK_WSI_CREATE:
      ESP_LOGI(TAG, "%s wsi create", header);
      break;

    case LWS_CALLBACK_WSI_DESTROY:
      ESP_LOGI(TAG, "%s wsi destroy", header);
      event_set_websocket_disconnected_from_server();
      device_set_state(eDisconnectedFromServer);
      break;

    case LWS_CALLBACK_GET_THREAD_ID:
      ESP_LOGI(TAG, "%s get thread id", header);
      break;

    case LWS_CALLBACK_ADD_POLL_FD:
      ESP_LOGI(TAG, "%s add poll fd", header);
      break;

    case LWS_CALLBACK_DEL_POLL_FD:
      ESP_LOGI(TAG, "%s del poll fd", header);
      break;

    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
      //ESP_LOGI(TAG, "%s change mode poll fd", header);
      break;

    case LWS_CALLBACK_LOCK_POLL:
      //ESP_LOGI(TAG, "%s lock poll", header);
      break;

    case LWS_CALLBACK_UNLOCK_POLL:
      //ESP_LOGI(TAG, "%s unlock poll", header);
      break;

    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
      ESP_LOGI(TAG, "%s ws peer initiated close", header);
      break;

    case LWS_CALLBACK_WS_EXT_DEFAULTS:
      ESP_LOGI(TAG, "%s ws ext defaults", header);
      break;

    case LWS_CALLBACK_CGI:
      ESP_LOGI(TAG, "%s callback cgi", header);
      break;

    case LWS_CALLBACK_CGI_TERMINATED:
      ESP_LOGI(TAG, "%s cgi terminated", header);
      break;

    case LWS_CALLBACK_CGI_STDIN_DATA:
      ESP_LOGI(TAG, "%s cgi stdin data", header);
      break;

    case LWS_CALLBACK_CGI_STDIN_COMPLETED:
      ESP_LOGI(TAG, "%s cgi stdin completed", header);
      break;

    case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
      ESP_LOGI(TAG, "%s established client http", header);
      break;

    case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
      ESP_LOGI(TAG, "%s closed client http", header);
      break;

    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
      ESP_LOGI(TAG, "%s receive client http", header);
      break;

    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
      ESP_LOGI(TAG, "%s completed client http", header);
      break;

    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
      ESP_LOGI(TAG, "%s receive client http read", header);
      break;

    case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
      ESP_LOGI(TAG, "%s http bind protocol", header);
      break;

    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
      ESP_LOGI(TAG, "%s http drop protocol", header);
      break;

    case LWS_CALLBACK_CHECK_ACCESS_RIGHTS:
      ESP_LOGI(TAG, "%s check access rights", header);
      break;

    case LWS_CALLBACK_PROCESS_HTML:
      ESP_LOGI(TAG, "%s process html", header);
      break;

    case LWS_CALLBACK_ADD_HEADERS:
      ESP_LOGI(TAG, "%s add headers", header);
      break;

    case LWS_CALLBACK_SESSION_INFO:
      ESP_LOGI(TAG, "%s session info", header);
      break;

    case LWS_CALLBACK_GS_EVENT:
      ESP_LOGI(TAG, "%s gs event", header);
      break;

    case LWS_CALLBACK_HTTP_PMO:
      ESP_LOGI(TAG, "%s http pmo", header);
      break;

    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
    {
      ESP_LOGI(TAG, "%s client http writeable", header);
      break;
    }
    case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
      ESP_LOGI(TAG, "%s openssl perform server cert verification", header);
      break;

    case LWS_CALLBACK_RAW_RX:
      ESP_LOGI(TAG, "%s raw rx", header);
      break;

    case LWS_CALLBACK_RAW_CLOSE:
      ESP_LOGI(TAG, "%s raw close", header);
      break;

    case LWS_CALLBACK_RAW_WRITEABLE:
      ESP_LOGI(TAG, "%s raw writeable", header);
      break;

    case LWS_CALLBACK_RAW_ADOPT:
      ESP_LOGI(TAG, "%s raw adopt", header);
      break;

    case LWS_CALLBACK_RAW_ADOPT_FILE:
      ESP_LOGI(TAG, "%s raw adopt file", header);
      break;

    case LWS_CALLBACK_RAW_RX_FILE:
      ESP_LOGI(TAG, "%s raw rx file", header);
      break;

    case LWS_CALLBACK_RAW_WRITEABLE_FILE:
      ESP_LOGI(TAG, "%s raw writeable file", header);
      break;

    case LWS_CALLBACK_RAW_CLOSE_FILE:
      ESP_LOGI(TAG, "%s raw close file", header);
      break;

    case LWS_CALLBACK_SSL_INFO:
      ESP_LOGI(TAG, "%s ssl info", header);
      break;

    case LWS_CALLBACK_CHILD_CLOSING:
      ESP_LOGI(TAG, "%s child closing", header);
      break;

    case LWS_CALLBACK_USER:
      ESP_LOGI(TAG, "%s user", header);
      break;

    case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY", header);
      break;

    case LWS_CALLBACK_CGI_PROCESS_ATTACH:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_CGI_PROCESS_ATTACH", header);
      break;

    case LWS_CALLBACK_TIMER:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_TIMER", header);
      break;

    case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_EVENT_WAIT_CANCELLED", header);
      break;

    case LWS_CALLBACK_VHOST_CERT_AGING:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_VHOST_CERT_AGING", header);
      break;

    case LWS_CALLBACK_VHOST_CERT_UPDATE:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_VHOST_CERT_UPDATE", header);
      break;

    case LWS_CALLBACK_CLIENT_CLOSED:
      ESP_LOGI(TAG, "%s LWS_CALLBACK_CLIENT_CLOSED", header);
      break;

  }

  return 0;
}

/*****************************************************************************/
