/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "lws.h"
#include "headers.h"

/*****************************************************************************/
#undef info
#define info(FORMAT,...)
/*****************************************************************************/

#ifndef CURRENT_SETUP_MODE_TYPE
#error forgot #include configuration.h ?
#endif

int lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                 void *user, void *in, size_t len)
{
  const char *header = "lws callback:";
  switch(reason)
  {
    case LWS_CALLBACK_ESTABLISHED:
      info("%s established", header);
      break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
      error("%s client connection error", header);
      if((in != NULL) && (len > 0))
        error("%s %s", header, (char *)in);
      else
        error("%s no error messages", header);
      break;
    }
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
      info("%s client filter pre establish", header);
      break;

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
      info("%s connected to server '%s:%d'", header,
           WEBSOCKET_SERVER_ADDR,
           WEBSOCKET_SERVER_PORT);
      event_set_websocket_connected_to_server();
      device_set_state(eConnectedToServer);
      break;
    }
    case LWS_CALLBACK_CLOSED:
    {
      warning("%s closed", header);
      event_set_websocket_disconnected_from_server();
      device_set_state(eDisconnectedFromServer);
      break;
    }
    case LWS_CALLBACK_CLOSED_HTTP:
      info("%s closed http", header);
      break;

    case LWS_CALLBACK_RECEIVE:
    {
#if CURRENT_SETUP_MODE_TYPE == SETUP_MODE_TYPE_AP
      ((char *)in)[len] = '\0';
      info("%s server receive '%s' length %d", header, (char *)in, (int)len);
      lws_server_receive_handler(wsi, (uint8_t *)in, (uint16_t)len);
#else
      error("FIXME! This was supposed to be AP only !!!!");
#endif
      break;
    }
    case LWS_CALLBACK_RECEIVE_PONG:
      info("%s receive pong", header);
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
      info("%s client receive pong", header);
      break;

    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
      info("%s client writeable", header);
      char *p = (char *)lws_get_protocol(wsi)->user;
      char buf[LWS_PRE + 128];
      memcpy(&buf[LWS_PRE], p, strlen(p));
      lws_write(wsi, (unsigned char *)&buf[LWS_PRE], strlen(p), LWS_WRITE_TEXT);
      break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE:
      info("%s server writeable", header);
      break;

    case LWS_CALLBACK_HTTP:
      info("%s http", header);
      break;

    case LWS_CALLBACK_HTTP_BODY:
      info("%s http body", header);
      break;

    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
      info("%s http body completion", header);
      break;

    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
      info("%s http file completion", header);
      break;

    case LWS_CALLBACK_HTTP_WRITEABLE:
      info("%s http writeable", header);
      break;

    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
      info("%s filter network connection", header);
      break;

    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
      info("%s filter http connection", header);
      break;

    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
      info("%s server new client instantiated", header);
      break;

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
      info("%s filter protocol connection", header);
      break;

    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
      info("%s openssl load extra client verify certs", header);
      break;

    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:
      info("%s openssl load extra server verify certs", header);
      break;

    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
      info("%s openssl perform client cert verification", header);
      break;

    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
    {
      info("%s client append handshake header", header);
      break;
    }
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
      info("%s confirm extension okay", header);
      break;

    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
      info("%s client confirm extension supported", header);
      break;

    case LWS_CALLBACK_PROTOCOL_INIT:
      info("%s protocol init", header);
      break;

    case LWS_CALLBACK_PROTOCOL_DESTROY:
      info("%s protocol destroy", header);
      break;

    case LWS_CALLBACK_WSI_CREATE:
      info("%s wsi create", header);
      break;

    case LWS_CALLBACK_WSI_DESTROY:
      info("%s wsi destroy", header);
      event_set_websocket_disconnected_from_server();
      device_set_state(eDisconnectedFromServer);
      break;

    case LWS_CALLBACK_GET_THREAD_ID:
      info("%s get thread id", header);
      break;

    case LWS_CALLBACK_ADD_POLL_FD:
      info("%s add poll fd", header);
      break;

    case LWS_CALLBACK_DEL_POLL_FD:
      info("%s del poll fd", header);
      break;

    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
      //info("%s change mode poll fd", header);
      break;

    case LWS_CALLBACK_LOCK_POLL:
      //info("%s lock poll", header);
      break;

    case LWS_CALLBACK_UNLOCK_POLL:
      //info("%s unlock poll", header);
      break;

    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
      info("%s ws peer initiated close", header);
      break;

    case LWS_CALLBACK_WS_EXT_DEFAULTS:
      info("%s ws ext defaults", header);
      break;

    case LWS_CALLBACK_CGI:
      info("%s callback cgi", header);
      break;

    case LWS_CALLBACK_CGI_TERMINATED:
      info("%s cgi terminated", header);
      break;

    case LWS_CALLBACK_CGI_STDIN_DATA:
      info("%s cgi stdin data", header);
      break;

    case LWS_CALLBACK_CGI_STDIN_COMPLETED:
      info("%s cgi stdin completed", header);
      break;

    case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
      info("%s established client http", header);
      break;

    case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
      info("%s closed client http", header);
      break;

    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
      info("%s receive client http", header);
      break;

    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
      info("%s completed client http", header);
      break;

    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
      info("%s receive client http read", header);
      break;

    case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
      info("%s http bind protocol", header);
      break;

    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
      info("%s http drop protocol", header);
      break;

    case LWS_CALLBACK_CHECK_ACCESS_RIGHTS:
      info("%s check access rights", header);
      break;

    case LWS_CALLBACK_PROCESS_HTML:
      info("%s process html", header);
      break;

    case LWS_CALLBACK_ADD_HEADERS:
      info("%s add headers", header);
      break;

    case LWS_CALLBACK_SESSION_INFO:
      info("%s session info", header);
      break;

    case LWS_CALLBACK_GS_EVENT:
      info("%s gs event", header);
      break;

    case LWS_CALLBACK_HTTP_PMO:
      info("%s http pmo", header);
      break;

    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
    {
      info("%s client http writeable", header);
      break;
    }
    case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
      info("%s openssl perform server cert verification", header);
      break;

    case LWS_CALLBACK_RAW_RX:
      info("%s raw rx", header);
      break;

    case LWS_CALLBACK_RAW_CLOSE:
      info("%s raw close", header);
      break;

    case LWS_CALLBACK_RAW_WRITEABLE:
      info("%s raw writeable", header);
      break;

    case LWS_CALLBACK_RAW_ADOPT:
      info("%s raw adopt", header);
      break;

    case LWS_CALLBACK_RAW_ADOPT_FILE:
      info("%s raw adopt file", header);
      break;

    case LWS_CALLBACK_RAW_RX_FILE:
      info("%s raw rx file", header);
      break;

    case LWS_CALLBACK_RAW_WRITEABLE_FILE:
      info("%s raw writeable file", header);
      break;

    case LWS_CALLBACK_RAW_CLOSE_FILE:
      info("%s raw close file", header);
      break;

    case LWS_CALLBACK_SSL_INFO:
      info("%s ssl info", header);
      break;

    case LWS_CALLBACK_CHILD_CLOSING:
      info("%s child closing", header);
      break;

    case LWS_CALLBACK_USER:
      info("%s user", header);
      break;

    case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:
      info("%s LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY", header);
      break;

    case LWS_CALLBACK_CGI_PROCESS_ATTACH:
      info("%s LWS_CALLBACK_CGI_PROCESS_ATTACH", header);
      break;

    case LWS_CALLBACK_TIMER:
      info("%s LWS_CALLBACK_TIMER", header);
      break;

    case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
      info("%s LWS_CALLBACK_EVENT_WAIT_CANCELLED", header);
      break;

    case LWS_CALLBACK_VHOST_CERT_AGING:
      info("%s LWS_CALLBACK_VHOST_CERT_AGING", header);
      break;

    case LWS_CALLBACK_VHOST_CERT_UPDATE:
      info("%s LWS_CALLBACK_VHOST_CERT_UPDATE", header);
      break;

    case LWS_CALLBACK_CLIENT_CLOSED:
      info("%s LWS_CALLBACK_CLIENT_CLOSED", header);
      break;

  }

  return 0;
}

/*****************************************************************************/
