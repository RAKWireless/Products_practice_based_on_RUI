#include "app.h"
#include "lora_config.h"
#include "rui.h"
#include "at_cmd.h"

//join cnt
#define JOIN_MAX_CNT 6
static uint8_t JoinCnt=0;
bool IsTxDone = false;   //Entry sleep flag
RUI_DEVICE_STATUS_T app_device_status; //record device status 
uint8_t a[50]={};    // Data buffer to be sent  
static uint8_t process_cli = 0;  //AT command flag bit
MlmeReq_t mlmeReq;  //query LoRaMAC parameter
#define CLI_LENGTH_MAX  256
char cli_buffer[CLI_LENGTH_MAX]; //Parse serial port strings buffer
uint16_t pdata_index=0;          //UART string point

TimerEvent_t autosend_timer;  //auto send timer
volatile uint8_t autosend_flag = 0;    //auto send flag

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
    UartPrint("at+recv=%d,%d,%d,%d:", Receive_datapackage->Port, Receive_datapackage->Rssi, Receive_datapackage->Snr, Receive_datapackage->BufferSize);   
    
    if ((Receive_datapackage->Buffer != NULL) && Receive_datapackage->BufferSize) {
        for (int i = 0; i < Receive_datapackage->BufferSize; i++) {
            sprintf(hex_str, "%02x", Receive_datapackage->Buffer[i]);
            UartPrint("%s", hex_str); 
        }
    }
    UartPrint("\r\n");
}
void LoRaP2PReceive_callback(RUI_LORAP2P_RECEIVE_T *Receive_P2Pdatapackage)
{
    char hex_str[3]={0};
    UartPrint("at+recv=%d,%d,%d:", Receive_P2Pdatapackage -> Rssi, Receive_P2Pdatapackage -> Snr, Receive_P2Pdatapackage -> BufferSize); 
    for(int i=0;i < Receive_P2Pdatapackage -> BufferSize; i++)
    {
        sprintf(hex_str, "%02X", Receive_P2Pdatapackage -> Buffer[i]);
        UartPrint("%s",hex_str);
    }
    UartPrint("\r\n");    
}
void LoRaWANJoined_callback(uint32_t status)
{
    MibRequestConfirm_t getPhy;
    PhyParam_t phyParam; 
    static int8_t dr; 
    if(status)  //Join Success
    {
        JoinCnt = 0;
        UartPrint("[LoRa]:Joined Successed!\r\n");
        autosend_flag = 1;  //set autosend_flag after join succeed 
    }else 
    {        
        if(JoinCnt<JOIN_MAX_CNT) // Join was not successful. Try to join again
        {
            JoinCnt++;
            UartPrint("[LoRa]:Join retry Cnt:%d\n",JoinCnt);
            getPhy.Type = MIB_ADR;
            if(LoRaMacMibGetRequestConfirm(&getPhy) == LORAMAC_STATUS_OK)
            {
                if(getPhy.Param.AdrEnable)
                {
                    getPhy.Type = MIB_CHANNELS_DATARATE;
                    if(LoRaMacMibGetRequestConfirm(&getPhy) == LORAMAC_STATUS_OK)
                    {
                        dr = getPhy.Param.ChannelsDatarate;
                    }
                    if(dr > 0)
                    {
                        mlmeReq.Req.Join.Datarate = dr - 1;
                    }else mlmeReq.Req.Join.Datarate = 0;

                    LoRaMacMlmeRequest( &mlmeReq );
                }else 
                {
                    mlmeReq.Req.Join.Datarate = app_device_status.lora_dr;
                    LoRaMacMlmeRequest( &mlmeReq );
                }
            }           
        }
        else   //Join failed
        {
            UartPrint("[LoRa]:Joined Failed! \r\n"); 
            JoinCnt=0;
            IsTxDone=true;          
        }          
    }    
}
void LoRaWANSendsucceed_callback(RUI_MCPS_T status)
{
    switch( status )
    {
        case RUI_MCPS_UNCONFIRMED:
        {
            UartPrint("[LoRa]: Unconfirm data send OK\r\n");
            break;
        }
        case RUI_MCPS_CONFIRMED:
        {
            UartPrint("[LoRa]: Confirm data send OK\r\n");

            break;
        }
        case RUI_MCPS_PROPRIETARY:
        {
            UartPrint("[LoRa]: MCPS_PROPRIETARY\r\n");
            break;
        }
        case RUI_MCPS_MULTICAST:
        {
            UartPrint("[LoRa]: MCPS_PROPRIETARY\r\n");
            break;           
        }
        default:             
            break;
    }  

    rui_timer_start(&autosend_timer);  //start autosend_timer after send success
    IsTxDone=true; 
}

