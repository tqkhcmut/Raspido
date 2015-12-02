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
struct ThesisData __sensors_data[SENSORS_MAX];
unsigned char sensor_active[SENSORS_MAX];
//uint32_t __unique_number = 1;

#define THESIS_DB_NAME "thesis.db"

int isOpenThesisDB = 0;
sqlite3 * thesis_db;

//pthread_mutex_t db_access;

int ThesisConnectDB(void)
{
	int errno = sqlite3_open(THESIS_DB_NAME, &thesis_db);
	if (errno == SQLITE_OK)
	{
		isOpenThesisDB = 1;
		return 1;
	}
	else
	{
#if THESIS_DEBUG
		printf("Database open error: %s.", sqlite3_errstr(errno));
#endif
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

int ThesisQueryData(struct ThesisData * data, uint32_t unique_number)
{
	struct Packet * packet = malloc(128);
	packet->id = DEV_MY_THESIS; // device number have been replaced by unique number
	memcpy(packet->unique_number, &unique_number, sizeof(uint32_t));
	packet->cmd = CMD_TYPE_QUERY | CMD_SENSORS_VALUE;
	packet->data_type = DATA_TYPE_THESIS_DATA | BIG_ENDIAN_BYTE_ORDER;
	packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] = checksum((char *)packet);

	Serial_SendMultiBytes((unsigned char*)packet, getPacketLength((char *)packet));

	usleep(5000); // 5ms

	if (Serial_Available())
	{
		Serial_GetData((char *)packet, getPacketLength((char *)packet));
		// checksum check
		if (packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] != checksum((char *)packet))
		{
#if THESIS_DEBUG
			printf("Thesis packet checksum fail.\n");
#endif
			sensor_active[unique_number] = 0;
			memset(packet->data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
			return 1;
		}
		else
		{
			sensor_active[unique_number] = 1;
			memcpy(data, packet->data, getTypeLength(DATA_TYPE_THESIS_DATA));
			return 0;
		}
	}
	else
	{
#if THESIS_DEBUG
		printf("Timeout exception.\n");
#endif
		sensor_active[unique_number] = 0;
		memset(packet->data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
		return 1;
	}
}

int ThesisStoreToDatabase(struct ThesisData * data, uint32_t unique_number)
{
	sqlite3_stmt *statement;
	int result = -1;
	char query[1024];

	sprintf(query,
			"INSERT INTO sensor_values(unique, gas, lighting, tempc) VALUES(NULL,%d,%0.3f,%0.3f,%0.3f)",
			unique_number, data->Gas, data->Lighting, data->TempC);
	if (ThesisConnectDB())
	{
		int prepare_query = sqlite3_prepare(thesis_db, query, -1, &statement, 0);
		if (prepare_query == SQLITE_OK)
		{
			int res = sqlite3_step(statement);
			result = res;
			sqlite3_finalize(statement);
		}
		else
		{
#if THESIS_DEBUG
		printf("Database prepare_query error: %s.", sqlite3_errstr(prepare_query));
#endif
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
	unsigned int _time_poll = 500000; // 500ms
	uint32_t _sensor_unique = (unsigned int) params;
	struct ThesisData _thesis_data;

	for(;;)
	{
		if (pthread_mutex_trylock(&serial_access) == 0)
		{
			if (ThesisQueryData(&_thesis_data, _sensor_unique) != 0)
			{
				// exit
				ThesisDisonnectDB();
				_time_poll = 1000000;
//				pthread_mutex_unlock(&serial_access);
//				pthread_exit(NULL);
			}
			else
			{
				_time_poll = 500000;
#if THESIS_DEBUG
				printf("Got data from %d.\n", _sensor_unique);
#endif
				// put it to database
				ThesisStoreToDatabase(&_thesis_data, _sensor_unique);

				// put to array
				__sensors_data[_sensor_unique] = _thesis_data;
			}
			pthread_mutex_unlock(&serial_access);
		}
		else
		{
#if THESIS_DEBUG
			printf("Thread: %d: cannot get serial access.\n", (int)pthread_self());
#endif
		}

		usleep(_time_poll);
	}
	return NULL;
}

int CreateThesisThread(pthread_t * handler, uint32_t unique_number)
{
	return pthread_create(handler, NULL, &ThesisThread, (void *)unique_number);
}

