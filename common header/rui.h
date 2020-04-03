/***************************************************************
 * @file        rui.h
 * @brief       RUI API is called by users for app development.
                For now Including Lora part,cellular part, BLE
                part, AT cmd part, task part sensor part, it can
                help users to build their app easily and elegantly.
 * @author      Rakwireless
 * @version     3.0
 * @date        2020.4
***************************************************************/

#ifndef _RUI_H
#define _RUI_H

#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stddef.h"
#ifdef SOFTDEVICE_PRESENT
#include "ble_types.h"
#include "ble_gap.h"
#include "ble_rcs_c.h"
#include "nrf_log.h"
#endif
#if defined STM32L1XX || STM32L0XX
#include "timer.h"
#endif

/****************************************************************************************
RUI AT return value. Now all RAK products use uniform AT response and form, for example
    1. Right answer
        send   ----> at+version 
        receive----> 
        at+version:
        Firmware Version: RUI v3.0.0.7
        OK
    2. Wrong answer
        send   ----> at+set_config=cellular:send_interval:0,150000 
        receive----> 
        at+set_config=cellular:send_interval:
        ERROR:RUI_AT_RW_FLASH_ERROR 2
******************************************************************************************/

typedef enum{
 RUI_AT_OK=0,
 RUI_AT_UNSUPPORT,
 RUI_AT_PARAMETER_INVALID,
 RUI_AT_RW_FLASH_ERROR,
 RUI_AT_GPIO_IRQ_DISABLE,
 RUI_AT_BUS_INIT_FAIL,
 RUI_AT_TIMER_FAIL,
 RUI_AT_IIC_RW_ERROR,
 RUI_AT_UART_SEND_ERROR,

 RUI_AT_SENSOR_OK=20,
 RUI_AT_BLE_STATUS_OK=40,
 RUI_AT_BLE_ERROR_INVALID_STATE=41,
 RUI_AT_CELLULAR_STATUS_OK=60,
 RUI_AT_LORA_BUSY=80,
 RUI_AT_LORA_SERVICE_UNKNOWN,
 RUI_AT_LORA_PARAMETER_INVALID,
 RUI_AT_LORA_FREQUENCY_INVALID,
 RUI_AT_LORA_DATARATE_INVALID,
 RUI_AT_LORA_FREQ_AND_DR_INVALID,
 RUI_AT_LORA_NO_NETWORK_JOINED,
 RUI_AT_LORA_LENGTH_ERROR,
 RUI_AT_LORA_DEVICE_OFF,
 RUI_AT_LORA_REGION_NOT_SUPPORTED,
 RUI_AT_LORA_DUTYCYCLE_RESTRICTED,
 RUI_AT_LORA_NO_CHANNEL_FOUND,
 RUI_AT_LORA_NO_FREE_CHANNEL_FOUND,
 RUI_AT_LORA_INFO_STATUS_ERROR,
 RUI_AT_LORA_INFO_STATUS_TX_TIMEOUT,
 RUI_AT_LORA_INFO_STATUS_RX1_TIMEOUT,
 RUI_AT_LORA_INFO_STATUS_RX2_TIMEOUT,
 RUI_AT_LORA_INFO_STATUS_RX1_ERROR,
 RUI_AT_LORA_INFO_STATUS_RX2_ERROR,
 RUI_AT_LORA_INFO_STATUS_JOIN_FAIL,
 RUI_AT_LORA_INFO_STATUS_DOWNLINK_REPEATED,
 RUI_AT_LORA_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR,
 RUI_AT_LORA_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS,
 RUI_AT_LORA_INFO_STATUS_ADDRESS_FAIL,
 RUI_AT_LORA_INFO_STATUS_MIC_FAIL
}RUI_AT_RESPONSE;


/* RUI API return value*/
typedef enum{
 RUI_STATUS_OK=0,
 RUI_STATUS_PARAMETER_INVALID,
 RUI_STATUS_RW_FLASH_ERROR,
 RUI_STATUS_GPIO_IRQ_DISABLE,
 RUI_STATUS_BUS_INIT_FAIL,
 RUI_STATUS_TIMER_FAIL,
 RUI_STATUS_IIC_RW_ERROR,
 RUI_STATUS_UART_SEND_ERROR,
 
 RUI_SENSOR_STATUS_OK=20,

 RUI_BLE_STATUS_OK=40,
 RUI_BLE_ERROR_INVALID_STATE=41,

 RUI_CELLULAR_STATUS_OK=60,


 RUI_LORA_STATUS_BUSY=80,
 RUI_LORA_STATUS_SERVICE_UNKNOWN,
 RUI_LORA_STATUS_PARAMETER_INVALID,
 RUI_LORA_STATUS_FREQUENCY_INVALID,
 RUI_LORA_STATUS_DATARATE_INVALID,
 RUI_LORA_STATUS_FREQ_AND_DR_INVALID,
 RUI_LORA_STATUS_NO_NETWORK_JOINED,
 RUI_LORA_STATUS_LENGTH_ERROR,
 RUI_LORA_STATUS_DEVICE_OFF,
 RUI_LORA_STATUS_REGION_NOT_SUPPORTED,
 RUI_LORA_STATUS_DUTYCYCLE_RESTRICTED,
 RUI_LORA_STATUS_NO_CHANNEL_FOUND,
 RUI_LORA_STATUS_NO_FREE_CHANNEL_FOUND,
 RUI_LORA_EVENT_INFO_STATUS_ERROR,
 RUI_LORA_EVENT_INFO_STATUS_TX_TIMEOUT,
 RUI_LORA_EVENT_INFO_STATUS_RX1_TIMEOUT,
 RUI_LORA_EVENT_INFO_STATUS_RX2_TIMEOUT,
 RUI_LORA_EVENT_INFO_STATUS_RX1_ERROR, 
 RUI_LORA_EVENT_INFO_STATUS_RX2_ERROR,
 RUI_LORA_EVENT_INFO_STATUS_JOIN_FAIL,
 RUI_LORA_EVENT_INFO_STATUS_DOWNLINK_REPEATED,
 RUI_LORA_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR,
 RUI_LORA_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS,
 RUI_LORA_EVENT_INFO_STATUS_ADDRESS_FAIL,
 RUI_LORA_EVENT_INFO_STATUS_MIC_FAIL
}RUI_RETURN_STATUS;

