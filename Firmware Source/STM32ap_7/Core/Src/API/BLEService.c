#include "main.h"
#include "ETH/Internet.h"
#include "memory/memory.h"
#include "buffer/CoderBuffer.h"
#include "JSON/JSONBuffer.h"
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
            
            // 센서 정보 자료형 SensorData를 정의하여 BLE 프레임을 처리한다.
            SensorData sensor;
            Result res = BLEList_ProcessSensor(rxbuffer,&sensor);
            //Debug("BLEList_ProcessSensor %s \n",(res==SUCCEED?"Succeed":"failed"));
            if(res==SUCCEED){
                BLESensorRead(sensor);//리스트 관리부로 데이터를 넘긴다. 성공했을 때만.
            }
            
        }
    }
    
}
void SensorListProcess()
{
    static uint32_t sensorlistCheckTimer;
    static uint32_t processTime;

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
                    //Debug("SensorListProcess.if RE-ENTERING start\n");
                    //printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                    //printf("Sensor %d[%d] re-entered!\n",i,sensorlist[i].idxUser);
                    printf("<%d",sensorlist[i].idxUser);
                    sensorlist[i].ioState = 0;//IN
                    sensorlist[i].needtobeSended = FALSE;
                    //Debug("SensorListProcess.if RE-ENTERING end\n");
                }

                //DETECT ENTERING
                //investigate if It's entering range(0,saveData.dist) && iostate:out (+cooling time:saveData.BeaconTimeout is greater > exittime)
                else if( sensorlist[i].ioState == 1 &&// (HAL_GetTick() - sensorlist[i].tenSecBeginTime > 10*1000) && //code for sending data every 10s if in range. 
                    (
                        (HAL_GetTick() - sensorlist[i].beginTime) <= saveData.BeaconTimeout*1000// it scanned the sensor in the range of time.
                        && sensorlist[i].distance < saveData.dist
                        && ( (HAL_GetTick() - sensorlist[i].exitTime) >= saveData.BeaconTimeout*1000 || (HAL_GetTick() < sensorlist[i].exitTime)/*prep for grabage value*/ )
                      || 
                        sensorlist[i].needtobeSended == TRUE
                    )
                    )
                {
                    printf("again : %dms\n",HAL_GetTick() - processTime);
                    processTime = HAL_GetTick();
                    
                    Debug("SensorListProcess.if ENTERING start\n");
                    sensorlist[i].tenSecBeginTime = HAL_GetTick();
                    //if() Debug("very the first recongition!\n");

                    SensorData sensor = SensorList2Data(&sensorlist[i]);
                    Debug("start : %dms\n",HAL_GetTick() - sensor.tenSecBeginTime);
                    HAL_GPIO_WritePin(LED_SENSOR_GPIO_Port,LED_SENSOR_Pin,SET);

                    sensorlist[i].needtobeSended = TRUE;
                    printf("(ioState,dist,time,exit)= (%d,%g,%d,%d)\n",sensorlist[i].ioState,sensorlist[i].distance,HAL_GetTick() - sensorlist[i].beginTime,HAL_GetTick() - sensorlist[i].exitTime);
                    
                    
                    res = SUCCEED;
                    sensor.sensorDate = sensorlist[i].date;
                    printf("NEW Sensor %d[%d] entered!\n",i,sensorlist[i].idxUser);
                    Debug("before BeaconSendAPI : %dms\n",HAL_GetTick() - sensor.tenSecBeginTime);
                    JSONBuffer obj = Beacon_SendAPI(sensor);
                    Debug("after BeaconSendAPI : %dms\n",HAL_GetTick() - sensor.tenSecBeginTime);
                    Debug("SensorListProcess.Beacon_SendAPI %s \n",(obj!=NULL?"Succeed":"failed"));

                    if(obj != NULL)
                    {
                        extern void Send_RS485(uint8_t * data);
                        
                        Debug("SensorListProcess.RS485send start\n");
                        char* name = json_getPropertyValue(obj->json,"nm");
                        char* number = json_getPropertyValue(obj->json,"no");
                        char* rtncode = json_getPropertyValue(obj->json,"rtncode");
                        Debug("SensorListProcess.RS485send 1\n");

                        
                        Debug("SensorListProcess.nameprint start\n");
                        Debug("name:%s,number:%s\n",name != NULL?name:"NULL",number != NULL?number:"NULL");
                        if(name != NULL && number != NULL){
                            extern UART_HandleTypeDef huart1;
                            //huart1.Init.BaudRate = 9600;
                            //if (HAL_UART_Init(&huart1) != HAL_OK){}

                            HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,SET);
                            HAL_UART_Transmit(&huart1,"\02",1,100);
                            HAL_UART_Transmit(&huart1,number,strlen(number),1000);
                            HAL_UART_Transmit(&huart1,",",1,100);
                            HAL_UART_Transmit(&huart1,name,strlen(name),1000);
                            HAL_UART_Transmit(&huart1,",",1,100);
                            HAL_UART_Transmit(&huart1,rtncode,strlen(rtncode),1000);
                            HAL_UART_Transmit(&huart1,"\03",1,100);
                            HAL_GPIO_WritePin(USART1_DE_GPIO_Port,USART1_DE_Pin,RESET);
                            
                            //huart1.Init.BaudRate = 256000;
                            //if (HAL_UART_Init(&huart1) != HAL_OK){}
                        }
                        Debug("SensorListProcess.nameprint end\n");
                    }
                    
                    JSONBuffer_free(obj);
                    sensorlist[i].ioState = 0;//IN
                    sensorlist[i].needtobeSended = FALSE;
                    Debug("SensorListProcess.if ENTERING end\n");
                    printf("end RS485 : %dms\n",HAL_GetTick() - sensor.tenSecBeginTime);
                }
                

                //DETECT GOING OUT
                //then now search if It's exiting range(saveData.dist,INF)&&iostate:in&&saveData.enableOutDetecting==true + timeout + need-to-be-sended
                else if(sensorlist[i].ioState == 0 && 
                 (sensorlist[i].distance >= saveData.dist || HAL_GetTick() - sensorlist[i].beginTime > saveData.BeaconTimeout*1000 || sensorlist[i].needtobeSended == TRUE))
                {
                    //Debug("SensorListProcess.if OUT start\n");
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
                    //Debug("SensorListProcess.if OUT end\n");
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
    sensor.tenSecBeginTime = list->tenSecBeginTime;
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