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
#include <stdint.h>

#include "sqlite3.h"
#include "usb_rf.h"

// global data
struct ThesisData __sensors_data[SENSORS_MAX];
unsigned char sensor_active[SENSORS_MAX];
//uint32_t __unique_number = 1;

#define GAS_LIMIT 1500
#define LIGHT_LIMIT 100
#define TEMPC_LIMIT 40


#define THESIS_DB_NAME "/var/www/tqk/thesis.db"

int isOpenThesisDB = 0;
sqlite3 * thesis_db;


#ifndef int_s
typedef union
{
	uint32_t int_n;
	uint8_t int_b[4];
} int_s;
#endif

//pthread_mutex_t db_access;

int ThesisConnectDB(void)
{
	int errno = sqlite3_open_v2(THESIS_DB_NAME, &thesis_db, SQLITE_OPEN_READWRITE |
			SQLITE_OPEN_CREATE | SQLITE_OPEN_WAL, NULL);
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
	int_s _unique_number;
	_unique_number.int_n = unique_number;
	struct Packet * packet = malloc(128);
	packet->id = DEV_MY_THESIS; // device number have been replaced by unique number
	packet->unique_number[0] = _unique_number.int_b[3];
	packet->unique_number[1] = _unique_number.int_b[2];
	packet->unique_number[2] = _unique_number.int_b[1];
	packet->unique_number[3] = _unique_number.int_b[0];
	//	memcpy(packet->unique_number, &unique_number, sizeof(uint32_t));
	packet->cmd = CMD_TYPE_QUERY | CMD_SENSORS_VALUE;
	packet->data_type = DATA_TYPE_THESIS_DATA | BIG_ENDIAN_BYTE_ORDER;
	packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] = checksum((char *)packet);

#if THESIS_DEBUG
	printf("Thread: %d: Query Packet: ", unique_number);
	int i;
	for (i = 0; i < getPacketLength((char *)packet); i++)
	{
		printf("%02X ", *((unsigned char *) packet + i));
	}
	printf("\nThread: %d: Checksum: %02X.\n", unique_number, packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)]);
#endif

	Serial_SendMultiBytes((unsigned char*)packet, getPacketLength((char *)packet));

	usleep(50000); // 50ms

	if (Serial_Available())
	{
		Serial_GetData((char *)packet, getPacketLength((char *)packet));
#if THESIS_DEBUG
		printf("Thread: %d: Received Packet: ", unique_number);
		int i;
		for (i = 0; i < getPacketLength((char *)packet); i++)
		{
			printf("%02X ", *((unsigned char *) packet + i));
		}
		printf("\nThread: %d: Checksum: %02X.\n", unique_number, packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)]);
#endif
		// checksum check
		if (packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] != checksum((char *)packet))
		{
#if THESIS_DEBUG
			printf("Thread: %d: Thesis packet checksum fail. Received: %X2. Own: %X2\n",
					unique_number,
					packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)],
					checksum((char *)packet));
#endif
			sensor_active[unique_number] = 0;
			memset(data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
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
		printf("Thread: %d: Timeout exception.\n", unique_number);
#endif
		sensor_active[unique_number] = 0;
		memset(packet->data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
		return 1;
	}
}

int UpdateSensorInfos(struct ThesisData * limit_data, uint32_t unique_number, int active_state)
{
	sqlite3_stmt *statement;
	int result = -1;
	char query[1024];

	int access_try = 50;
	while (pthread_mutex_trylock(&db_access) != 0)
	{
		access_try--;
		if (access_try == 0)
			break;
		usleep(10000); // 10ms
	}
	if (access_try > 0)
	{
		if (ThesisConnectDB())
		{
			sprintf(query,
					"update sensors set active_state = %d, gas_limit = %0.3f, light_limit = %0.3f, tempc_limit = %0.3f, active_time = current_timestamp WHERE unique_number = %d",
					active_state,
					limit_data->Gas,
					limit_data->Lighting,
					limit_data->TempC,
					unique_number);
#if DATABASE_DEBUG
			printf("Thread: %d: Database query: %s.\n", unique_number, query);
#endif

			int prepare_query = sqlite3_prepare(thesis_db, query, -1, &statement, 0);
			if (prepare_query == SQLITE_OK)
			{
				int res = sqlite3_step(statement);
				result = res;
				sqlite3_finalize(statement);
			}
			else
			{
#if DATABASE_DEBUG
				printf("Thread: %d: Database prepare_query error: %s.", unique_number, sqlite3_errstr(prepare_query));
#endif
			}
			ThesisDisonnectDB();
			pthread_mutex_unlock(&db_access);
			return result;
		}
		else
		{
#if DATABASE_DEBUG
			printf("Thread: %d: Database cannot connect..", unique_number);
#endif
			result = 0;
		}
		pthread_mutex_unlock(&db_access);
	}
	else
	{
#if DATABASE_DEBUG
		printf("Thread: %d: cannot get db access.\n", unique_number);
#endif
	}
	return result;
}

