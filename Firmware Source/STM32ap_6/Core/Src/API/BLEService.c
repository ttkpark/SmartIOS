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

        HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_RESET);
        HAL_SPI_Receive(&hspi1, rxbuffer, sizeof(rxbuffer), 1000);
        HAL_GPIO_WritePin(BLE_SS_GPIO_Port,BLE_SS_Pin,GPIO_PIN_SET);
        if(rxbuffer[0].AESData[0] != 0 && rxbuffer[0].AESData[1] != 0xFF)
        {
            // BLE Log #1
            //Debug("LE%d(%d)\n",rxbuffer[0].AESNo,rxbuffer[0].rssi);

            // Debug("AESNo : %d\3",rxbuffer[0].AESNo);
            // Debug("rssi  : %d\3",rxbuffer[0].rssi);
            // Debug("txPWR : %d\3",rxbuffer[0].txPower);

            // SPI Log #1
            // Debug("SPI SEND : %s\3",txbuffer);
            // uint8_t* pointer = rxbuffer;
            // Debug("{%02X %02X %02X %02X %02X %02X %02X %02X}\3"
            //   ,pointer[0],pointer[1],pointer[2],pointer[3]
            //   ,pointer[4],pointer[5],pointer[6],pointer[7]);
            SensorData sensor;
            Result res = BLEList_ProcessSensor(rxbuffer,&sensor);
            //Debug("BLEList_ProcessSensor %s \n",(res==SUCCEED?"Succeed":"failed"));
            if(res==SUCCEED)
            {
                BLESensorRead(sensor);
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
                
                //DETECT ENTERING
                //investigate if It's entering range(0,saveData.dist) && iostate:out (+cooling time:saveData.BeaconTimeout is greater > exittime)
                if(sensorlist[i].ioState == 1 && 
                    (
                        (HAL_GetTick() - sensorlist[i].beginTime) <= saveData.BeaconTimeout*1000// it scanned the sensor in the range of time.
                        && sensorlist[i].distance < saveData.dist
                        && ( (HAL_GetTick() - sensorlist[i].exitTime) >= saveData.BeaconTimeout*1000 || (HAL_GetTick() < sensorlist[i].exitTime)/*prep for grabage value*/ )
                      || 
                        sensorlist[i].needtobeSended == TRUE
                    )
                    )
                {
                    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,TRUE);
                    sensorlist[i].needtobeSended = TRUE;
                    printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                    SensorData sensor = SensorList2Data(&sensorlist[i]);
                    
                    res = SUCCEED;
                    sensor.sensorDate = sensorlist[i].date;
                    printf("NEW Sensor %d[%d] entered!\n",i,sensorlist[i].idxUser);
                    res = Beacon_SendAPI(sensor);
                    
                    Debug("Beacon_SendAPI %s \n",(res==SUCCEED?"Succeed":"failed"));
                    extern void Send_RS485(uint8_t * data);
                    Send_RS485("Sensor Input!!");
                    sensorlist[i].ioState = 0;//IN
                    sensorlist[i].needtobeSended = FALSE;
                }

                //DETECT GOING OUT
                //then now search if It's exiting range(saveData.dist,INF)&&iostate:in&&saveData.enableOutDetecting==true + timeout + need-to-be-sended
                else if(sensorlist[i].ioState == 0 && 
                 (sensorlist[i].distance >= saveData.dist || HAL_GetTick() - sensorlist[i].beginTime > saveData.BeaconTimeout*1000 || sensorlist[i].needtobeSended == TRUE))
                {
                    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,TRUE);
                    sensorlist[i].needtobeSended = TRUE;
                    printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                    printf("Sensor %d[%d] exited!\n",i,sensorlist[i].idxUser);
                    res = SUCCEED;
                    if(saveData.enableOutDetecting == TRUE){
                        SensorData sensor = SensorList2Data(&sensorlist[i]);
                        res = Beacon_SendAPI(sensor);
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
                HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,FALSE);
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