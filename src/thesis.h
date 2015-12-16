/*
 * thesis.h
 *
 *  Created on: Nov 26, 2015
 *      Author: kieutq
 */

#ifndef SRC_THESIS_H_
#define SRC_THESIS_H_

#include <pthread.h>
#include <stdint.h>
#include "packet.h"

#ifndef THESIS_DEBUG
#define THESIS_DEBUG 1
#endif

int CreateThesisThread(pthread_t * handler1, pthread_t * handler2, uint32_t unique_number);


#define SENSORS_MAX	10
extern struct ThesisData __sensors_data[];
extern unsigned char sensor_active[];

#endif /* SRC_THESIS_H_ */