typedef struct RUI_RECEIVE
{
    /*!
     * Application port
     */
    uint8_t Port;
    /*!
     * Downlink datarate
     */
    uint8_t RxDatarate;
    /*!
     * Pointer to the received data stream
     */
    uint8_t *Buffer;
    /*!
     * Size of the received data stream
     */
    uint8_t BufferSize;

    /*!
     * Rssi of the received packet
     */
    int16_t Rssi;
    /*!
     * Snr of the received packet
     */
    int8_t Snr;

    /*!
     * The downlink counter value for the received frame
     */
    uint32_t DownLinkCounter;
} RUI_RECEIVE_T;

typedef struct RUI_LORAP2P_RECEIVE
{
    /*!
     * Pointer to the received data stream
     */
    uint8_t *Buffer;
    /*!
     * Size of the received data stream
     */
    uint8_t BufferSize;

    /*!
     * Rssi of the received packet
     */
    int16_t Rssi;
    /*!
     * Snr of the received packet
     */
    uint8_t Snr;

} RUI_LORAP2P_RECEIVE_T;

typedef enum RUI_EMCPS
{
    /*!
     * Unconfirmed LoRaMAC frame
     */
    RUI_MCPS_UNCONFIRMED,
    /*!
     * Confirmed LoRaMAC frame
     */
    RUI_MCPS_CONFIRMED,
    /*!
     * Multicast LoRaMAC frame
     */
    RUI_MCPS_MULTICAST,
    /*!
     * Proprietary frame
     */
    RUI_MCPS_PROPRIETARY,
    /*!
     * LoRa event error
     */
    RUI_LORAEVENT_ERROR
} RUI_MCPS_T;

typedef enum RUI_DRIVER_MODE
{
    RUI_NORMAL_MODE = 0,
    RUI_POWER_ON_MODE,
    RUI_POWER_OFF_MODE,
    RUI_SLEEP_MODE,
    RUI_STANDBY_MODE
} RUI_DRIVER_MODE;


typedef enum RUI_LORA_JOIN_MODE
{
    RUI_OTAA = 0,
    RUI_ABP
} RUI_LORA_JOIN_MODE;


typedef enum RUI_LORA_CLASS_MODE
{
    RUI_CLASS_A = 0,
    RUI_CLASS_B,
    RUI_CLASS_C
} RUI_LORA_CLASS_MODE;

typedef enum RUI_LORA_WORK_MODE
{
    RUI_LORAWAN = 0,
    RUI_P2P,
    RUI_TESTMODE
} RUI_LORA_WORK_MODE;

typedef enum RUI_LORA_AUTO_SEND_MODE
{
    RUI_AUTO_DISABLE=0,     // Disable lora auto send.
    RUI_AUTO_ENABLE_SLEEP,  // Enable lora auto send, sleep when the system is idle.
    RUI_AUTO_ENABLE_NORMAL  // Enable lora auto send, runs normally whe the system is idle. Applicable only to RAK7200, RAK811.
} RUI_LORA_AUTO_SEND_MODE;

typedef enum LORA_REGION
{
    AS923,
    AU915,
    CN470,
    CN779,
    EU433,
    EU868,
    KR920,
    IN865,
    US915,
    US915_Hybrid
} LORA_REGION;

typedef struct RUI_GPS_DATA
{
    uint8_t UTC[4];
    uint8_t Date[3];
    bool LatitudeNS;        //0:N   1:S
    uint8_t LatitudeDegree;
    uint32_t LatitudeMinute;
    double Latitude;
    bool LongitudaEW;       //0:E   1:W
    uint8_t LongitudaDegree;
    double Longitude;
    uint32_t LongitudaMinute;
    int Quality;
    int NumSatellities;
    int16_t HDOP;
    int16_t Altitude;
    int16_t GeoidHeight;
} RUI_GPS_DATA;

typedef enum {
    BLE_MODE_PERIPHERAL = 0,
    BLE_MODE_CENTRAL,
    BLE_MODE_OBSERVER
} BLE_WORK_MODE;

typedef enum RUI_UART_DEF
{
    RUI_UART0 = 0,
    RUI_UART1,
    RUI_UART2,
    RUI_UART3
} RUI_UART_DEF;

typedef enum RUI_UART_MODE
{
    RUI_UART_NORAMAL= 0,
    RUI_UART_UNVARNISHED
} RUI_UART_MODE;

