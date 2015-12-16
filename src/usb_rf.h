/*
 * usb_rf.h
 *
 *  Created on: Dec 15, 2015
 *      Author: kieutq
 */

#ifndef SRC_USB_RF_H_
#define SRC_USB_RF_H_

#include <stdint.h>

#ifndef USBRF_AUTO
#define USBRF_AUTO 0
#endif

extern void USBRF_Init(void);
extern int USBRF_ConnectAvailable(void);
extern int USBRF_Connect(void);
extern int USBRF_DisConnect(void);
extern int USBRF_DataAvailable(void);
extern int USBRF_DataSend(uint8_t * buffer, int len);
extern int USBRF_DataGet(uint8_t * buffer, int len);

#endif /* SRC_USB_RF_H_ */
