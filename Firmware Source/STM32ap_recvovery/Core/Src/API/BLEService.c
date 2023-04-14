#include "main.h"
#include "ETH/Internet.h"
#include "memory/memory.h"
#include "API/AttenderAPI.h"
#include "API/BLEService.h"

extern SPI_HandleTypeDef hspi1;
extern SavedData saveData;
extern SensorList sensorlist[SENSORLIST_LEN];

// reads the BLE val
// and processes the sensorlist.

void FetchBLEData()
{
    //READ the Pin_READY. (0:waiting 1:free)
    if(HAL_GPIO_ReadPin(BLE_READY_GPIO_Port,BLE_READY_Pin) == GPIO_PIN_RESET)
    {
        
        //uint8_t txbuffer[8] = "this.TX";
        BLEInfo rxbuffer[1] = {0,};

        //NRF로부터 데이터를 가져온다. (SPI 통신으로 배열을 수신함)
        HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_RESET);
        HAL_SPI_Receive(&hspi1, rxbuffer, sizeof(rxbuffer), 1000);
        HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_SET);
        //데이터가 확실하면(00 혹은 FF가 아니면)
        if(rxbuffer[0].AESData[0] != 0 && rxbuffer[0].AESData[1] != 0xFF)
        {
            // BLE Log #1
            // BLE 데이터의 번호, 데이터, 신호세기, 전송PW 등 비콘 프레임의 정보를 받는다.
            //Debug("LE%d(%d)\n",rxbuffer[0].AESNo,rxbuffer[0].rssi);
            // Debug("AESNo : %d\3",rxbuffer[0].AESNo);
            // Debug("rssi  : %d\3",rxbuffer[0].rssi);
            // Debug("txPWR : %d\3",rxbuffer[0].txPower);

            // SPI Log #1 이런 식으로 데이터가 쌓인다.
            // Debug("SPI SEND : %s\3",txbuffer);
            // uint8_t* pointer = rxbuffer;
            // Debug("{%02X %02X %02X %02X %02X %02X %02X %02X}\3"
            //   ,pointer[0],pointer[1],pointer[2],pointer[3]
            //   ,pointer[4],pointer[5],pointer[6],pointer[7]);
            
            // 센서 정보 자료형 SensorData를 정의하여 BLE 프레임을 처리한다.
            SensorData sensor;
            Result res = BLEList_ProcessSensor(rxbuffer,&sensor);
            //Debug("BLEList_ProcessSensor %s \n",(res==SUCCEED?"Succeed":"failed"));
            if(res==SUCCEED)
            {
                BLESensorRead(sensor);//리스트 관리부로 데이터를 넘긴다. 성공했을 때만.
            }
            
        }
    }
    
}
void SensorListProcess()
{
    //else Debug("empty line..\n");
    
    static uint32_t sensorlistCheckTimer;

    if(HAL_GetTick() - sensorlistCheckTimer > 100)
    {
        sensorlistCheckTimer = HAL_GetTick();
        volatile Result res;
        for(int i=0;i<SENSORLIST_LEN;i++)
        {
            if (sensorlist[i].idxUser != 0)
            {
                //DETECT re-entering but fluctuating(which will not be applied)
                if(sensorlist[i].ioState == 1 && //OUT
                    (
                        (HAL_GetTick() - sensorlist[i].beginTime) <= saveData.BeaconTimeout*1000// it scanned the sensor in the range of time.
                        && sensorlist[i].distance < saveData.dist
                        && (HAL_GetTick() - sensorlist[i].exitTime) <= saveData.resnd/*(m)*/*60/*(s)*/*1000
                      || 
                        sensorlist[i].needtobeSended == TRUE
                    )
                    )
                {
                    //printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                    //printf("Sensor %d[%d] re-entered!\n",i,sensorlist[i].idxUser);
                    printf("<%d>",sensorlist[i].idxUser);
                    sensorlist[i].ioState = 0;//IN
                    sensorlist[i].needtobeSended = FALSE;
                }

                //DETECT ENTERING
                //investigate if It's entering range(0,saveData.dist) && iostate:out (+cooling time:saveData.BeaconTimeout is greater > exittime)
                else if(sensorlist[i].ioState == 1 && 
                    (
                        (HAL_GetTick() - sensorlist[i].beginTime) <= saveData.BeaconTimeout*1000// it scanned the sensor in the range of time.
                        && sensorlist[i].distance < saveData.dist
                        && ( (HAL_GetTick() - sensorlist[i].exitTime) >= saveData.BeaconTimeout*1000 || (HAL_GetTick() < sensorlist[i].exitTime)/*prep for grabage value*/ )
                      || 
                        sensorlist[i].needtobeSended == TRUE
                    )
                    )
                {
                    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,SET);
                    sensorlist[i].needtobeSended = TRUE;
                    printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                    SensorData sensor = SensorList2Data(&sensorlist[i]);
                    
                    res = SUCCEED;
                    sensor.sensorDate = sensorlist[i].date;
                    printf("NEW Sensor %d[%d] entered!\n",i,sensorlist[i].idxUser);
                    cJSON* obj = Beacon_SendAPI(sensor);
                    
                    Debug("Beacon_SendAPI %s \n",(obj!=NULL?"Succeed":"failed"));

                    if(obj != NULL)
                    {
                        extern void Send_RS485(uint8_t * data);
                        
                        Debug("RS485send start\n");
                        char* name = cJSON_GetStringValue(cJSON_GetObjectItem(obj,"nm"));
                        char* number = cJSON_GetStringValue(cJSON_GetObjectItem(obj,"no"));
                        Debug("RS485send 1\n");

                        printf("name:%s,number:%s\n",name != NULL?name:"NULL",number != NULL?number:"NULL");
                        if(name != NULL && number != NULL){
                            extern UART_HandleTypeDef huart1;
                            HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,SET);
                            HAL_UART_Transmit(&huart1,"\02",1,100);
                            HAL_UART_Transmit(&huart1,number,strlen(number),1000);
                            HAL_UART_Transmit(&huart1,",",1,100);
                            HAL_UART_Transmit(&huart1,name,strlen(name),1000);
                            HAL_UART_Transmit(&huart1,"\03",1,100);
                            HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,RESET);
                        }
                        Debug("RS485send 2\n");
                    }

                    sensorlist[i].ioState = 0;//IN
                    sensorlist[i].needtobeSended = FALSE;
                }

                //DETECT GOING OUT
                //then now search if It's exiting range(saveData.dist,INF)&&iostate:in&&saveData.enableOutDetecting==true + timeout + need-to-be-sended
                else if(sensorlist[i].ioState == 0 && 
                 (sensorlist[i].distance >= saveData.dist || HAL_GetTick() - sensorlist[i].beginTime > saveData.BeaconTimeout*1000 || sensorlist[i].needtobeSended == TRUE))
                {
                    sensorlist[i].needtobeSended = TRUE;
                    res = SUCCEED;
                    if(saveData.enableOutDetecting == TRUE){
                        HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,TRUE);
                        printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                        printf("Sensor %d[%d] exited!\n",i,sensorlist[i].idxUser);
                   
                        SensorData sensor = SensorList2Data(&sensorlist[i]);
                        //res = Beacon_SendAPI(sensor);
                        Debug("Beacon_SendAPI %s \n",(res==SUCCEED?"Succeed":"failed"));
                    }
                    sensorlist[i].ioState = 1;//OUT
                    sensorlist[i].needtobeSended = FALSE;
                    sensorlist[i].exitTime = HAL_GetTick();
                }
                
                if(res==-1)
                {
                    if(Send_Ping("www.google.com",4) == FAILED)
                        resetNetwork();
                    checkConnectivity(TRUE);
                }
                HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,RESET);
            }
        }
    }
}
SensorData SensorList2Data(SensorList* list)
{
    SensorData sensor;
    sensor.sensorDate = list->date;
    sensor.idxUser = list->idxUser;
    sensor.HackingCode = list->hackingCode;
    sensor.distance = list->distance;
    sensor.C = list->C;
    return sensor;
}

void BLESensorRead(SensorData sensor)
{
    int i=0;
    BOOL isinRange = sensor.distance < saveData.IdentifyDistance;
    //VACANCY SERACHING
    for(i=0;i<SENSORLIST_LEN;i++)
    {
        if(sensorlist[i].idxUser==sensor.idxUser)break;
    }
    if(i<SENSORLIST_LEN);//if found
    else //unless found
    {
        for(i=0;i<SENSORLIST_LEN;i++)
        {
            if(sensorlist[i].isAlive == FALSE)break;
        }
        if(i>=SENSORLIST_LEN)printf("ERROR! sensorLIST FULL\n");
        
        sensorlist[i].ioState = 1;//OUT
    }
    //CONFIGURATE or UPDATE data.
    sensorlist[i].idxUser = sensor.idxUser;
    sensorlist[i].isAlive = isinRange;
    sensorlist[i].beginTime = HAL_GetTick();
    sensorlist[i].distance = sensor.distance;
    sensorlist[i].hackingCode = sensor.HackingCode;
    sensorlist[i].C = sensor.C;
    sensorlist[i].date = sensor.sensorDate;

}