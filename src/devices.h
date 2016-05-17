/*
 * sensor.h
 *
 *  Created on: Sep 17, 2015
 *      Author: kieu
 */

#ifndef DEVICES_H_
#define DEVICES_H_

/*
 * this file contain devices communication source code
 * used for get or set data to devices
 */

//#include "global_config.h"
#include "packet.h"
#include <pthread.h>


#ifndef DEVICE_DEBUG
#define DEVICE_DEBUG 1
#endif

#ifndef DEVICE_DATABASE
#define DEVICE_DATABASE 0
#endif


struct polling_control
{
	unsigned char destroy;
	unsigned char enable;
	unsigned int time_poll_ms;
};

struct Device
{
	//
	struct polling_control polling_control;

	// required packet.h
	struct DataPacket dev_data;
};

/*
 * float structure, use for byte order correction
 */
union float_struct
{
	float f;
	unsigned char f_byte[4];
};
typedef union float_struct float_struct_t;

extern int Device_Init(void);
extern int Device_Polling(void);


#endif /* SENSOR_H_ */
