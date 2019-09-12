#include "board.h"
#include "rui.h"

#define VREFINT_CAL       ( *( uint16_t* )0x1FF80078 )  //VREF calibration value
extern RUI_GPIO_ST Bat_level;
extern RUI_GPIO_ST Adc_vref;

uint32_t BoardBatteryMeasureVolage( float *voltage )
{
    uint16_t vcal = VREFINT_CAL;
    uint16_t vref = 0;
    uint16_t vdiv = 0;

	rui_adc_get(&Bat_level,&vdiv);
	rui_adc_get(&Adc_vref,&vref);

    *voltage = (float)3000 * vcal * vdiv / (float)( vref * 4096);
			
    //                                vDiv
    // Divider bridge  VBAT <-> 100k -<--|-->- 150k <-> GND => vBat = (5 * vDiv )/3
    *voltage = (5 * *voltage )/3;
    DUartPrint("vdiv= %d ,vref= %d ,Vcal=%d\r\n",vdiv, vref,vcal);

}