typedef enum RUI_UART_BAUDRATE
{
    BAUDRATE_1200,
    BAUDRATE_2400,
    BAUDRATE_4800,
    BAUDRATE_9600,
    BAUDRATE_19200,
    BAUDRATE_38400,
    BAUDRATE_57600,
    BAUDRATE_115200
} RUI_UART_BAUDRATE;

typedef enum RUI_IF_READ_WRITE
{
    RUI_IF_READ = 0,
    RUI_IF_WRITE = 1
} RUI_IF_READ_WRITE;

typedef struct RUI_LORA_STATUS
{
    uint32_t dev_addr;
    uint8_t dev_eui[8];
    uint8_t app_eui[8];
    uint8_t app_key[16];
    uint8_t nwks_key[16];
    uint8_t apps_key[16];
    RUI_LORA_WORK_MODE work_mode;
    RUI_LORA_CLASS_MODE class_status;
    RUI_LORA_JOIN_MODE join_mode;
    uint8_t lora_dr;
    uint8_t confirm;
    uint16_t lorasend_interval;
    uint8_t autosend_status;
    bool IsJoined;
    bool AdrEnable;
    uint8_t region[5]; //region string e.g:"EU868"
} RUI_LORA_STATUS_T;

#define I2C_REG_MAGIC   0xAA
typedef struct RUI_I2C_ST {
    uint32_t INSTANCE_ID;
    uint32_t PIN_SDA;   // SDA pin num
    uint32_t PIN_SCL;   // SCL pin num
    uint32_t FREQUENCY;
    uint32_t REG_NULL; // if no reg , should be 0xAA
} RUI_I2C_ST;

typedef enum RUI_I2C_FREQ_ST {
    RUI_I2C_FREQ_100K,
    RUI_I2C_FREQ_400K
} RUI_I2C_FREQ_ST;

typedef struct {
    uint32_t INSTANCE_ID;
    uint32_t PIN_CS;   //
    uint32_t PIN_SCL;   //
    uint32_t PIN_MISO;   //
    uint32_t PIN_MOSI;   //
    uint32_t FREQUENCY;
} RUI_SPI_ST;

typedef enum RUI_SPI_FREQ_ST {
    RUI_I2C_FREQ_125K,
    RUI_I2C_FREQ_250K,
    RUI_I2C_FREQ_500K,
    RUI_I2C_FREQ_1M,
    RUI_I2C_FREQ_2M,
    RUI_I2C_FREQ_4M,
    RUI_I2C_FREQ_8M
} RUI_SPI_FREQ_ST;

typedef enum
{
    RUI_GPIO_PIN_DIR_INPUT, // Input.
    RUI_GPIO_PIN_DIR_OUTPUT // Output.
} RUI_GPIO_PIN_DIR_ST;

typedef enum
{
    RUI_GPIO_PIN_NOPULL,   //  Pin pull-up resistor disabled.
    RUI_GPIO_PIN_PULLDOWN, //  Pin pull-down resistor enabled.
    RUI_GPIO_PIN_PULLUP    //  Pin pull-up resistor enabled.
} RUI_GPIO_PIN_PULL_ST;

typedef enum {
    RUI_GPIO_EDGE_RAISE,
    RUI_GPIO_EDGE_FALL,
    RUI_GPIO_EDGE_FALL_RAISE
} RUI_GPIO_INTERRUPT_EDGE;

typedef enum {
    RUI_GPIO_IRQ_HIGH_PRIORITY,
    RUI_GPIO_IRQ_NORMAL_PRIORITY,
    RUI_GPIO_IRQ_LOW_PRIORITY,
} RUI_GPIO_INTERRUPT_PRIORITY;

typedef struct {
    uint32_t pin_num;
    RUI_GPIO_PIN_DIR_ST dir;
    RUI_GPIO_PIN_PULL_ST pull;
} RUI_GPIO_ST;

typedef enum
{
    RUI_TIMER_MODE_SINGLE_SHOT,                 /**< The timer will expire only once. */
    RUI_TIMER_MODE_REPEATED                     /**< The timer will restart each time it expires. */
} RUI_TIMER_MODE_ST;

typedef void (* TIMER_CALLBACK) (void);

typedef struct {
    uint32_t timeout_ms;
    RUI_TIMER_MODE_ST timer_mode;
    TIMER_CALLBACK timer_callback;
} RUI_TIMER_ST;

typedef enum {
    RUI_FLASH_USER = 0,
    RUI_FLASH_ORIGIN
} RUI_FLASH_MODE;

typedef struct RUI_PWM_ST {
    RUI_GPIO_ST rui_gpio;
    uint32_t frequency;  //frequency <= 160000*dutycycle in RAK811
    uint8_t dutycycle;  //value range:0~100 
} RUI_PWM_ST;

/***************************************************************************************
 * @brief       This API is used to configure the parameters for uart.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_UART_DEF uart_def:  the instance of uart.
                RUI_UART_BAUDRATE baudrate:  Uart baudrate value.
***************************************************************************************/
RUI_RETURN_STATUS rui_uart_init(RUI_UART_DEF uart_def,RUI_UART_BAUDRATE baudrate);

/***************************************************************************************
 * @brief       This API is used to uninit uart.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_UART_DEF uart_def:  the instance of uart.
***************************************************************************************/
RUI_RETURN_STATUS rui_uart_uninit(RUI_UART_DEF uart_def);

/***************************************************************************************
 * @brief       This API is used to send data via uart.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_UART_DEF uart_def:  the instance of uart.
                uint8_t *pdata:  the pointer of data.
                uint16_t len:  the lengh of data.
***************************************************************************************/
RUI_RETURN_STATUS rui_uart_send(RUI_UART_DEF uart_def, uint8_t *pdata, uint16_t len);