/*******************************************************************************************
 * UART string frames timer and event,user mustn't modify this funtion
 * *****************************************************************************************/  
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // UartPrint("HAL_TIM_PeriodElapsedCallback%s\r\n",cli_buffer);
    process_cli = 1;

}

/*******************************************************************************************
 * RUI UART interrupt callback: parameter len always be 1
 * *****************************************************************************************/ 
void rui_uart_data_recv(RUI_UART_DEF uart_def, uint8_t *pdata, uint16_t len)
{
    switch(uart_def)
    {
        case RUI_UART1: 
            if(pdata_index == CLI_LENGTH_MAX) 
            {
                process_cli=0;
                memset(cli_buffer,0,CLI_LENGTH_MAX);
                UartPrint("String over max length <256 Bytes>.\r\n");
                return;
            }            
            cli_buffer[pdata_index++]=*pdata;   //  UART1 receive data store in cli_buffer[];        
            TimerIdleStart();  //Listening to the frame  ,this timer is Special for frames listening           

            break;
        case RUI_UART3:
            /******************************************************
                * UART3 receive data callback code here
                *******************************************************/
            break;
        default:break;
    }
}

/*******************************************************************************************
 * Parse UART frames
 * *****************************************************************************************/ 
int Parse_string_loop(void)
{
    if(app_device_status.uart_mode == RUI_UART_NORAMAL)  //Uart work at normal mode
    {
        if(strncmp(cli_buffer,"at+",3) == 0)  //AT command handle code
        {       
            if ((strstr(cli_buffer,"\r") != NULL) || (strstr(cli_buffer,"\n") != NULL))
            { 
                *strchr(cli_buffer,'\n')='\0';
                *strchr(cli_buffer,'\r')='\0';
                at_cmd_process(cli_buffer);  //AT command function
            }else  //"AT format error"
            {
                /******************************************************
                * user process code
                *******************************************************/

            }
            
        }else //Not AT command frames
        {
            /******************************************************
             * user process code
            *******************************************************/
        } 
    }else  //Uart works at unvarnished transmit mode
    {
        /******************************************************
             * user process code
        *******************************************************/
       if(strcmp(cli_buffer,"+++") == 0)  //end unvarnished transmit mode
       {
           rui_uart_mode_config(RUI_UART1,RUI_UART_NORAMAL);
           rui_parameters_save();  //Save parameters to flash
           UartPrint("End unvarnished transmit mode,uart work mode switch to normal.\r\n");
       }else  //unvarnished transmit
       {
          rui_lora_send(8,cli_buffer,pdata_index); 
       }
    }
    process_cli=0;    
    pdata_index = 0;
    memset(cli_buffer,0,CLI_LENGTH_MAX);     
}


/*******************************************************************************************
 * the app_main function
 * *****************************************************************************************/ 
void autosend_event(void)
{
    autosend_flag = 1;
}


