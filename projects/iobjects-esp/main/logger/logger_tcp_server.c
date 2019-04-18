/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "logger.h"
#include "headers.h"

static const char TAG[] = "tcp_server";

static QueueHandle_t tcp_server_send_data_queue;
static QueueHandle_t tcp_queue_item_pool;
static QueueHandle_t tcp_sock_queue;

typedef struct tcp_queue_item_t
{
  char data[TCP_SERVER_SEND_DATA_LEN];
  size_t size;
} *tcp_queue_item_t;

static size_t min_sz(size_t a, size_t b)
{
  return a < b ? a : b;
}

static esp_err_t mem_pool_init()
{
  enum { POOL_LEN = TCP_SERVER_SEND_QUEUE_LEN + 1 };
  tcp_queue_item_pool = xQueueCreate(POOL_LEN, sizeof(tcp_queue_item_t));
  if(tcp_queue_item_pool == NULL)
  {
    ESP_LOGE(TAG, "can't create pull queue, tcp logger disabled");
    return ESP_FAIL;
  }

  for(int i = 0; i < POOL_LEN; i++)
  {
    static struct tcp_queue_item_t pool[POOL_LEN];
    tcp_queue_item_t item = &pool[i];
    if(xQueueSend(tcp_queue_item_pool, &item, 0) != pdPASS)
    {
      ESP_LOGE(TAG, "can't create pool, tcp logger disabled");
      return ESP_FAIL;
    }
  }
  return ESP_OK;
}

static tcp_queue_item_t mem_pool_malloc()
{
  tcp_queue_item_t item;
  if(xQueueReceive(tcp_queue_item_pool, &item, 0) == pdPASS)
  {
    return item;
  }
  else
  {
    return NULL;
  }
}

static void mem_pool_free(tcp_queue_item_t item)
{
  if(xQueueSend(tcp_queue_item_pool, &item, 0) != pdPASS)
  {
    ESP_LOGE(TAG, "pool is damaged, tcp logger disabled");
    tcp_server_send_data_queue = NULL;
  }
}

static size_t logger_tcp_server_send_data(const char data[], size_t size)
{
  if(tcp_server_send_data_queue != NULL)
  {
    tcp_queue_item_t item = mem_pool_malloc();
    if(item != NULL)
    {
      item->size = min_sz(size, TCP_SERVER_SEND_DATA_LEN);
      memcpy(item->data, data, item->size);
      if(xQueueSend(tcp_server_send_data_queue, &item, 0) == errQUEUE_FULL)
      {
        tcp_queue_item_t del;
        if(xQueueReceive(tcp_server_send_data_queue, &del, 0) == pdPASS)
        {
          mem_pool_free(del);
        }
        if(xQueueSend(tcp_server_send_data_queue, &item, 0) != pdPASS)
        {
          mem_pool_free(item);
        }
      }
    }
  }

  return size;
}

/* see newlib and LDFLAGS += -Wl,--wrap=__swrite in makefile */

typedef _READ_WRITE_RETURN_TYPE _EXFUN(swrite_t,(struct _reent *, void *,
                                       const char *,
                                       _READ_WRITE_BUFSIZE_TYPE));

swrite_t __real___swrite, __wrap___swrite;

_READ_WRITE_RETURN_TYPE
_DEFUN(__wrap___swrite, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       char const *buf _AND
       _READ_WRITE_BUFSIZE_TYPE n)
{
  logger_tcp_server_send_data(buf, n);
  return __real___swrite(ptr, cookie, buf, n);
}

static void async_socket_task(void *pvParameters)
{
  char addr_str[128];
  int addr_family;
  int ip_protocol;
  struct sockaddr_in destAddr;

  wifi_sta_wait_got_ip(portMAX_DELAY);

  destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  destAddr.sin_family = AF_INET;
  destAddr.sin_port = htons(TCP_PORT);
  addr_family = AF_INET;
  ip_protocol = IPPROTO_IP;
  inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

  int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
  if(listen_sock < 0)
  {
    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
  }
  else
  {
    /* IP Addr assigned to STA */
    tcpip_adapter_ip_info_t ip_info;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
    ESP_LOGW(TAG, "Socket created ip: %s:%d", ip4addr_ntoa(&ip_info.ip), TCP_PORT);

    int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if(err != 0)
    {
      ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    else
    {
      ESP_LOGI(TAG, "Socket binded");

      err = listen(listen_sock, 1);
      if(err != 0)
      {
        ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
      }
      else
      {
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 sourceAddr;
        uint addrLen = sizeof(sourceAddr);

        while(1)
        {
          int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
          if(sock < 0)
          {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
          }
          else
          {
            xQueueOverwrite(tcp_sock_queue, &sock);

            ESP_LOGI(TAG, "Socket accepted");

            int len;
            char rx_buffer[TCP_SERVER_SEND_DATA_LEN];

            while(1)
            {
              len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
              if(len == 0)
              {
                ESP_LOGI(TAG, "Connection closed");
                break;
              }
              else if(len > 0)
              {
                rx_buffer[len - 1] = 0;
                cmdline_proc(rx_buffer);
              }
            }

            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            close(sock);
            sock = -1;

            xQueueOverwrite(tcp_sock_queue, &sock);
          }
        }
      }
    }
  }

  vTaskDelete(NULL);
}

void logger_tcp_server_task(void *pvParameters)
{
  if(mem_pool_init() != ESP_OK)
  {
    vTaskDelete(NULL);
  }

  tcp_server_send_data_queue = xQueueCreate(TCP_SERVER_SEND_QUEUE_LEN, sizeof(tcp_queue_item_t));
  if(tcp_server_send_data_queue == NULL)
  {
    ESP_LOGE(TAG, "can't create send queue, tcp logger disabled");
    vTaskDelete(NULL);
    return;
  }

  tcp_sock_queue = xQueueCreate(1, sizeof(int));
  if(tcp_sock_queue == NULL)
  {
    ESP_LOGE(TAG, "can't create sock queue, tcp logger disabled");
    vTaskDelete(NULL);
    return;
  }

  xTaskCreate(&async_socket_task, "async_socket_task", 4*1024, NULL, 10, NULL);

  int sock = -1;

  while(1)
  {
    if(sock == -1)
    {
      ESP_LOGI(TAG, "waiting for socket");
    }
    xQueueReceive(tcp_sock_queue, &sock, sock == -1 ? portMAX_DELAY : 0);
    tcp_queue_item_t tx_item;
    xQueueReceive(tcp_server_send_data_queue, &tx_item, portMAX_DELAY);
    if(sock != -1)
    {
      send(sock, tx_item->data, min_sz(tx_item->size, TCP_SERVER_SEND_DATA_LEN), 0);
    }
    mem_pool_free(tx_item);
  }

  vTaskDelete(NULL);
}

/*****************************************************************************/