int ThesisStoreToDatabase(struct ThesisData * data, uint32_t unique_number)
{
	sqlite3_stmt *statement;
	int result = -1;
	char query[1024];


	int access_try = 50;
	while (pthread_mutex_trylock(&db_access) != 0)
	{
		access_try--;
		if (access_try == 0)
			break;
		usleep(10000); // 10ms
	}
	if (access_try > 0)
	{
		sprintf(query,
				"INSERT INTO sensor_values(unique_number, gas, lighting, tempc) VALUES(%d,%0.3f,%0.3f,%0.3f)",
				unique_number, data->Gas, data->Lighting, data->TempC);
#if DATABASE_DEBUG
		printf("Thread: %d: Database query: %s.\n", unique_number, query);
#endif
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
#if DATABASE_DEBUG
				printf("Thread: %d: Database prepare_query error: %s.", unique_number, sqlite3_errstr(prepare_query));
#endif
			}
			ThesisDisonnectDB();
			pthread_mutex_unlock(&db_access);
			return result;
		}
		else
		{
#if DATABASE_DEBUG
			printf("Thread: %d: Database cannot connect..", unique_number);
#endif
			result = 0;
		}
		pthread_mutex_unlock(&db_access);
	}
	else
	{
#if DATABASE_DEBUG
			printf("Thread: %d: cannot get db access.\n", unique_number);
#endif
	}
	return result;
}

void * ThesisThread(void * params)
{
	unsigned int _time_poll = 1000000; // 500ms
	uint32_t _sensor_unique = (unsigned int) params;
	struct ThesisData _thesis_data, limit;
	limit.Gas = GAS_LIMIT;
	limit.Lighting = LIGHT_LIMIT;
	limit.TempC = TEMPC_LIMIT;
	int access_try = 50, disconnect_try = 3;
	int active_reg = 0;
	UpdateSensorInfos(&limit, _sensor_unique, 0);
	for(;;)
	{
		access_try = 50;
		while (pthread_mutex_trylock(&serial_access) != 0)
		{
			access_try--;
			if (access_try == 0)
				break;
			usleep(10000); // 10ms
		}
		if (access_try > 0)
		{
			if (ThesisQueryData(&_thesis_data, _sensor_unique) != 0)
			{
				// exit
				//				ThesisDisonnectDB();
				_time_poll = 1000000;
				//				pthread_mutex_unlock(&serial_access);
				//				pthread_exit(NULL);
				if (disconnect_try == 0)
				{
					if (active_reg == 1)
					{
						UpdateSensorInfos(&limit, _sensor_unique, 0);
						active_reg = 0;
					}
				}
				else
					disconnect_try --;
			}
			else
			{
				_time_poll = 1000000;
				disconnect_try = 3;
#if THESIS_DEBUG
				printf("Thread: %d: Got data from %d.\n", _sensor_unique, _sensor_unique);
#endif
				// put it to database
				ThesisStoreToDatabase(&_thesis_data, _sensor_unique);

				// put to array
				__sensors_data[_sensor_unique] = _thesis_data;

				if (active_reg == 0)
				{
					UpdateSensorInfos(&limit, _sensor_unique, 1);
					active_reg = 1;
				}
			}
			pthread_mutex_unlock(&serial_access);
		}
		else
		{
#if THESIS_DEBUG
			printf("Thread: %d: cannot get serial access.\n", _sensor_unique);
#endif
		}

		usleep(_time_poll);
	}
	return NULL;
}