/***************************************************************************************
 * @brief       This API is used to receive data from uart.
 * @return      NULL
 * @param       RUI_UART_DEF uart_def:  the instance of uart.
                uint8_t *pdata:  the pointer of data.
                uint16_t len:  the lengh of data,This value is always 1.
***************************************************************************************/
void rui_uart_recv(RUI_UART_DEF uart_def, uint8_t *pdata, uint16_t len);

/***************************************************************************************
 * @brief       This API is used to configure uart work mode.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_UART_DEF uart_def:the instance of uart.
                RUI_UART_MODE uart_mode: value for RUI_UART_NORAMAL,RUI_UART_UNVARNISHED.
***************************************************************************************/
RUI_RETURN_STATUS rui_uart_mode_config(RUI_UART_DEF uart_def,RUI_UART_MODE uart_mode);

/***************************************************************************************
 * @brief       This API is used to configure gpio.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_GPIO_ST *rui_gpio:the instance of gpio.
***************************************************************************************/
RUI_RETURN_STATUS rui_gpio_init(RUI_GPIO_ST *rui_gpio);

/***************************************************************************************
 * @brief       This API is used to deinit gpio.
 * @return      void
 * @param       RUI_GPIO_ST *rui_gpio:the instance of gpio.
***************************************************************************************/
void rui_gpio_uninit(RUI_GPIO_ST *rui_gpio);

/***************************************************************************************
 * @brief       This API is used to read or set a certain gpio's status.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_IF_READ_WRITE rw: read or set.
                RUI_GPIO_ST *rui_gpio: the  stru of gpio which you want to read or set.
                uint8_t* status:  the pointer of gpio value.
                                  0: low level, 1: high level.
***************************************************************************************/
RUI_RETURN_STATUS rui_gpio_rw(RUI_IF_READ_WRITE rw_status,RUI_GPIO_ST *rui_gpio,uint8_t* status);

/***************************************************************************************
 * @brief       This API is used to init ADC
 * @return      RUI_RETURN_STATUS
 * @param       RUI_GPIO_ST *rui_gpio:the instance of gpio.
***************************************************************************************/
RUI_RETURN_STATUS rui_adc_init(RUI_GPIO_ST *rui_gpio);

/***************************************************************************************
 * @brief       This API is used to uninit ADC
 * @return      RUI_RETURN_STATUS
 * @param       RUI_GPIO_ST *rui_gpio:the instance of gpio.
***************************************************************************************/
RUI_RETURN_STATUS rui_adc_uninit(RUI_GPIO_ST *rui_gpio);

/***************************************************************************************
 * @brief       This API is used to read the value of ADC pin
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t* value:  the value which is read from ADC.
                RUI_GPIO_ST *rui_gpio:  gpio stru
***************************************************************************************/
RUI_RETURN_STATUS rui_adc_get(RUI_GPIO_ST *rui_gpio, uint16_t* value);

/***************************************************************************************
 * @brief       This API is used to read/write through I2C.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_I2C_ST *rui_i2c:        i2c instance.
                RUI_IF_READ_WRITE rw:       read or write through I2C.
                uint8_t devAddr:            device address, in Hex format. if no addr, use 0
                uint16_t regAddr:           the sensor's register address, in Hex format.
                uint8_t* data:              the data read out or will be write through i2c.
                uint16_t len:               the data length. the unit is byte.
***************************************************************************************/
RUI_RETURN_STATUS rui_i2c_rw(RUI_I2C_ST *rui_i2c,RUI_IF_READ_WRITE rw, uint8_t devAddr, uint16_t regAddr, uint8_t* data, uint16_t len);

/***************************************************************************************
 * @brief       This API is used to init I2C.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_I2C_ST *rui_i2c:  init parameters struct.
***************************************************************************************/
RUI_RETURN_STATUS rui_i2c_init(RUI_I2C_ST *rui_i2c);

/***************************************************************************************
 * @brief       This API is used to init SPI.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_SPI_ST *rui_spi:  init parameters struct.
***************************************************************************************/
RUI_RETURN_STATUS rui_spi_init(RUI_SPI_ST *rui_spi);

/***************************************************************************************
 * @brief       This API is used to read/write through SPI.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_IF_READ_WRITE mode: read or write
                uint8_t *tx:      write through spi.
                uint16_t tx_len:  tx length.
                uint8_t *rx:      read buffer.
                uint16_t rx_len:  rx length.
***************************************************************************************/
RUI_RETURN_STATUS rui_spi_rw(RUI_IF_READ_WRITE mode, uint8_t *tx, uint16_t tx_len, uint8_t *rx, uint16_t rx_len);

/***************************************************************************************
 * @brief       This API is used to get the GPS data from GPS module.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_GPS_DATA *data:  the GPS data
***************************************************************************************/
RUI_RETURN_STATUS rui_gps_get(RUI_GPS_DATA *data);

/***************************************************************************************
 * @brief       This API is used to set the work mode of the GPS module.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_DRIVER_MODE mode:   the GPS module's work mode.
***************************************************************************************/
RUI_RETURN_STATUS rui_gps_set_mode(RUI_DRIVER_MODE mode);

/***************************************************************************************
 * @brief       This API is used to send data through cellular.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *data:  the data which will be sent through cellular.
 **************************************************************************************/
