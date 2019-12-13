#include "rui.h"

#ifndef __RUI_LOG_PRINT_MERGE
#define __RUI_LOG_PRINT_MERGE
#define RUI_LOG_PRINTF_MERGE(fmt, args...);  {uart_log_printf(fmt, ##args);RUI_LOG_PRINTF(fmt, ##args);}
#endif


#define JOIN_MAX_CNT 6
uint8_t JoinCnt = 0;
bool IsJoiningflag= false;  //flag whether joining or not status
RUI_LORA_STATUS_T app_lora_status; //record status 
RUI_RETURN_STATUS rui_return_status;
bool autosend_flag = false;
static uint8_t lora_data[80]={};    // Data buffer to be sent by lora
static uint16_t lora_data_len=0;

/* The following variables are temporarily used */
extern uint8_t g_lora_join_success;


void rui_lora_autosend_callback(void)
{
	autosend_flag = true;
    IsJoiningflag = false;
}

void app_loop(void)
{
    rui_lora_get_status(false,&app_lora_status);
    if(app_lora_status.IsJoined)  //if LoRaWAN is joined
    {
        if(autosend_flag)
        {
            autosend_flag = false;
            rui_delay_ms(5);

            /* 
                User data example  
                                    */
            // lora_data[0] = 0x01;
            // lora_data[1] = 0x02;
            // lora_data[2] = 0x03;
            // lora_data[3] = 0x04;
            // lora_data_len = 4;

            if(lora_data_len != 0)
            {                    
                RUI_LOG_PRINTF_MERGE("\r\n");
                rui_return_status = rui_lora_send(8, lora_data, lora_data_len);
                switch(rui_return_status)
                {
                    case RUI_STATUS_OK:RUI_LOG_PRINTF_MERGE("[LoRa]: send out\r\n");
                        break;
                    default: RUI_LOG_PRINTF_MERGE("[LoRa]: send error %d\r\n",rui_return_status);
                        rui_lora_get_status(false,&app_lora_status); 
                        switch(app_lora_status.autosend_status)
                        {
                            case RUI_AUTO_ENABLE_SLEEP:rui_lora_set_send_interval(RUI_AUTO_ENABLE_SLEEP,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                                break;
                            case RUI_AUTO_ENABLE_NORMAL:rui_lora_set_send_interval(RUI_AUTO_ENABLE_NORMAL,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                                break;
                            default:break;
                        }
						break;
                }                 
                lora_data_len=0;                       
            }	
            else 
            {                
                rui_lora_set_send_interval(RUI_AUTO_DISABLE,0);  //stop it auto send data if no sensor data.
            }
        }
    }
    else if(IsJoiningflag == false)
    {
        IsJoiningflag = true;
        if(app_lora_status.join_mode == RUI_OTAA)
        {
            rui_return_status = rui_lora_join();
            switch(rui_return_status)
            {
                case RUI_STATUS_OK:
                    RUI_LOG_PRINTF_MERGE("OTAA Join Start...\r\n");
                    break;
                case RUI_LORA_STATUS_PARAMETER_INVALID:RUI_LOG_PRINTF_MERGE("ERROR: RUI_AT_PARAMETER_INVALID %d\r\n",RUI_AT_PARAMETER_INVALID);
                    rui_lora_get_status(false,&app_lora_status);  //The query gets the current status 
                    switch(app_lora_status.autosend_status)
                    {
                        case RUI_AUTO_ENABLE_SLEEP:rui_lora_set_send_interval(RUI_AUTO_ENABLE_SLEEP,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                            break;
                        case RUI_AUTO_ENABLE_NORMAL:rui_lora_set_send_interval(RUI_AUTO_ENABLE_NORMAL,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                            break;
                        default:break;
                    } 
                    break;
                default: RUI_LOG_PRINTF_MERGE("ERROR: LORA_STATUS_ERROR %d\r\n",rui_return_status);
                    rui_lora_get_status(false,&app_lora_status); 
                    switch(app_lora_status.autosend_status)
                    {
                        case RUI_AUTO_ENABLE_SLEEP:rui_lora_set_send_interval(RUI_AUTO_ENABLE_SLEEP,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                            break;
                        case RUI_AUTO_ENABLE_NORMAL:rui_lora_set_send_interval(RUI_AUTO_ENABLE_NORMAL,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                            break;
                        default:break;
                    }
                    break;
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
    RUI_LOG_PRINTF_MERGE("at+recv=%d,%d,%d,%d", Receive_datapackage->Port, Receive_datapackage->Rssi, Receive_datapackage->Snr, Receive_datapackage->BufferSize);   
    
    if ((Receive_datapackage->Buffer != NULL) && Receive_datapackage->BufferSize) {
        RUI_LOG_PRINTF_MERGE(":");
        for (int i = 0; i < Receive_datapackage->BufferSize; i++) {
            sprintf(hex_str, "%02x", Receive_datapackage->Buffer[i]);
            RUI_LOG_PRINTF_MERGE("%s", hex_str); 
        }
    }
    RUI_LOG_PRINTF_MERGE("\r\n");
}

void LoRaWANJoined_callback(uint32_t status)
{
    static int8_t dr; 
    if(status)  //Join Success
    {
        JoinCnt = 0;
        IsJoiningflag = false;
        g_lora_join_success = 1;
        RUI_LOG_PRINTF_MERGE("[LoRa]:Join Success\r\nOK\r\n");
        rui_lora_get_status(false,&app_lora_status);
        if(app_lora_status.autosend_status != RUI_AUTO_DISABLE)
        {
            autosend_flag = true;  //set autosend_flag after join LoRaWAN succeeded 
        }       
    }else 
    {        
        if(JoinCnt<JOIN_MAX_CNT) // Join was not successful. Try to join again
        {
            JoinCnt++;
            RUI_LOG_PRINTF_MERGE("[LoRa]:Join retry Cnt:%d\r\n",JoinCnt);
            rui_lora_get_status(false,&app_lora_status);
            if(app_lora_status.lora_dr > 0)
            {
                app_lora_status.lora_dr -= 1;
            }else app_lora_status.lora_dr = 0;
            rui_lora_set_dr(app_lora_status.lora_dr);
            rui_lora_join();                    
        }
        else   //Join failed
        {
            RUI_LOG_PRINTF_MERGE("ERROR: RUI_AT_LORA_INFO_STATUS_JOIN_FAIL %d\r\n",RUI_AT_LORA_INFO_STATUS_JOIN_FAIL); 
			rui_lora_get_status(false,&app_lora_status); 
            switch(app_lora_status.autosend_status)
            {
                case RUI_AUTO_ENABLE_SLEEP:rui_lora_set_send_interval(RUI_AUTO_ENABLE_SLEEP,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                    break;
                case RUI_AUTO_ENABLE_NORMAL:rui_lora_set_send_interval(RUI_AUTO_ENABLE_NORMAL,app_lora_status.lorasend_interval);  //start autosend_timer after send success
                    break;
                default:break;
            }
            JoinCnt=0;   
        }          
    }    
}

void LoRaWANSendsucceed_callback(RUI_MCPS_T mcps_type,RUI_RETURN_STATUS status)
{
    if(status == RUI_STATUS_OK)
    {
        switch( mcps_type )
        {
            case RUI_MCPS_UNCONFIRMED:
            {
                RUI_LOG_PRINTF_MERGE("[LoRa]: RUI_MCPS_UNCONFIRMED send success\r\nOK\r\n");
                break;
            }
            case RUI_MCPS_CONFIRMED:
            {
                RUI_LOG_PRINTF_MERGE("[LoRa]: RUI_MCPS_CONFIRMED send success\r\nOK\r\n");
                break;
            }
            case RUI_MCPS_PROPRIETARY:
            {
                RUI_LOG_PRINTF_MERGE("[LoRa]: RUI_MCPS_PROPRIETARY send success\r\nOK\r\n");
                break;
            }
            case RUI_MCPS_MULTICAST:
            {
                RUI_LOG_PRINTF_MERGE("[LoRa]: RUI_MCPS_MULTICAST send success\r\nOK\r\n");
                break;           
            }
            default:             
            break;
        } 
    }
    else
    {
        RUI_LOG_PRINTF_MERGE("ERROR: RUI_RETURN_STATUS %d\r\n",status); 
    }

    rui_lora_get_status(false,&app_lora_status);//The query gets the current status 
    switch(app_lora_status.autosend_status)
    {
        case RUI_AUTO_ENABLE_SLEEP:rui_lora_set_send_interval(RUI_AUTO_ENABLE_SLEEP,app_lora_status.lorasend_interval);  //start autosend_timer after send success
            rui_delay_ms(5);  
            break;
        case RUI_AUTO_ENABLE_NORMAL:rui_lora_set_send_interval(RUI_AUTO_ENABLE_NORMAL,app_lora_status.lorasend_interval);  //start autosend_timer after send success
            break;
        default:break;
    } 
}

/*******************************************************************************************
 * The RUI is used to receive data from uart.
 * 
 * *****************************************************************************************/ 
void rui_uart_recv(RUI_UART_DEF uart_def, uint8_t *pdata, uint16_t len)
{
    switch(uart_def)
    {
        case RUI_UART1://process code if RUI_UART1 work at RUI_UART_UNVARNISHED
            break;

        default:break;
    }
}

/*  the function will run before sleep, 
    user can add code to make sensor into low power mode */
void user_sensor_sleep(void)
{
    // ...
}

/*  the function will run after wake up, 
    user can add code to wake up and init sensor module. */
void user_sensor_wakeup(void)
{
    // ...
}

void main(void)
{
    rui_init();

    /*******************************************************************************************
     * Register LoRaMac callback function
     * 
     * *****************************************************************************************/
    rui_lora_register_recv_callback(LoRaReceive_callback);  
    rui_lorajoin_register_callback(LoRaWANJoined_callback); 
    rui_lorasend_complete_register_callback(LoRaWANSendsucceed_callback); 

    /*******************************************************************************************
     * Register Sleep and Wakeup callback function
     * 
     * *****************************************************************************************/
    rui_sensor_register_callback(user_sensor_wakeup, user_sensor_sleep);

    /*******************************************************************************************    
     *The query gets the current status 
    * 
    * *****************************************************************************************/ 
    rui_lora_get_status(false,&app_lora_status);

	if(app_lora_status.autosend_status)
        RUI_LOG_PRINTF_MERGE("autosend_interval: %us\r\n", app_lora_status.lorasend_interval);

    /*******************************************************************************************    
     *Init OK ,print board status and auto join LoRaWAN
    * 
    * *****************************************************************************************/  
    switch(app_lora_status.work_mode)
	{
		case RUI_LORAWAN:
            RUI_LOG_PRINTF_MERGE("Initialization OK,Current work_mode:LoRaWAN,");
            if(app_lora_status.join_mode == RUI_OTAA)
            {
                RUI_LOG_PRINTF_MERGE(" join_mode:OTAA,");  
                switch(app_lora_status.class_status)
                {
                    case RUI_CLASS_A:RUI_LOG_PRINTF_MERGE(" Class: A\r\n");
                        break;
                    case RUI_CLASS_B:RUI_LOG_PRINTF_MERGE(" Class: B\r\n");
                        break;
                    case RUI_CLASS_C:RUI_LOG_PRINTF_MERGE(" Class: C\r\n");
                        break;
                    default:break;
                }              
            }else if(app_lora_status.join_mode == RUI_ABP)
            {
                RUI_LOG_PRINTF_MERGE(" join_mode:ABP,\r\n");
                switch(app_lora_status.class_status)
                {
                    case RUI_CLASS_A:RUI_LOG_PRINTF_MERGE(" Class: A\r\n");
                        break;
                    case RUI_CLASS_B:RUI_LOG_PRINTF_MERGE(" Class: B\r\n");
                        break;
                    case RUI_CLASS_C:RUI_LOG_PRINTF_MERGE(" Class: C\r\n");
                        break;
                    default:break;
                } 
                if(rui_lora_join() == RUI_STATUS_OK)//join LoRaWAN by ABP mode
                {
                    LoRaWANJoined_callback(1);  //ABP mode join success
                }
            }
			break;
		default: break;
	}   
    RUI_LOG_PRINTF_MERGE("\r\n");
    rui_delay_ms(100);

    while(1)
    {
        //here run system work and do not modify
        rui_running();

        //do your work here, then call rui_device_sleep(1) to sleep
        rui_lora_get_status(false,&app_lora_status);//The query gets the current status 
        switch(app_lora_status.work_mode)
        {
            case RUI_LORAWAN:
                app_loop(); 
                break;

            default :break;
        }

    }
}

