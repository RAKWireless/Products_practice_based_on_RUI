#include "rui.h"
#include "board.h"

//join cnt
#define JOIN_MAX_CNT 6
static uint8_t JoinCnt=0;
static bool IsTxDone = false;   //Entry sleep flag
static RUI_DEVICE_STATUS_T app_device_status; //record device status 
static RUI_LORA_STATUS_T app_lora_status; //record lora status 

/*******************************************************************************************
 * The BSP user functions.
 * 
 * *****************************************************************************************/ 
#define LED_1                                   8
#define LED_2                                   9
#define BAT_LEVEL_CHANNEL                       20
#define ADC_VREF_CHANNEL                        0xFF

const uint8_t level[2]={0,1};
#define low     &level[0]
#define high    &level[1]
RUI_GPIO_ST Led_Blue;  //join LoRaWAN successed indicator light
RUI_GPIO_ST Led_Green;  //send data successed indicator light 
RUI_GPIO_ST Bat_level;
RUI_GPIO_ST Adc_vref;
RUI_I2C_ST I2c_1;
TimerEvent_t Led_Green_Timer;
TimerEvent_t Led_Blue_Timer;  //LoRa send out indicator light
volatile static bool autosend_flag = false;    //auto send flag
static uint8_t a[80]={};    // Data buffer to be sent by lora


void rui_lora_autosend_callback(void)  //auto_send timeout event callback
{
    autosend_flag = true;
}

void OnLed_Green_TimerEvent(void)
{
    rui_timer_stop(&Led_Green_Timer);
    rui_gpio_rw(RUI_IF_WRITE,&Led_Green, high);

    rui_device_get_status(&app_device_status);//The query gets the current device status 
    if(app_device_status.autosend_status)autosend_flag = true;  //set autosend_flag after join LoRaWAN succeeded 
}

void OnLed_Blue_TimerEvent(void)
{
    rui_timer_stop(&Led_Blue_Timer);
    rui_gpio_rw(RUI_IF_WRITE,&Led_Blue, high);

    rui_device_get_status(&app_device_status);//The query gets the current device status 
    if(app_device_status.autosend_status)
    {
        IsTxDone=true;  //Sleep flag set true
    }
}
void bsp_led_init(void)
{
    Led_Green.pin_num = LED_1;
    Led_Blue.pin_num = LED_2;
    Led_Green.dir = RUI_GPIO_PIN_DIR_OUTPUT;
    Led_Blue.dir = RUI_GPIO_PIN_DIR_OUTPUT;
    Led_Green.pull = RUI_GPIO_PIN_NOPULL;
    Led_Blue.pull = RUI_GPIO_PIN_NOPULL;

    rui_gpio_init(&Led_Green);
    rui_gpio_init(&Led_Blue);
    rui_gpio_rw(RUI_IF_WRITE,&Led_Green,low);
    rui_gpio_rw(RUI_IF_WRITE,&Led_Blue,low);
    rui_delay_ms(200);
    rui_gpio_rw(RUI_IF_WRITE,&Led_Green,high);
    rui_gpio_rw(RUI_IF_WRITE,&Led_Blue,high);
    rui_timer_init(&Led_Green_Timer,OnLed_Green_TimerEvent);
    rui_timer_init(&Led_Blue_Timer,OnLed_Blue_TimerEvent);
    rui_timer_setvalue(&Led_Green_Timer,100);
    rui_timer_setvalue(&Led_Blue_Timer,100);
}
void bsp_adc_init(void)
{
    Bat_level.pin_num = BAT_LEVEL_CHANNEL;
    Bat_level.dir = RUI_GPIO_PIN_DIR_INPUT;
    Bat_level.pull = RUI_GPIO_PIN_NOPULL;
    rui_adc_init(&Bat_level);

    Adc_vref.pin_num = ADC_VREF_CHANNEL;
    rui_adc_init(&Adc_vref);
}
void bsp_i2c_init(void)
{
    I2c_1.INSTANCE_ID = 1;
    I2c_1.PIN_SDA = I2C_SDA;
    I2c_1.PIN_SCL = I2C_SCL;
    I2c_1.FREQUENCY = RUI_I2C_FREQ_400K;

    rui_i2c_init(&I2c_1);

}
void bsp_init(void)
{
    bsp_led_init();
    bsp_adc_init();
    bsp_i2c_init();
    BME680_Init();
}