RUI_RETURN_STATUS rui_cellular_send(uint8_t *data);

/***************************************************************************************
 * @brief       This API is used to join to remote server with user's config.
 * @return      RUI_RETURN_STATUS
 * @param       void
**************************************************************************************/
RUI_RETURN_STATUS rui_cellular_join(void);


/***************************************************************************************
 * @brief       This API is used to config ip and port of remote server for tcp link.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *ip:  ip of remote server.
                uint8_t *port:  port of remote server.
**************************************************************************************/
RUI_RETURN_STATUS rui_cellular_set_server(uint8_t *ip,uint8_t *port);

/***************************************************************************************
 * @brief       This API is used to open socket of remote server for tcp link.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t* data: open tcp link cmd like:AT+QIOPEN=1,0,"TCP","ip",port,0,1"
**************************************************************************************/
RUI_RETURN_STATUS rui_cellular_open_socket(uint8_t* data);

/***************************************************************************************
 * @brief       This API is used to config parameters of operator.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *APN:       APN of operator.
                uint8_t *operator_long_name :  operator_long_name of operator.
                uint8_t *operator_short_name:  operator_short_name of operator
                uint8_t operator_net        :  operator_net of operator
**************************************************************************************/
RUI_RETURN_STATUS rui_cellular_set_operator(uint8_t *APN,uint8_t *operator_long_name,uint8_t *operator_short_name,uint8_t operator_net);

/***************************************************************************************
 * @brief       This API is used to wait for a correct response from cellular module.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *response:  the response of server string
                uint32_t len:       response data length
                uint32_t timeout:   the time to wait for response until timeout.
****************************************************************************************/
RUI_RETURN_STATUS rui_cellular_response(uint8_t *response, uint32_t len, uint32_t timeout);

/****************************************************************************************
 * @brief       This API is used to set the work mode of the cellular module.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_DRIVER_MODE mode:  the cellular module's work mode
 ***************************************************************************************/
RUI_RETURN_STATUS rui_cellular_set_mode(RUI_DRIVER_MODE mode);

/****************************************************************************************
 * @brief       This API is used to get the connect status the cellular module.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *status: 0 holoram; 1 server
 ***************************************************************************************/
RUI_RETURN_STATUS rui_cellular_connect_status(uint8_t *status);

/****************************************************************************************
 * @brief       This API is used to send all sensors data to holorgam server.
 * @return      RUI_RETURN_STATUS
 * @param       void
 ***************************************************************************************/
RUI_RETURN_STATUS rui_cellular_hologram_send(void);

/***************************************************************************************
 * @brief       This API is used to register a callback function for cellular in application
                so that application can receive cellular data automatically.
 * @return      RUI_RETURN_STATUS
 * @param       cellular_receive callback:  the callback function for receiving cellular data.
***************************************************************************************/
typedef void (*cellular_receive)(uint8_t *data);
RUI_RETURN_STATUS rui_cellular_register_recv_callback(cellular_receive callback);

/***************************************************************************************
 * @brief       rui_lora_join       join to server
 * @return      RUI_RETURN_STATUS
 * @param       void
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_join(void);

/***************************************************************************************
 * @brief       rui_lora_send
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t port:   send data port
                uint8_t* data:  send data string
                uint8_t len:    send data length
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_send(uint8_t port,uint8_t* data,uint8_t len);

/***************************************************************************************
 * @brief       This API is used to register a callback function for LoRa in application,
                so that application can receive the LoRa data automatically.
 * @return      RUI_RETURN_STATUS
 * @param       lora_receive callback:  the callback function for receiving LoRa data.
***************************************************************************************/
typedef void (*lora_receive)(RUI_RECEIVE_T *data);
RUI_RETURN_STATUS rui_lora_register_recv_callback(lora_receive callback);

