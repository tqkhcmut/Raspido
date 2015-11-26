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

int CreateThesisThread(pthread_t * handler, uint32_t unique_number);

#endif /* SRC_THESIS_H_ */
