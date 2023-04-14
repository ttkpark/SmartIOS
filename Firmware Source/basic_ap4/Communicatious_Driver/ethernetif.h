#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
void ENC_Init();
uint32_t sys_now(void);
uint8_t ENC_SPI_SendWithoutSelection(uint8_t command);
uint8_t ENC_SPI_Send(uint8_t command);
void ENC_SPI_SendBuf(uint8_t *master2slave, uint8_t *slave2master, uint16_t bufferSize);
void ENC_SPI_Select(uint8_t select);

/* Activate ENC28J60 interrupts processing */
#define ENC28J60_INTERRUPT

/** @defgroup PIN configuration for ENC28J60
  * @{
  */
  /*
        ENC28J60_CS ----------------------> PA4
        ENC28J60_INT ---------------------> PB0
  */
#define ENC_CS_PIN    GPIO_PIN_4
#define ENC_INT_PIN   GPIO_PIN_0


#define MAC_ADDR0   0x8f
#define MAC_ADDR1   0x7b
#define MAC_ADDR2   0xed
#define MAC_ADDR3   0x06
#define MAC_ADDR4   0x91
#define MAC_ADDR5   0xe0
/**
  * @}
  */

#endif
