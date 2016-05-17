/*
 * server.c
 *
 *  Created on: May 14, 2016
 *      Author: Tra Quang Kieu
 */

#include "server.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

pthread_t main_server_thread;
pthread_t client_threads;
int valid_thread_id = 0;
void * main_server_poll(void * params);
void * client_service(void * params);

int CreateServer(int conn)
{
	pthread_create(&main_server_thread, NULL, main_server_poll, (void *)conn);

	return 0;
}

void * main_server_poll(void * params)
{
	int _conn = (int)params;
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	printf("Server listener started.\r\n");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("ERROR opening socket. Server sleep forever.\r\n");
		for (;;)
			sleep(1);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(5438);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
	{
		printf("ERROR on binding. Server sleep forever.\r\n");
		for (;;)
			sleep(1);
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	for (;;)
	{
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr,
				&clilen);
		if (newsockfd < 0)
		{
			printf("Server ERROR on accept.\r\n");
			for (;;)
				sleep(1);
		}
		else
			pthread_create(&client_threads, NULL, client_service, (void *)newsockfd);
	}
	close(sockfd);
	return NULL;
}


void * client_service(void * params)
{
	int clisockfd = (int)params;
	int n;
	printf("Server service for client: %d.\r\n", clisockfd);
	for(;;)
	{
		n = send(clisockfd, "I got your message.\r\n", 21, MSG_NOSIGNAL);
		if (n < 0)
		{
			printf("ERROR writing to socket.\r\n");
			break;
		}
		sleep(1);
	}

	printf("Close connection for client: %d.\r\n", clisockfd);
	close(clisockfd);

	return NULL;
}