/***************************************************************************************
 * @brief       This API is used to set the work mode of LoRa module.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_DRIVER_MODE mode:  lora peripheral work mode
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_device_mode(RUI_DRIVER_MODE mode);

/***************************************************************************************
 * @brief       This API is used to set the device EUI for LoRaWAN OTAA mode.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *dev_eui:   the device EUI.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_dev_eui(uint8_t *dev_eui);

/***************************************************************************************
 * @brief       This API is used to set the application EUI for LoRaWAN OTAA mode.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *app_eui:   the application EUI.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_app_eui(uint8_t *app_eui);

/***************************************************************************************
 * @brief       This API is used to set the application key for LoRaWAN OTAA mode.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *app_key:   the application key.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_app_key(uint8_t *app_key);

/***************************************************************************************
 * @brief       This API is used to set the device address for LoRaWAN ABP mode.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *dev_addr:  the device address.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_dev_addr(uint8_t *dev_addr);

/***************************************************************************************
 * @brief       This API is used to set the application session key for LoRaWAN ABP mode.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *apps_key: the application session key.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_apps_key(uint8_t *apps_key);

/***************************************************************************************
 * @brief       This API is used to set the network session key for LoRaWAN ABP mode.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *nwks_key:  the network session key.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_nwks_key(uint8_t *nwks_key);

/***************************************************************************************
 * @brief       This API is used to turn a certain channel on or off.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t channel:  the channel number you want to set.
                uint8_t on_off:  turn on or turn off.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_channel_mask(uint8_t channel, uint8_t on_off);

/***************************************************************************************
 * @brief       This API is used to set the LoRaWAN Class.
 * @return      RUI_RETURN_STATUS
 * @param       LORA_CLASS_MODE class:  Class A, Class B, or Class C.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_class(RUI_LORA_CLASS_MODE class);

/***************************************************************************************
 * @brief       This API is used to set the send confirm.
 * @return      RUI_RETURN_STATUS
 * @param       bool is_confirm: true-confirm, false-unconfirm.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_confirm(bool is_confirm);

/***************************************************************************************
 * @brief       This API is used to set the ADR for LoRa node.
 * @return      RUI_RETURN_STATUS
 * @param       bool is_enable: true-enable, false-disable
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_adr(bool is_enable);

/***************************************************************************************
 * @brief       This API is used to set the DR for LoRa node.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t dr: the value of DR.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_dr(uint8_t dr);

/***************************************************************************************
 * @brief       This API is used to set the join mode of LoRaWAN.
 * @return      RUI_RETURN_STATUS
 * @param       LORA_JOIN_MODE mode:    OTAA or ABP
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_join_mode(RUI_LORA_JOIN_MODE mode);

/***************************************************************************************
 * @brief       This API is used to set the work mode of LoRa module.
 * @return      RUI_RETURN_STATUS
 * @param       LORA_WORK_MODE mode:    LaRaWAN, P2P, or Test mode.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_work_mode(RUI_LORA_WORK_MODE mode);

/***************************************************************************************
 * @brief       This API is used to set the interval time of sending data.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_LORA_AUTO_SEND_MODE mode: lora auto send mode, refer to RUI.
                uint16_t app_interval:  the interval time of sending data.(unit:s)
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_send_interval(RUI_LORA_AUTO_SEND_MODE mode,uint16_t interval_time);

/***************************************************************************************
 * @brief       This API is used to convert region from string to LORA_REGION enum.
 * @return      LORA_REGION value
 * @param       uint8_t *p_buf: the pointer of region string.
***************************************************************************************/
LORA_REGION rui_lora_region_convert(uint8_t *p_buf);

/***************************************************************************************
 * @brief       This API is used to set the region of LoRaWAN you want it to work in.
 * @return      RUI_RETURN_STATUS
 * @param       LORA_REGION region:     the region of LoRaWAN.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_region(LORA_REGION region);

/***************************************************************************************
 * @brief       This API is used to get all status about LoRa.
 * @return      RUI_RETURN_STATUS
 * @param       bool IsPrint:whether print parameters through serial
                RUI_LORA_STATUS_T *status:  the status about LoRa.
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_get_status(bool IsPrint,RUI_LORA_STATUS_T *status);

/***************************************************************************************
 * @brief       This API is used to print all channel list about LoRa.
 * @return      RUI_RETURN_STATUS
 * @param       NULL
***************************************************************************************/
RUI_RETURN_STATUS rui_get_channel_list(void);

#ifdef SOFTDEVICE_PRESENT
/***************************************************************************************
 * @brief       This API is used to set the work mode of BLE.
 * @return      RUI_RETURN_STATUS
 * @param       BLE_WORK_MODE mode:  BLE_MODE_PERIPHERAL, BLE_MODE_CENTRAL, BLE_MODE_OBSERVER
 * @param       long_range_enable: true or false
***************************************************************************************/
RUI_RETURN_STATUS rui_ble_set_work_mode(BLE_WORK_MODE mode, bool long_range_enable);

/***************************************************************************************
 * @brief       This API is used to get peripherial advertise report.
 * @return      NULL
 * @param       int8_t rssi_value:    peripheral rssi value.
                uint8_t *p_adv_data:  the pointer of advertise data.
                uint16_t adv_data_len:  the length of advertise data.
                uint8_t *p_device_mac:  the pointer of peripheral MAC.
***************************************************************************************/
void rui_ble_scan_adv(int8_t rssi_value, uint8_t *p_adv_data, uint16_t adv_data_len, uint8_t *p_device_mac);

/***************************************************************************************
 * @brief       This API is used to start advertising in ble peripheral mode.
                BLE broadcast will stop automatically after 60 seconds.
 * @return      NULL
 * @param       NULL
***************************************************************************************/
void rui_ble_advertising_start(void);

/***************************************************************************************
 * @brief       This API is used to register ble event callback functions.
 * @return      RUI_RETURN_STATUS
 * @param       ble_evt_connect:    the callback function for ble connected event.
                ble_evt_disconnect: the callback function for ble disconnected event.
***************************************************************************************/
typedef void (*ble_evt_connect)(void);
typedef void (*ble_evt_disconnect)(void);
RUI_RETURN_STATUS rui_ble_evt_register_callback(ble_evt_connect callback1, ble_evt_disconnect callback2);

/***************************************************************************************
 * @brief       This API is used to handle nofify data from ble peripheral.
 * @return      NULL
 * @param       uint8_t *pdata: the pointer of receive data.
                uint16_t len:   the lengh of receive data.
***************************************************************************************/
void rui_ble_rx_data_notify(uint8_t *pdata, uint16_t len);

/***************************************************************************************
 * @brief       This API is used to handle read operation data receive from ble peripheral.
 * @return      NULL
 * @param       uint8_t *pdata: the pointer of receive data.
                uint16_t len:   the lengh of receive data.
***************************************************************************************/
void rui_ble_rx_data_read(uint8_t *pdata, uint16_t len);

