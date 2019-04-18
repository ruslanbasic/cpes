/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "stpm3x.h"

#include <stddef.h>
#include "metrology.h"
#include "sdkconfig.h"

#define FACTOR_POWER_ON_ENERGY (858) //(3600 * 16000000 / 0x4000000) = 858.3...

/*+---------------------------------------------------------------------------+
 |                                    U32                                     |
 |------------------|------------------|------------------|-------------------|
 |    STPM EXT4     |    STPM EXT3     |    STPM EXT2     |   STPM EXT1       |
 |------------------|------------------|------------------|-------------------|
 |   u4   |    u4   |   u4   |   u4    |    u4  |     u4  |    u4   |  u4     |
 |--------|---------|---------------------------------------------------------|
 |CH masks|STPM type|CH masks|STPM type|CH masks|STPM type|CH masks |STPM type|
 |--------|---------|---------------------------------------------------------|

STPM CFG EXTx (u8):
-----------------
MSB u4 : Channel  Mask :  Channels affected to STPM
    0 : No Channel affected
    1 : Channel 1 affected
    2 : Channel 2 affected
    4 : Channel 3 affected
    8 : Channel 4 affected

LSB u4 :  STPM type : 6 to 8
    0 : No STPM
    6 : STPM32
    7 : STPM33
    8 : STPM34

EX : STPM EXT 1: One STPM34 with Channels 2 and 3 affected on it
LSB u4 = 8 (STPM34)
MSB u4 = 6 ( 4:Channel 3 + 2:Channel 2)

STPM CONFIG : U32 = 0x00000068
-----------------------------------------------------------------------------*/

const nvmLeg_t metroDefault = {
                    // config
  0x00000016,
  {                 // data1[19] STPM (Config for SH)
    0x040000a0,
    0x240000a0,
    0x000004e0,
    0x00000000,
    0x003ff800,
    0x003ff800,
    0x003ff800,
    0x003ff800,
    0x00000fff,
    0x00000fff,
    0x00000fff,
    0x00000fff,
    0x03270327,
    0x03270327,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00004007,
  },
  {                // powerFact[2]      calibration ki = 0,96
    (7226760LL*1000*5*96)/(CONFIG_STPM32_KSI_SHUNT_MILLIOHM*1000*CONFIG_STPM32_CALIBRATION),       // ch 1
    (7226760LL*1000*5*96)/(CONFIG_STPM32_KSI_SHUNT_MILLIOHM*1000*CONFIG_STPM32_CALIBRATION)        // ch 2
  },
  {                // voltageFact[2]
    116274,        // ch 1
    116274         // ch 2
  },
  {                // currentFact[2]    calibration ki = 0,96
    (12291LL*1000*5*96)/(CONFIG_STPM32_KSI_SHUNT_MILLIOHM*1000*CONFIG_STPM32_CALIBRATION),         // ch 1
    (12291LL*1000*5*96)/(CONFIG_STPM32_KSI_SHUNT_MILLIOHM*1000*CONFIG_STPM32_CALIBRATION),         // ch 2
  }
};

METRO_Device_Config_t Tab_METRO_Global_Devices_Config[NB_MAX_DEVICE];

extern METRO_Device_Config_t Tab_METRO_internal_Devices_Config[NB_MAX_DEVICE];

#ifdef UART_XFER_STPM3X /* UART MODE */

void stpm3x_uart_set_pins(uart_port_t port, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num)
{
  uart_set_pin(port, tx_io_num, rx_io_num, rts_io_num, cts_io_num);
}

