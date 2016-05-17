#include "packet.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int phy_to_data(struct PhysicalPacket * phy, struct DataPacket * data)
{
	int ret = 0;
	data->type = phy->type;
	data->id = phy->id;
	/*
	 * @warning: make sure buffer have right allocation
	 */
	memcpy(data->unique_number, phy->unique_number, 4);
	if (phy->data != NULL)
	{
		data->data_type = phy->data[0]; // first byte of physical data
		data->data_length = get_data_length(data->data_type);
		/*
		 * @warning: make sure buffer have right allocation
		 */
//		if (data->data != NULL)
//			free(data->data);
//		data->data = calloc(data->data_length, 1);
		memcpy(data->data, (phy->data + 1), data->data_length);
	}

	return ret;
}
int data_to_phy(struct DataPacket * data, struct PhysicalPacket *phy)
{
	int ret = 0;

//	/*
//	 *
//	 */
//	printf("Convert Data packet to Physical packet.\r\n");
//
//	printf("Copy type and id\r\n");
	phy->type = data->type;
	phy->id = data->id;

	/*
	 * @warning: make sure buffer have right allocation
	 */
//	printf("Copy unique number.\r\n");
	memcpy(phy->unique_number, data->unique_number, 4);
	/*
	 * @warning: make sure buffer have right allocation
	 */
//	printf("Copy data.\r\n");
	if (data->data != NULL)
	{
//		if (phy->data != NULL)
//			free(phy->data);
//		phy->data = calloc(data.data_length + 1, 1);
		phy->data[0] = data->data_type; // first byte of physical data
		memcpy((phy->data + 1), data->data, data->data_length);
	}
//	printf("Update PhysicalPacket length.\r\n");
	phy->length = data->data_length 	// data
			+ 4 					// unique number
			+ 1 					// id
			+ 2; 					// type and length

	/*
	 *
	 */
//	printf("End if converting Data packet to Physical packet.\r\n");

	return ret;
}


int get_data_length(int data_type)
{
	int ret = 0;
	if (IS_SUPPORT_DATA_TYPE(data_type))
	{
		switch(data_type)
		{
		case DATA_TYPE_U8:
		case DATA_TYPE_S8:		ret = 1; break;
		case DATA_TYPE_U16:
		case DATA_TYPE_S16:		ret = 2; break;
		case DATA_TYPE_U32:
		case DATA_TYPE_S32:		ret = 4; break;
		case DATA_TYPE_U64:
		case DATA_TYPE_S64:		ret = 8; break;
		case DATA_TYPE_FLOAT: 	ret = 4; break;
		case DATA_TYPE_DOUBLE: 	ret = 8; break;

		case DATA_TYPE_ULTRA_SONIC:
			ret = sizeof(struct UltraSonic);
			break;
		case DATA_TYPE_TEMPERATURE:
			break;
		case DATA_TYPE_TIME:
			break;
		case DATA_TYPE_CALENDER:
			break;
		default: break;
		}
	}

	return ret;
}
uint8_t checksum(struct PhysicalPacket * phy)
{
	uint8_t checksum = 0, i;
	uint8_t * packet = (uint8_t *)phy;
	for (i = 0; i < phy->length; i++)
		checksum += packet[i];
	checksum = (uint8_t)!checksum + 1;
	return checksum;
}