/***************************************************************************************
 * @brief       This API is used to write data to write_handle which in peripheral via ble.
 * @return      RUI_RETURN_STATUS
 * @param       BLE_CLIENT * p_ble_rcs_c:  ble_rcs_c instances which is m_rcs_c[i] in multiple link.
                uint8_t *pdata:  the pointer of transmit data.
                uint16_t len:  the lengh of transmit data.
***************************************************************************************/
RUI_RETURN_STATUS rui_ble_tx_data_write(BLE_CLIENT * p_ble_rcs_c, uint8_t *pdata, uint16_t len);

/***************************************************************************************
 * @brief       This API is used to write data to read_notify_handle which in peripheral via ble.
 * @return      RUI_RETURN_STATUS
 * @param       BLE_CLIENT * p_ble_rcs_c:  ble_rcs_c instances which is m_rcs_c[i] in multiple link.
***************************************************************************************/
RUI_RETURN_STATUS rui_ble_tx_data_read(BLE_CLIENT * p_ble_rcs_c);
#endif

/***************************************************************************************
 * @brief       This API is used to get the current firmware version.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *version:   the current firmware version.
***************************************************************************************/
RUI_RETURN_STATUS rui_device_version(uint8_t *version);

/***************************************************************************************
 * @brief       This API is used to reset the device.
 * @return      RUI_RETURN_STATUS
 * @param       void
***************************************************************************************/
RUI_RETURN_STATUS rui_device_reset(void);

/***************************************************************************************
 * @brief       This API is used to let the device go to sleep mode.
 * @return      NULL
 * @param       uint32_t on/off: on/off
                sensor_wakeup,sensor_sleep: app callback
***************************************************************************************/
typedef void (*sensor_wakeup)(void);
typedef void (*sensor_sleep)(void);
RUI_RETURN_STATUS rui_sensor_register_callback(sensor_wakeup callback1,sensor_sleep callback2);
RUI_RETURN_STATUS rui_device_sleep(uint32_t on);

/***************************************************************************************
 * @brief       This API is used to get the current voltage value of the battery.
 * @return      RUI_RETURN_STATUS
 * @param       float *voltage: the current voltage value of the battery
***************************************************************************************/
RUI_RETURN_STATUS rui_device_get_battery_level(float *voltage);

/***************************************************************************************
 * @brief       This API is used to config LoRaP2P parameters.
 * @return      RUI_RETURN_STATUS
 * @param       Frequency:Frequency in Hz,
                Spreadfact:Spreadfactare limite to the 6-12 range,
                Bandwidth:Bandwidth limite to the 0-2 range,
                Codingrate:Codingrate limite to the 1-4 range,
                Preamlen:Preamlen limite to the 2-66535 range,
                Powerdbm:Powerdbm limite to the 0-20 range.
***************************************************************************************/
RUI_RETURN_STATUS rui_lorap2p_config(uint32_t Frequency,uint8_t  Spreadfact,uint8_t  Bandwidth,uint8_t  Codingrate,uint16_t  Preamlen,uint8_t  Powerdbm);

/***************************************************************************************
 * @brief       This API is used to send data by LoRaP2P mode.
 * @return      RUI_RETURN_STATUS
 * @param       data: data package,  
                len:data size.
***************************************************************************************/
RUI_RETURN_STATUS rui_lorap2p_send(uint8_t* data,uint16_t len);

/***************************************************************************************
 * @brief       This API is used to register a callback function for LoRaP2P in application,
                so that application can receive the LoRa data automatically.
 * @return      RUI_RETURN_STATUS
 * @param       lora_receive callback:  the callback function for receiving LoRaP2P data.
***************************************************************************************/
typedef void (*lorap2p_receive)(RUI_LORAP2P_RECEIVE_T *data);
RUI_RETURN_STATUS rui_lorap2p_register_recv_callback(lorap2p_receive callback);

/***************************************************************************************
 * @brief       This API is used to register a callback function for LoRaP2P in application,
 * @return      RUI_RETURN_STATUS
 * @param       lorap2p_send callback:  the callback function for LoRaP2P send success.
***************************************************************************************/
typedef void (*lorap2p_send)(void);
RUI_RETURN_STATUS rui_lorap2p_complete_register_callback(lorap2p_send callback);

/***************************************************************************************
 * @brief       This API is used to register a callback function for LoRaWAN join,
                so that application can start LoRaWAN function.
 * @return      RUI_RETURN_STATUS
 * @param       lorajoin callback: the callback function for LoRaWAN join .
                status: 1 -> join succeed ,0 -> join fail.
***************************************************************************************/
typedef void (*lorajoin)(uint32_t status);
RUI_RETURN_STATUS rui_lorajoin_register_callback(lorajoin callback);

/***************************************************************************************
 * @brief       This API is used to register a callback function for LoRaWAN send complete,
                so that application can start LoRaWAN function.
 * @return      RUI_RETURN_STATUS
 * @param       lorasend callback: the callback function for LoRaWAN send complete.
                RUI_MCPS_T type: send packet type.
***************************************************************************************/
typedef void (*lorasend)(RUI_MCPS_T type,RUI_RETURN_STATUS  status);
RUI_RETURN_STATUS rui_lorasend_complete_register_callback(lorasend callback);

/***************************************************************************************
 * @brief       This API is used to set  lora sending power.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t power_value.<range:0~15 according thr region>
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_set_tx_power(uint8_t power_value);

/***************************************************************************************
 * @brief       This API is used to creat and init a timer.
 * @return      RUI_RETURN_STATUS
 * @param       void *obj:timer instance.
                void ( *callback )( void ):timer event callback function.
***************************************************************************************/
RUI_RETURN_STATUS rui_timer_init(void *obj, void ( *callback )( void ));