void stpm3x_uart_initialize(uart_port_t port, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num)
  {
    const int uart_num = port;
    uart_config_t uart_config = {
        .baud_rate = STPM_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(uart_num, &uart_config);
    stpm3x_uart_set_pins(uart_num, tx_io_num, rx_io_num, rts_io_num, cts_io_num);
    uart_driver_install(uart_num, UART_FIFO_LEN * 2, 0, 0, NULL, 0);

    #define UART_EMPTY_THRESH_DEFAULT  (10)
    #define UART_FULL_THRESH_DEFAULT  (1)
    #define UART_TOUT_THRESH_DEFAULT (10)

    uart_intr_config_t uart_intr = {
           .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M
                               | UART_RXFIFO_TOUT_INT_ENA_M
                               | UART_FRM_ERR_INT_ENA_M
                               | UART_RXFIFO_OVF_INT_ENA_M
                               | UART_BRK_DET_INT_ENA_M
                               | UART_PARITY_ERR_INT_ENA_M,
           .rxfifo_full_thresh = UART_FULL_THRESH_DEFAULT,
           .rx_timeout_thresh = UART_TOUT_THRESH_DEFAULT,
           .txfifo_empty_intr_thresh = UART_EMPTY_THRESH_DEFAULT
       };
    ESP_ERROR_CHECK(uart_intr_config(port, &uart_intr));
  }
#endif

#ifdef SPI_XFER_STPM3X /* SPI MODE */
  static void stpm3x_spi_initialize()
  {

  }
#endif

void stpm3x_initialize(uart_port_t port)
{
  hal_set_esp32_port_num(port);

#ifdef UART_XFER_STPM3X /* UART MODE */
  //stpm3x_uart_initialize();
  uart_flush(port);
#endif

#ifdef SPI_XFER_STPM3X /* SPI MODE */
  stpm3x_spi_initialize();
#endif

  /* initialization device type and number of channel */
  Metro_Setup((uint32_t)metroDefault.config);

  /* power STPM properly with EN pin to set it in UART or SPI mode */
  Metro_power_up_device();

  /* initialization steps for STPM device */
  Metro_Init();

  #ifdef UART_XFER_STPM3X /* UART MODE */
    /* Change UART speed for STPM communication between Host and EXT1*/
    Metro_UartSpeed(STPM_UART_BAUD_RATE);
  #endif

  /* Write configuration to STPM device and read back configuration from STPM device */
  Metro_ApplyConfig((uint32_t)metroDefault.config,(uint32_t)metroDefault.data1);

  /* Initialize the factors for the computation */
  for(int i = 0; i < sizeof(metroDefault.powerFact)/sizeof(metroDefault.powerFact[0]); i++)
  {
    Metro_Set_Hardware_Factors((METRO_Channel_t)(CHANNEL_1 + i),
                               (uint32_t)metroDefault.powerFact[i],
                               (uint32_t)metroDefault.powerFact[i] / FACTOR_POWER_ON_ENERGY,
                               (uint32_t)metroDefault.voltageFact[i],
                               (uint32_t)metroDefault.currentFact[i]);
  }

  if(Tab_METRO_internal_Devices_Config[EXT1].device != 0)
  {
    /* Set default latch device type inside Metro struct for Ext chips */
    Metro_Register_Latch_device_Config_type(EXT1, LATCH_SW);
  }
}

void stpm3x_latch_measures()
{
  if(Tab_METRO_internal_Devices_Config[EXT1].device != 0)
  {
    Metro_Set_Latch_device_type(EXT1, LATCH_SW);
  }
}

metroData_t stpm3x_update_measures()
{
  static metroData_t metroData;

  if(Tab_METRO_internal_Devices_Config[EXT1].device != 0)
  {
    metroData.energyActive   = Metro_Read_energy(CHANNEL_1, E_W_ACTIVE);
    metroData.energyReactive = Metro_Read_energy(CHANNEL_1, E_REACTIVE);
    metroData.energyApparent = Metro_Read_energy(CHANNEL_1, E_APPARENT);

    metroData.powerActive = Metro_Read_Power(CHANNEL_1, W_ACTIVE);
    metroData.powerReactive = Metro_Read_Power(CHANNEL_1, REACTIVE);
    metroData.powerApparent = Metro_Read_Power(CHANNEL_1, APPARENT_RMS);

    Metro_Read_RMS(CHANNEL_1,&metroData.rmsvoltage,&metroData.rmscurrent,1);
  }

  return metroData;
}

void stpm3x_set_auto_latch()
{
  Metro_Set_Latch_device_type(EXT1, LATCH_AUTO);
}

int32_t stpm3x_get_adc_voltage(int8_t* error)
{
  return Metro_HAL_read_adc_v(error);
}

int32_t stpm3x_get_adc_current(int8_t* error)
{
  return Metro_HAL_read_adc_c(error);
}

boolean stpm3x_is_alive()
{
  /* Make one access ( first reg u32 reg) to ext chip to check if it is available */
  uint32_t out_p_Buffer;
  Metrology_HAL_ReadBlock(EXT1, 0, 1, &out_p_Buffer);
  return out_p_Buffer == 0x040000a0;
}

//void stpm3x_task(void *pvParameters)
//{
//  stpm3x_initialize();
//
//  for(;;)
//  {
//    stpm3x_latch_measures();
//    stpm3x_update_measures();
//
//    vTaskDelay(1000 / portTICK_PERIOD_MS);
//  }
//  vTaskDelete(NULL);
//}

/*****************************************************************************/
