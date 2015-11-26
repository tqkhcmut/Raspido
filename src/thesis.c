/*
 * thesis.c
 *
 *  Created on: Nov 26, 2015
 *      Author: kieutq
 */
#include "thesis.h"

// need serial
#include "serial.h"
// need access token
#include "devices.h"
// need packet format
#include "packet.h"

// need standard c function
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sqlite3.h"

// global data
uint32_t __unique_number = 1;

#define THESIS_DB_NAME "thesis.db"

int isOpenThesisDB = 0;
sqlite3 * thesis_db;

int ThesisConnectDB(void)
{
	if (sqlite3_open(THESIS_DB_NAME, &thesis_db) == SQLITE_OK)
	{
		isOpenThesisDB = 1;
		return 1;
	}

	return 0;
}

void ThesisDisonnectDB(void)
{
	if (isOpenThesisDB == 1)
	{
		sqlite3_close(thesis_db);
	}
}

int ThesisQueryData(struct ThesisData * data)
{
	struct Packet * packet = malloc(128);
	packet->id = DEV_MY_THESIS; // device number have been replaced by unique number
	memcpy(packet->unique_number, &__unique_number, sizeof(__unique_number));
	packet->cmd = CMD_TYPE_QUERY | CMD_SENSORS_VALUE;
	packet->data_type = DATA_TYPE_THESIS_DATA | BIG_ENDIAN_BYTE_ORDER;
	packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] = checksum((char *)packet);

	Serial_SendMultiBytes((unsigned char*)packet, getPacketLength((char *)packet));

	usleep(5000); // 5ms

	Serial_GetData((char *)packet, getPacketLength((char *)packet));
	// checksum check
	if (packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] != checksum((char *)packet))
	{
#if THESIS_DEBUG
		printf("Thesis packet checksum fail.\n");
#endif
		memset(data, packet->data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
		return 1;
	}
	else
	{
		memcpy(data, packet->data, getTypeLength(DATA_TYPE_THESIS_DATA));
		return 0;
	}
}

int ThesisStoreToDatabase(struct ThesisData * data)
{
	sqlite3_stmt *statement;
	int result = -1;
	char query[1024];

	sprintf(query, "INSERT INTO sensor_values(unique, gas, lighting, tempc) VALUES(NULL,0.030, '')", )
	if (ThesisConnectDB())
	{
		int prepare_query = sqlite3_prepare(dbfile, query, -1, &statement, 0);
		if (prepare_query == SQLITE_OK)
		{
			int res = sqlite3_step(statement);
			result = res;
			sqlite3_finalize(statement);
		}
		ThesisDisonnectDB();
		return result;
	}
	else
	{
		result = 0;
	}
	return result;
}

void * ThesisThread(void * params)
{
	struct ThesisData _thesis_data;
	for(;;)
	{
		if (pthread_mutex_trylock(&serial_access) == 0)
		{
			if (ThesisQueryData(&_thesis_data) != 0)
			{
				// exit
				ThesisDisonnectDB();
				pthread_mutex_unlock(&serial_access);
				pthread_exit(NULL);
			}
			else
			{
				// put it to database
			}
		}
		else
		{

		}
	}
	return NULL;
}

int CreateThesisThread(pthread_t * handler, uint32_t unique_number)
{
	return pthread_create(handler, NULL, &ThesisThread, (void *)unique_number);
}