void app_loop(void)
{
    float temp_f=0;
    uint32_t humidity;
    int16_t temperature;
    uint32_t pressure;
    int temp=0; 
    int x,y,z;
    float x0,y0,z0;
    static uint8_t i=0;
    rui_lora_get_status(&app_lora_status);
    if(app_lora_status.IsJoined)
    {
        if (autosend_flag) 
        {
            rui_delay_ms(50);
            BoardBatteryMeasureVolage(&temp_f);
            temp_f=temp_f/1000.0;   //convert mV to V
            RUI_LOG_PRINTF("Battery Voltage = %d.%d V \r\n",(uint32_t)(temp_f), (uint32_t)((temp_f)*1000-((int32_t)(temp_f)) * 1000));
            temp=(uint16_t)round(temp_f*100.0);
            a[i++]=0x08;
            a[i++]=0x02;
            a[i++]=(temp&0xffff) >> 8;
            a[i++]=temp&0xff;				

            if(BME680_get_data(&humidity,&temperature,&pressure)==0)
            {
                a[i++]=0x07;
                a[i++]=0x68;
                a[i++]=( humidity / 500 ) & 0xFF;
					
                a[i++]=0x06;
                a[i++]=0x73;
                a[i++]=(( pressure / 10 ) >> 8 ) & 0xFF;
                a[i++]=(pressure / 10 ) & 0xFF;
			
                a[i++]=0x02;
                a[i++]=0x67;
                a[i++]=(( temperature / 10 ) >> 8 ) & 0xFF;
                a[i++]=(temperature / 10 ) & 0xFF;
            }

	        if(i != 0)
            {                    
                autosend_flag=false; 
                if(rui_lora_send(8,a,i) !=0)
                {
                    RUI_LOG_PRINTF("[LoRa]: send error\r\n");                            
                    if(app_device_status.autosend_status)
                    {
                        rui_lora_get_status(&app_lora_status);  //The query gets the current lora status 
                        rui_lora_set_send_interval(1,app_lora_status.lorasend_interval);  //start autosend_timer after send success

                    }
                }else RUI_LOG_PRINTF("[LoRa]: send out\r\n"); 
                i=0;                       
            }	
            else 
            {
                RUI_LOG_PRINTF("No Sensor data detect.\n");  
                if(app_device_status.autosend_status)
                {
                    autosend_flag=false; 
                    rui_lora_get_status(&app_lora_status);  //The query gets the current lora status 
                    rui_lora_set_send_interval(1,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                    IsTxDone=true;  //Sleep flag set true
                }                        
            }                					
        }
    }	
}

/*******************************************************************************************
 * LoRaMac callback functions
 * * void LoRaReceive_callback(RUI_RECEIVE_T* Receive_datapackage);//LoRaWAN callback if receive data 
 * * void LoRaP2PReceive_callback(RUI_LORAP2P_RECEIVE_T *Receive_P2Pdatapackage);//LoRaP2P callback if receive data 
 * * void LoRaWANJoined_callback(uint32_t status);//LoRaWAN callback after join server request
 * * void LoRaWANSendsucceed_callback(RUI_MCPS_T status);//LoRaWAN call back after send data complete
 * *****************************************************************************************/  
void LoRaReceive_callback(RUI_RECEIVE_T* Receive_datapackage)
{
    char hex_str[3] = {0}; 
    RUI_LOG_PRINTF("at+recv=%d,%d,%d,%d:", Receive_datapackage->Port, Receive_datapackage->Rssi, Receive_datapackage->Snr, Receive_datapackage->BufferSize);   
    
    if ((Receive_datapackage->Buffer != NULL) && Receive_datapackage->BufferSize) {
        for (int i = 0; i < Receive_datapackage->BufferSize; i++) {
            sprintf(hex_str, "%02x", Receive_datapackage->Buffer[i]);
            RUI_LOG_PRINTF("%s", hex_str); 
        }
    }
    RUI_LOG_PRINTF("\r\n");
}
void LoRaP2PReceive_callback(RUI_LORAP2P_RECEIVE_T *Receive_P2Pdatapackage)
{
    char hex_str[3]={0};
    RUI_LOG_PRINTF("at+recv=%d,%d,%d:", Receive_P2Pdatapackage -> Rssi, Receive_P2Pdatapackage -> Snr, Receive_P2Pdatapackage -> BufferSize); 
    for(int i=0;i < Receive_P2Pdatapackage -> BufferSize; i++)
    {
        sprintf(hex_str, "%02X", Receive_P2Pdatapackage -> Buffer[i]);
        RUI_LOG_PRINTF("%s",hex_str);
    }
    RUI_LOG_PRINTF("\r\n");    
}
void LoRaWANJoined_callback(uint32_t status)
{
    static int8_t dr; 
    if(status)  //Join Success
    {
        JoinCnt = 0;
        RUI_LOG_PRINTF("[LoRa]:Joined Successed!\r\n");
        rui_gpio_rw(RUI_IF_WRITE,&Led_Green, low);
        rui_timer_start(&Led_Green_Timer);        
    }else 
    {        
        if(JoinCnt<JOIN_MAX_CNT) // Join was not successful. Try to join again
        {
            JoinCnt++;
            RUI_LOG_PRINTF("[LoRa]:Join retry Cnt:%d\n",JoinCnt);
            rui_lora_get_status(&app_lora_status);
            if(app_lora_status.lora_dr > 0)
            {
                app_lora_status.lora_dr -= 1;
            }else app_lora_status.lora_dr = 0;
            rui_lora_set_dr(app_lora_status.lora_dr);
            rui_lora_join();                    
        }
        else   //Join failed
        {
            RUI_LOG_PRINTF("[LoRa]:Joined Failed! \r\n"); 
            JoinCnt=0;
            IsTxDone=true;  //Sleep flag set true       
        }          
    }    
}
void LoRaWANSendsucceed_callback(RUI_MCPS_T status)
{
    switch( status )
    {
        case RUI_MCPS_UNCONFIRMED:
        {
            RUI_LOG_PRINTF("[LoRa]: Unconfirm data send OK\r\n");
            break;
        }
        case RUI_MCPS_CONFIRMED:
        {
            RUI_LOG_PRINTF("[LoRa]: Confirm data send OK\r\n");

            break;
        }
        case RUI_MCPS_PROPRIETARY:
        {
            RUI_LOG_PRINTF("[LoRa]: MCPS_PROPRIETARY\r\n");
            break;
        }
        case RUI_MCPS_MULTICAST:
        {
            RUI_LOG_PRINTF("[LoRa]: MCPS_PROPRIETARY\r\n");
            break;           
        }
        default:             
            break;
    }     
    rui_lora_get_status(&app_lora_status);  //The query gets the current lora status 
    rui_lora_set_send_interval(1,app_lora_status.lorasend_interval);  //start autosend_timer after send success
    rui_gpio_rw(RUI_IF_WRITE,&Led_Blue, low);
    rui_timer_start(&Led_Blue_Timer);  
}

/*******************************************************************************************
 * The RUI is used to receive data from uart.
 * 
 * *****************************************************************************************/ 
void rui_uart_recv(RUI_UART_DEF uart_def, uint8_t *pdata, uint16_t len)
{
    switch(uart_def)
    {
        case RUI_UART1:
            rui_lora_send(8,pdata,len);  //process code if RUI_UART1 work at RUI_UART_UNVARNISHED
            break;
        case RUI_UART3:
            break;
        default:break;
    }
}

/*******************************************************************************************
 * the app_main function
 * *****************************************************************************************/ 
void main(void)
{
    static bool autosendtemp_status;

    rui_init();
    bsp_init();

/*******************************************************************************************
 * Register LoRaMac callback function
 * 
 * *****************************************************************************************/
    rui_lora_register_recv_callback(LoRaReceive_callback);  
    rui_lorap2p_register_recv_callback(LoRaP2PReceive_callback);
    rui_lorajoin_register_callback(LoRaWANJoined_callback); 
    rui_lorasend_complete_register_callback(LoRaWANSendsucceed_callback); 


/*******************************************************************************************    
 *The query gets the current device and lora status 
 * 
 * *****************************************************************************************/    
    rui_device_get_status(&app_device_status);
    rui_lora_get_status(&app_lora_status);
	
	RUI_LOG_PRINTF("autosend_interval: %us\r\n", app_lora_status.lorasend_interval);
/*******************************************************************************************    
 *Init OK ,print board status and auto join LoRaWAN
 * 
 * *****************************************************************************************/  
    switch(app_device_status.uart_mode)
    {
        case RUI_UART_NORAMAL: RUI_LOG_PRINTF("Initialization OK,AT Uart work mode:normal mode, "); 
            break;
        case RUI_UART_UNVARNISHED:RUI_LOG_PRINTF("Initialization OK,AT Uart work mode:unvarnished transmit mode, ");
            break;   
    }   

    switch(app_lora_status.work_mode)
	{
		case RUI_LORAWAN:
            if(app_lora_status.join_mode == RUI_OTAA)
            {
                switch(app_lora_status.class_status)
                {
                    case RUI_CLASS_A:RUI_LOG_PRINTF("Current work_mode:LoRaWAN, join_mode:OTAA, Class: A\r\n");
                        break;
                    case RUI_CLASS_B:RUI_LOG_PRINTF("Current work_mode:LoRaWAN, join_mode:OTAA, Class: B\r\n");
                        break;
                    case RUI_CLASS_C:RUI_LOG_PRINTF("Current work_mode:LoRaWAN, join_mode:OTAA, Class: C\r\n");
                        break;
                    default:break;
                }                
                rui_lora_join();  //join LoRaWAN by OTAA mode
            }else if(app_lora_status.join_mode == RUI_ABP)
            {
                switch(app_lora_status.class_status)
                {
                    case RUI_CLASS_A:RUI_LOG_PRINTF("Current work_mode:LoRaWAN, join_mode:ABP, Class: A\r\n");
                        break;
                    case RUI_CLASS_B:RUI_LOG_PRINTF("Current work_mode:LoRaWAN, join_mode:ABP, Class: B\r\n");
                        break;
                    case RUI_CLASS_C:RUI_LOG_PRINTF("Current work_mode:LoRaWAN, join_mode:ABP, Class: C\r\n");
                        break;
                    default:break;
                }
                if(rui_lora_join() == 0)//join LoRaWAN by ABP mode
                {
                    LoRaWANJoined_callback(1);  //ABP mode join success
                }
            }
			break;
		case RUI_P2P:RUI_LOG_PRINTF("Current work_mode:P2P\r\n");
			break;
		default: break;
	}   
  
    while(1)
    {       
        rui_device_get_status(&app_device_status);//The query gets the current device status 
        rui_lora_get_status(&app_lora_status);//The query gets the current lora status 
        rui_running();
        switch(app_lora_status.work_mode)
        {
            case RUI_LORAWAN:
                if(autosendtemp_status != app_device_status.autosend_status) 
                {
                    autosendtemp_status = app_device_status.autosend_status;
                    if(autosendtemp_status == false)
                    {
                        autosendtemp_status = app_device_status.autosend_status;
                        rui_lora_set_send_interval(0,0);  //stop auto send data 
                        autosend_flag=false;
                    }else
                    {
                        autosend_flag=true;    
                    }          
                }

                if(IsTxDone)
                {
                    IsTxDone=false;   
                    rui_device_sleep(1);               
                }  

                app_loop();    

                break;
            case RUI_P2P:
                /*************************************************************************************
                 * user code at LoRa P2P mode
                *************************************************************************************/
                break;
            default :break;
        }
    }
}