/***************************************************************************************
 * @brief       This API is used to uninit timer.
 * @return      RUI_RETURN_STATUS
 * @param       void *obj:timer instance.
***************************************************************************************/
RUI_RETURN_STATUS rui_timer_uninit(void *obj);

/***************************************************************************************
 * @brief       This API is used to set value for timer.
 * @return      RUI_RETURN_STATUS
 * @param       void *obj: timer instance.
                uint32_t value: timer value.
***************************************************************************************/
RUI_RETURN_STATUS rui_timer_setvalue(void *obj, uint32_t value );

/***************************************************************************************
 * @brief       This API is used to start timer.
 * @return      RUI_RETURN_STATUS
 * @param       void *obj: timer instance.
***************************************************************************************/
RUI_RETURN_STATUS rui_timer_start(void *obj);

/***************************************************************************************
 * @brief       This API is used to stop timer.
 * @return      RUI_RETURN_STATUS
 * @param       void *obj: timer instance.
***************************************************************************************/
RUI_RETURN_STATUS rui_timer_stop(void *obj);

/***************************************************************************************
 * @brief       This API is used to delay time (unit:us).
 * @return      RUI_RETURN_STATUS
 * @param       uint32_t value: delay time value.
***************************************************************************************/
RUI_RETURN_STATUS rui_delay_us( uint32_t value );

/***************************************************************************************
 * @brief       This API is used to delay time (unit:ms).
 * @return      RUI_RETURN_STATUS
 * @param       uint32_t value: delay time value.
***************************************************************************************/
RUI_RETURN_STATUS rui_delay_ms( uint32_t value );

/***************************************************************************************
 * @brief       This API is used to init system.
 * @return      none
 * @param       none
***************************************************************************************/
void rui_init(void);

/***************************************************************************************
 * @brief       This API is used to run preset tasks, including low-power processing.
 * @return      none
 * @param       none
***************************************************************************************/
void rui_running(void);

/***************************************************************************************
 * @brief       This API is auto send data timeout callback by lora.
 * @return      none
 * @param       none
***************************************************************************************/
void rui_lora_autosend_callback(void);

/***************************************************************************************
 * @brief       This API is flash write and read.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *str
                uint8_t len: should less than 128 byte
                RUI_FLASH_MODE mode: user data or origin data
***************************************************************************************/
RUI_RETURN_STATUS rui_flash_write(RUI_FLASH_MODE mode, uint8_t *str, uint8_t len);

/***************************************************************************************
 * @brief       This API is auto send data timeout callback by lora.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t *str
                uint8_t len: should less than 128 byte
                RUI_FLASH_MODE mode: user data or origin data
***************************************************************************************/
RUI_RETURN_STATUS rui_flash_read(RUI_FLASH_MODE mode,uint8_t *str, uint8_t len);

/***************************************************************************************
 * @brief       This API is used to return info for user defined AT,
                so that application can use their at.
 * @return      void
 * @param       bool is_success: true or false
                uint8_t *p_msg: user message
                uint16_t ret_code:user error code
***************************************************************************************/
void  rui_at_response(bool is_success, uint8_t *p_msg, uint16_t ret_code);

/***************************************************************************************
 * @brief       This API is used to regist external gpio, it may lead to power current 
                increase. e.g 130uA in nordic platform.
 * @return      RUI_RETURN_STATUS
 * @param       bool control: true or false, use it or not.
                RUI_GPIO_ST st: gpio struct
                edge: detect in raise or fall
                pro: priority for interrupt
                callback: interrupt callback.
***************************************************************************************/
typedef void (*interrupt_callback)(void);
RUI_RETURN_STATUS rui_gpio_interrupt(bool control, RUI_GPIO_ST st, RUI_GPIO_INTERRUPT_EDGE edge, RUI_GPIO_INTERRUPT_PRIORITY pro,interrupt_callback callback);

/***************************************************************************************
 * @brief       This API is used to get current data rate and payloadsize.
 * @return      RUI_RETURN_STATUS
 * @param       uint8_t* dr: current DR
                uint16_t* lengthM:Maximum acceptable size 
***************************************************************************************/
RUI_RETURN_STATUS rui_lora_get_dr(uint8_t* dr, uint16_t* lengthM);

/***************************************************************************************
 * @brief       This API is used to init PWM.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_PWM_ST* pwm_st: pwm structure pointer  
***************************************************************************************/
RUI_RETURN_STATUS rui_pwm_init(RUI_PWM_ST* pwm_st);

/***************************************************************************************
 * @brief       This API is used to start PWM.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_PWM_ST* pwm_st: pwm structure pointer 
***************************************************************************************/
RUI_RETURN_STATUS rui_pwm_start(RUI_PWM_ST* pwm_st);

/***************************************************************************************
 * @brief       This API is used to stop PWM.
 * @return      RUI_RETURN_STATUS
 * @param       RUI_PWM_ST* pwm_st: pwm structure pointer 
***************************************************************************************/
RUI_RETURN_STATUS rui_pwm_stop(RUI_PWM_ST* pwm_st);

#ifdef SOFTDEVICE_PRESENT
#define RUI_LOG_PRINTF(fmt, args...)  NRF_LOG_INFO(fmt, ##args)
#else
#define RUI_LOG_PRINTF  UartPrint
#endif



#endif
