#include "main.h"
#include "Sensor.h"

extern ADC_HandleTypeDef hadc;


#define VREFINT_CAL_VOLTAGE (3.f)
#define VREFINT_CAL *((uint16_t*)0x1FF80078)


#define TSENSE_CAL_VOLTAGE (3.f)
#define TSENSE_CAL1 *((uint16_t*)0x1FF8007A)//30
#define TSENSE_CAL2 *((uint16_t*)0x1FF8007E)//130

float TSENSE_CAL1_V,TSENSE_CAL2_V;
float TempSlope,Tempslope_B;

void init_Sensor()
{
    TSENSE_CAL1_V = TSENSE_CAL1*TSENSE_CAL_VOLTAGE/0xFFF;// Temp/V
    TSENSE_CAL2_V = TSENSE_CAL2*TSENSE_CAL_VOLTAGE/0xFFF;

    TempSlope = (130-30)/(TSENSE_CAL2_V-TSENSE_CAL1_V);// Temp/V
    Tempslope_B = 30 - TempSlope*TSENSE_CAL1_V;
    // a(Vcal1) + b = 30 , b = 30-aVcal1
}

void ADC_Select_Channel(uint32_t Channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = Channel;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    if(HAL_ADC_ConfigChannel(&hadc,&sConfig) != HAL_OK)
        Error_Handler();
}

uint32_t getADCValue(uint32_t Channel)
{
    ADC_Select_Channel(Channel);
    // Get ADC value
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
    uint32_t ADCVal = HAL_ADC_GetValue(&hadc);
    HAL_ADC_Stop(&hadc);
    return ADCVal;
}


float getTemperature()
{
    uint16_t vtmpVal = getADCValue(ADC_CHANNEL_TEMPSENSOR);
    //uint16_t vrefVal = getADCValue(ADC_CHANNEL_VREFINT);

    float Vdd = 3.3;//VREFINT_CAL_VOLTAGE*VREFINT_CAL/vrefVal;
    
    float Vtemp = vtmpVal*Vdd/0xFFF;

    float Temperature = TempSlope*Vtemp + Tempslope_B;

    printf("%03X,%03X,%03X , %2.3fV,%2.3fV,%2.3fV mV/'C = %2.3fV  Temp : (%2.3f'C)\n" ,TSENSE_CAL1,vtmpVal,TSENSE_CAL2 ,TSENSE_CAL1_V,Vtemp,TSENSE_CAL2_V ,1000.f/TempSlope,Temperature);
    return 0.f;
}