int ThesisQueryDataUSBRF(struct ThesisData * data, uint32_t unique_number)
{
	if (USBRF_ConnectAvailable())
	{
		int_s _unique_number;
		_unique_number.int_n = unique_number;
		struct Packet * packet = malloc(128);
		packet->id = DEV_MY_THESIS; // device number have been replaced by unique number
		packet->unique_number[0] = _unique_number.int_b[3];
		packet->unique_number[1] = _unique_number.int_b[2];
		packet->unique_number[2] = _unique_number.int_b[1];
		packet->unique_number[3] = _unique_number.int_b[0];
		//	memcpy(packet->unique_number, &unique_number, sizeof(uint32_t));
		packet->cmd = CMD_TYPE_QUERY | CMD_SENSORS_VALUE;
		packet->data_type = DATA_TYPE_THESIS_DATA | BIG_ENDIAN_BYTE_ORDER;
		packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] = checksum((char *)packet);

#if THESIS_DEBUG
		printf("RF Thread: %d: Query Packet: ", unique_number);
		int i;
		for (i = 0; i < getPacketLength((char *)packet); i++)
		{
			printf("%02X ", *((unsigned char *) packet + i));
		}
		printf("\nRF Thread: %d: Checksum: %02X.\n", unique_number, packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)]);
#endif
		USBRF_Connect();
		USBRF_DataSend((uint8_t *)packet, getPacketLength((char *)packet));

		usleep(50000); // 50ms

		if (USBRF_DataGet((uint8_t *)packet, getPacketLength((char *)packet)))
		{
#if THESIS_DEBUG
			printf("RF Thread: %d: Received Packet: ", unique_number);
			int i;
			for (i = 0; i < getPacketLength((char *)packet); i++)
			{
				printf("%02X ", *((unsigned char *) packet + i));
			}
			printf("\nRF Thread: %d: Checksum: %02X.\n", unique_number, packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)]);
#endif
			// checksum check
			if (packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)] != checksum((char *)packet))
			{
#if THESIS_DEBUG
				printf("RF Thread: %d: Thesis packet checksum fail. Received: %X2. Own: %X2\n",
						unique_number,
						packet->data[getTypeLength(DATA_TYPE_THESIS_DATA)],
						checksum((char *)packet));
#endif
				sensor_active[unique_number] = 0;
				memset(data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
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
			printf("RF Thread: %d: Timeout exception.\n", unique_number);
#endif
			sensor_active[unique_number] = 0;
			memset(packet->data, 0, getTypeLength(DATA_TYPE_THESIS_DATA));
			return 1;
		}
		USBRF_DisConnect();
	}
	return 1;
}

void * ThesisThreadUSBRF(void * params)
{
	unsigned int _time_poll = 1000000; // 500ms
	uint32_t _sensor_unique = (unsigned int) params;
	struct ThesisData _thesis_data, limit;
	limit.Gas = GAS_LIMIT;
	limit.Lighting = LIGHT_LIMIT;
	limit.TempC = TEMPC_LIMIT;
	int access_try = 50;
	int active_reg = 0;
	UpdateSensorInfos(&limit, _sensor_unique, 0);
	for(;;)
	{
		access_try = 50;
		while (pthread_mutex_trylock(&usbrf_access) != 0)
		{
			access_try--;
			if (access_try == 0)
				break;
			usleep(10000); // 10ms
		}
		if (access_try > 0)
		{
			if (ThesisQueryDataUSBRF(&_thesis_data, _sensor_unique) != 0)
			{
				// exit
				//				ThesisDisonnectDB();
				_time_poll = 1000000;
				//				pthread_mutex_unlock(&serial_access);
				//				pthread_exit(NULL);
				if (active_reg == 1)
				{
					UpdateSensorInfos(&limit, _sensor_unique, 0);
					active_reg = 0;
				}
			}
			else
			{
				_time_poll = 1000000;
#if THESIS_DEBUG
				printf("RF Thread: %d: Got data from %d.\n", _sensor_unique, _sensor_unique);
#endif
				// put it to database
				ThesisStoreToDatabase(&_thesis_data, _sensor_unique);
				if (active_reg == 0)
				{
					UpdateSensorInfos(&limit, _sensor_unique, 1);
					active_reg = 1;
				}

				// put to array
				__sensors_data[_sensor_unique] = _thesis_data;

			}
			pthread_mutex_unlock(&usbrf_access);
		}
		else
		{
#if THESIS_DEBUG
			printf("RF Thread: %d: cannot get USBRF access.\n", _sensor_unique);
#endif
		}

		usleep(_time_poll);
	}
	return NULL;
}

int CreateThesisThread(pthread_t * handler1, pthread_t * handler2, uint32_t unique_number)
{
	pthread_create(handler1, NULL, &ThesisThread, (void *)unique_number);
	pthread_create(handler2, NULL, &ThesisThreadUSBRF, (void *)unique_number);
	return 0;
}

