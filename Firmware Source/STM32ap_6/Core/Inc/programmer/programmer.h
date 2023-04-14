
void UARTReceiver_init();
void UARTReceiver_Process(char* msg);

void Start_DebugUDP();
void debugUDP(char* str,int len);
void End_DebugUDP();
void GetDebugRx();