#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

/* Exported types ------------------------------------------------------------*/
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);
void ethernetif_set_link(struct netif *netif);
void ethernetif_update_config(struct netif *netif);
void ethernetif_notify_conn_changed(struct netif *netif);
void ethernet_transmit(void);

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
