/*
 * main.c
 *
 *  Created on: Sep 22, 2015
 *      Author: kieutq
 */

#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#ifdef __linux
#include <unistd.h>
#include <signal.h>
#endif

void die(int err)
{
	printf("Program exit with code: %d\n", err);
	exit(err);
}


int main (int argc, char * argv[])
{
	signal(SIGINT, die);
	printf("Sensor Host.\n");

	Device_Init();

	for (;;)
	{
		Device_Polling();
	}
	return 0;
}
