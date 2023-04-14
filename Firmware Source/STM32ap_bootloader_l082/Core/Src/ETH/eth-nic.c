#include "main.h"
#include "eth-nic.h"

void NIC_CS(BOOL state)
{
    LL_GPIO_SetOutputPin(MEM_CS_GPIO_Port,MEM_CS_Pin);
    LL_GPIO_SetOutputPin(BLE_SS_GPIO_Port,BLE_SS_Pin);
    if(state)LL_GPIO_SetOutputPin(ENC_SS_GPIO_Port,ENC_SS_Pin);
    else LL_GPIO_ResetOutputPin(ENC_SS_GPIO_Port,ENC_SS_Pin);
}
void NIC_RESET_PIN(BOOL state)
{
    if(state)LL_GPIO_SetOutputPin(RST_GPIO_Port,RST_Pin);
    else LL_GPIO_ResetOutputPin(RST_GPIO_Port,RST_Pin);
}

uint8_t SPI_TransmitReceive(SPI_TypeDef* spi,uint8_t data)
{
    if(!LL_SPI_IsEnabled(spi)){
        LL_SPI_Enable(spi);
    }
    
    while(!LL_SPI_IsActiveFlag_TXE(spi));
    LL_SPI_TransmitData8(spi,data);
    while(!LL_SPI_IsActiveFlag_RXNE(spi));
    return LL_SPI_ReceiveData8(spi);
}
uint8_t ENC_SPI_SendWithoutSelection(uint8_t command)
{
    return SPI_TransmitReceive(SPI1,command);
}

void nic_reset (void)
{

	NIC_CS(1);

	//Reset high for >2uS
	NIC_RESET_PIN(1);

	LL_mDelay(1);
	// NIC_DELAY_400NS;
	// NIC_DELAY_400NS;
	// NIC_DELAY_400NS;
	// NIC_DELAY_400NS;
	// NIC_DELAY_400NS;
	
	//Low to reset for at least 400nS
	NIC_RESET_PIN(0);
	//NIC_DELAY_400NS;
	LL_mDelay(1);

	//High to run
	NIC_RESET_PIN(1);

	//Pause 1mS
	LL_mDelay(1);
}
//**************************************************
//**************************************************
//********** NIC BIT FIELD CLEAR REGISTER **********
//**************************************************
//**************************************************
void nic_bit_field_clear_register (BYTE address, BYTE data)
{
	BYTE b_temp;

	//SELECT NIC
	NIC_CS(0);

	//SEND COMMAND
	b_temp = ENC_SPI_SendWithoutSelection(NIC_BFC | address);
	
	//SEND DATA
	b_temp = ENC_SPI_SendWithoutSelection(data);
	
	
	//DE-SELECT NIC
	NIC_CS(1);
}



//************************************************
//************************************************
//********** NIC BIT FIELD SET REGISTER **********
//************************************************
//************************************************
void nic_bit_field_set_register (BYTE address, BYTE data)
{
	BYTE b_temp;

	//SELECT NIC
	NIC_CS(0);
	
	//SEND COMMAND
	b_temp = ENC_SPI_SendWithoutSelection((NIC_BFS | address));
	
	//SEND DATA
	b_temp = ENC_SPI_SendWithoutSelection(data);
	
	//DE-SELECT NIC
	NIC_CS(1);
}

void nic_select_bank (WORD reg)
{
	nic_bit_field_clear_register(NIC_ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
	nic_bit_field_set_register(NIC_ECON1, ((WORD_VAL*)&reg)->v[1]);
}
//******************************
//******************************
//********** NIC READ **********
//******************************
//******************************
//Call with:-
//	5 bit address of the ETH control register to read from (the top 3 bits must be 0)
//Returns:-
//	Byte read from the Ethernet controller's ETH register.
NIC_REG nic_read (BYTE address)
{
	NIC_REG reg;

	//SELECT NIC
	NIC_CS(0);
	
	//WRITE ADDRESS	
	reg.val = ENC_SPI_SendWithoutSelection((NIC_RCR | address));

	//READ REGISTER
	reg.val = ENC_SPI_SendWithoutSelection(0);//Send a dummy byte to read

	//DE-SELECT NIC
	NIC_CS(1);

	return(reg);
}