/*
 * sensor.c
 *
 *  Created on: Sep 17, 2015
 *      Author: kieu
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "devices.h"
#include "serial.h"
#include "raspi_ext.h"

#include <unistd.h>
#include "../wiringPi/wiringPi.h"

#include "server.h"

#include <pthread.h>

#define DEV_HOST_NUMBER 4 // 4 USB interfaces

struct Device dev_host[DEV_HOST_NUMBER];

#define BUFFER_SIZE 128
uint8_t send_buff[BUFFER_SIZE];
uint8_t recv_buff[BUFFER_SIZE];
uint8_t data_buff[BUFFER_SIZE];

union int_s
{
  uint32_t n;
  uint8_t b[4];
};
union float_s
{
	float f;
	uint8_t b[4];
};


int sendControl(struct Device dev)
{

	return 0;
}

int queryData(struct Device * dev)
{
	int serial_len = 0;
	int i;

	/*
	 *
	 */
	printf("Query data for device: %2X\r\n", dev->dev_data.id);

	// required physical packet had it length plus 1 for checksum
	struct PhysicalPacket * send_phy = (struct PhysicalPacket *)send_buff;
	data_to_phy(&(dev->dev_data), send_phy);
	((uint8_t *)send_phy)[send_phy->length] = checksum(send_phy);
	Serial_SendMultiBytes((unsigned char *)send_phy, send_phy->length + 1);

	/*
	 *
	 */
	printf("Query Packet: ");
	for (i = 0; i < send_phy->length + 1; i++)
	{
		printf("%2X ", ((unsigned char *)send_phy)[i]);
	}
	printf("\r\n");

	// wait for devices responses
	usleep(10000); // 10ms

	serial_len = Serial_Available();
	if (serial_len > 7)
	{
//		char * raw_buffer = calloc(serial_len, sizeof(char));
		Serial_GetData((char *)recv_buff, serial_len);

		/*
		 *
		 */
		printf("Received Packet: ");
		for (i = 0; i < serial_len; i++)
		{
			printf("%2X ", recv_buff[i]);
		}
		printf("\r\n");

		struct PhysicalPacket * recv_phy = (struct PhysicalPacket *)recv_buff;

		if (checksum(recv_phy) != recv_buff[serial_len-1])
		{
			printf("Wrong checksum.\r\n");
		}
		else
		{
			struct DataPacket * data = (struct DataPacket * )data_buff;
			phy_to_data(recv_phy, data);

			printf("Receive data from: %2X.\r\n", data->id);
			if (data->id == DEV_ULTRA_SONIC)
			{
				union float_s distance;
				union int_s unique_number;
				distance.b[0] = data->data[3];
				distance.b[1] = data->data[2];
				distance.b[2] = data->data[1];
				distance.b[3] = data->data[0];
				unique_number.b[0] = data->unique_number[3];
				unique_number.b[1] = data->unique_number[2];
				unique_number.b[2] = data->unique_number[1];
				unique_number.b[3] = data->unique_number[0];
				printf("Receive distance: %0.3f from ultra sonic: %d.\r\n",
						distance.f, unique_number.n);
			}
		}

//		free(raw_buffer);
	}

//	free(phy);
	return 0;
}

int Device_Polling(void) // thread
{
	int i;
	for (i = 0; i < DEV_HOST_NUMBER; i++)
	{
		queryData(&dev_host[i]);
		sleep(1);
	}
	return 0;
}
int Device_Init(void)
{
	int i = 0;
	printf("Initial Sensor Host.\r\n");
	
	// for external components controller
	RaspiExt_Init();

	Serial_Init();
	for (i = 0; i < DEV_HOST_NUMBER; i++)
	{
		printf("Initial Sensor Host %d parameters.\r\n", i);
		dev_host[i].polling_control.enable = 0;
		dev_host[i].polling_control.time_poll_ms = 200;
		dev_host[i].polling_control.destroy = 0;
		dev_host[i].dev_data.id = DEV_ULTRA_SONIC;
//		dev_host[i].dev_data.data = NULL;
		dev_host[i].dev_data.data_length = 0;
		dev_host[i].dev_data.data_type = DATA_TYPE_NULL;
		dev_host[i].dev_data.type = CMD_ULTRA_SONIC | CMD_TYPE_QUERY;
		dev_host[i].dev_data.unique_number[0] = 0;
		dev_host[i].dev_data.unique_number[1] = 0;
		dev_host[i].dev_data.unique_number[2] = 2;
		dev_host[i].dev_data.unique_number[3] = i;
	}

	CreateServer(4);

	return 0;
}