int main( void )
{
    static bool autosendtemp_status;
    static uint16_t lorasend_tempinterval;
    double P_PSI,Temperature;
    static uint8_t i=0;
    MibRequestConfirm_t getPhy; //LoRaWAN status check parameter

    // Borad Init 
    BoardInit();
    Pressure_init();  //Prssure Init

    //Register LoRaMac callback function
    rui_lora_register_recv_callback(LoRaReceive_callback);  
    rui_lorap2p_register_recv_callback(LoRaP2PReceive_callback);
    rui_lorajoin_register_callback(LoRaWANJoined_callback); 
    rui_lorasend_complete_register_callback(LoRaWANSendsucceed_callback); 

    //The query gets the current device status and Init status flag
    rui_device_get_status(&app_device_status);
    autosendtemp_status = app_device_status.autosend_status;
    lorasend_tempinterval = app_device_status.lorasend_interval; 

    //Init OK ,print board status and auto join LoRaWAN  
    switch(app_device_status.uart_mode)
    {
        case RUI_UART_NORAMAL: UartPrint("Initialization OK,AT Uart work mode:normal mode, "); 
            break;
        case RUI_UART_UNVARNISHED:UartPrint("Initialization OK,AT Uart work mode:unvarnished transmit mode, ");
            break;   
    }   
    switch(app_device_status.work_mode)
	{
		case LORAWAN:
            if(app_device_status.join_mode == RUI_OTAA)
            {
                switch(app_device_status.class_status)
                {
                    case RUI_CLASS_A:UartPrint("Current work_mode:LoRaWAN, join_mode:OTAA, Class: A\r\n");
                        break;
                    case RUI_CLASS_B:UartPrint("Current work_mode:LoRaWAN, join_mode:OTAA, Class: B\r\n");
                        break;
                    case RUI_CLASS_C:UartPrint("Current work_mode:LoRaWAN, join_mode:OTAA, Class: C\r\n");
                        break;
                    default:break;
                }                
                rui_lora_join();  //join LoRaWAN at OTAA mode
            }else if(app_device_status.join_mode == RUI_ABP)
            {
                switch(app_device_status.class_status)
                {
                    case RUI_CLASS_A:UartPrint("Current work_mode:LoRaWAN, join_mode:ABP, Class: A\r\n");
                        break;
                    case RUI_CLASS_B:UartPrint("Current work_mode:LoRaWAN, join_mode:ABP, Class: B\r\n");
                        break;
                    case RUI_CLASS_C:UartPrint("Current work_mode:LoRaWAN, join_mode:ABP, Class: C\r\n");
                        break;
                    default:break;
                }
                if(rui_lora_join() == 0)//join LoRaWAN at ABP mode
                {
                    LoRaWANJoined_callback(1);  //ABP mode join success
                }
            }
			break;
		case P2P:UartPrint("Current work_mode:P2P\r\n");
			break;
		case TEST:UartPrint("Current work_mode:TEST\r\n");
			break;
		default: break;
	} 

    rui_timer_init(&autosend_timer, autosend_event);
    rui_timer_setvalue(&autosend_timer,30000); //auto send interval set:30s

    //application instance
    while(1)
    {
        rui_device_get_status(&app_device_status);  //The query gets the current device status
        switch(app_device_status.work_mode)
        {
            case LORAWAN:
                if(process_cli == 1)
                {
                    Parse_string_loop();  //Parse serial port strings 
                }
                if(autosend_flag == 1)
                {
                    autosend_flag = 0;
                    Get_Pressure(&P_PSI,&Temperature);

                    a[i++]=0x02;
					a[i++]=0x67;
                    a[i++]=((uint16_t)(Temperature*10)>>8) & 0xff;
                    a[i++]=((uint16_t)(Temperature*10)) & 0xff;
                    a[i++]=0x03;
					a[i++]=0x02;
                    a[i++]=((uint16_t)(P_PSI*100)>>8) & 0xff;
                    a[i++]=((uint16_t)(P_PSI*100)) & 0xff;

                    getPhy.Type = MIB_NETWORK_JOINED;
                    if(LoRaMacMibGetRequestConfirm(&getPhy) == LORAMAC_STATUS_OK)  //check LoRaWAN net status
                    {                        
                        if(rui_lora_send(8,a,i) !=0)
                        {
                            UartPrint("[LoRa]: send Error\r\n");                            
                            if(app_device_status.autosend_status)rui_timer_start(&autosend_timer);  //restart autosend_timer if send error
                        }else UartPrint("[LoRa]: Send out\r\n");
                        i = 0;
                    }
                }
                if(IsTxDone)
                {
                    IsTxDone=false;    
                    rui_device_sleep(1);               
                }
                break;
            case P2P:
                if(process_cli == 1)Parse_string_loop();  //Parse serial port strings
                break;
            case TEST:
                break;
        }     
    }    
}

