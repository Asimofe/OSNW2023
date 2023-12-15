#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXLINE 1024
#define PORTNUM 3600

typedef struct cal_data {
	int data1, data2;
	char op;
	char str_data[128];
	int result;
	short error;
	struct tm local_time;
}cal_data;

int count = 0;
pthread_mutex_t t_lock[256];
pthread_cond_t t_cond[256];
cal_data sdata[256];

void clean_struct(int index){
	sdata[index].data1 = 0;
	sdata[index].data2 = 0;
	sdata[index].op = ' ';
	memset(sdata[index].str_data , 0x00, 128);
	sdata[index].result = 0;
	sdata[index].error = 0;
}

void * producer_thread(void *data)
{
	int TH_index = count;
	time_t now_time;
	struct tm *current_time;
	int flag = 0;
	char *token;

	int sockfd = *((int *)data);
	int readn, retval;
	socklen_t addrlen;
	char buf[MAXLINE];
	struct sockaddr_in client_addr;
	memset(buf, 0x00, MAXLINE);
	addrlen = sizeof(client_addr);
	getpeername(sockfd, (struct sockaddr *)&client_addr, &addrlen);
	
	/* select initialize setting start*/
	fd_set readfds;
	struct timeval timeout;
	timeout.tv_sec = 10;  /* 10sec timeout */
	timeout.tv_usec = 0;
	/* select initialize setting end */	
	
	printf("Create thread %d\n" , TH_index);

	while(1){
		memset(buf, 0x00, MAXLINE); /* buffer memory initialize */
		/* fd setting start */
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		/* fd setting end */	

		
		retval = select(256, &readfds, NULL, NULL, &timeout);
		if(retval == -1){
			perror("read fd fail");
			return 0;
		}else if(retval == 0){
			/* time out -> Consumer Signal Process */
			/* print flag (Consoumer -> Client */
			if(flag == 0) continue;
			/* time update */
			time(&now_time);
			current_time = localtime(&now_time);
			sdata[TH_index].local_time = *current_time;

			pthread_cond_signal(&t_cond[TH_index]);
		}
		else{
			if(FD_ISSET(sockfd, &readfds)){
				//clean_struct(TH_index);
				memset(&sdata[TH_index], 0x00, sizeof(cal_data));
				if(read(sockfd, buf, MAXLINE) <= 0){
					perror("read error :");
					return 0;
				}
				if(flag == 0) flag = 1;
				printf("Read data : %s" , buf);
				/* data slicing */		
				token = strtok(buf, " \n");
				if(token != NULL) sdata[TH_index].data1 = atoi(token);
				token = strtok(NULL, " \n");
				if(token != NULL) sdata[TH_index].data2 = atoi(token);
				token = strtok(NULL, " \n");
				if(token != NULL) sdata[TH_index].op = token[0];
				token = strtok(NULL, " \n");
				if(token != NULL) strcpy(sdata[TH_index].str_data , token);
				
				/* quit process */
				if(strcmp(sdata[TH_index].str_data, "quit") == 0){
					pthread_cond_signal(&t_cond[TH_index]);
					return 0;
				}

				/* calculate and write data in struct data */
				switch(sdata[TH_index].op){
					case '+' :
						sdata[TH_index].result = sdata[TH_index].data1 + sdata[TH_index].data2;
						break;
					case '-' :
						sdata[TH_index].result = sdata[TH_index].data1 - sdata[TH_index].data2;
						break;
					case 'x' :
						sdata[TH_index].result = sdata[TH_index].data1 * sdata[TH_index].data2;
						break;
					case '/' :
						if(sdata[TH_index].data2 == 0){
							sdata[TH_index].error = 1;
							break;
						}
						sdata[TH_index].result = sdata[TH_index].data1 / sdata[TH_index].data2;
						break;
					default:
						sdata[TH_index].error = -1;
				}

				/*Time setting */
				time(&now_time);
				current_time = localtime(&now_time);
				sdata[TH_index].local_time = *current_time;

				/* Counsumer Signal Process */
				pthread_cond_signal(&t_cond[TH_index]);
							
			}
		}

	}
	close(sockfd);
	printf("worker thread end\n");
	return 0;
}

void * consumer_thread(void *data)
{
	int TH_index = count;

	int sockfd = *((int  *)data);
	int readn;
	char buf[MAXLINE];
	char rbuf[20], tbuf[50];
	memset(buf, 0x00, MAXLINE);


	strcpy(buf, "Hello");
	socklen_t addrlen;
	struct sockaddr_in client_addr;
	addrlen = sizeof(client_addr);
	getpeername(sockfd, (struct sockaddr *)&client_addr, &addrlen);
	while(1){
		/* buffer initialize */	
		memset(buf, 0x00, MAXLINE);
		memset(rbuf, 0x00, 20);
		memset(tbuf, 0x00, 50);

		/* Mutex locck and cond_wait -> critical section */
		pthread_mutex_lock(&t_lock[TH_index]);
		pthread_cond_wait(&t_cond[TH_index], &t_lock[TH_index]);

		/* quit process */
		if(strcmp(sdata[TH_index].str_data, "quit") == 0){
			write(sockfd, "quit", 5);
			return 0;
		}

		/* Struct data -> String data */
		sprintf(buf, "%d %d %c ", sdata[TH_index].data1, sdata[TH_index].data2, sdata[TH_index].op);
		if(sdata[TH_index].error == 0){
			sprintf(rbuf, "%d ", sdata[TH_index].result);
			strcat(buf, rbuf);
		}else if(sdata[TH_index].error == -1){
			strcat(buf, "operator-error ");
		}else{
			strcat(buf, "Zero-Dibision-Error ");
		}
		strftime(tbuf, sizeof(tbuf), "%Y %b %d %a %H:%M:%S ", &(sdata[TH_index].local_time));
		strcat(buf, tbuf);

		/* write to client */
		if( write(sockfd, buf, strlen(buf)) <= 0){
			perror("Write error Consumer");
			return 0;
		}
		/* Mutex unlock */
		pthread_mutex_unlock(&t_lock[TH_index]);
	}
}

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];
	pthread_t thread_id;

	struct sockaddr_in server_addr, client_addr;

	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==-1	)
	{
		perror("bind error");
		return 1;
	}
	if(listen(listen_fd, 5) == -1)
	{
		perror("listen error");
		return 1;
	}

	while(1)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd,
			(struct sockaddr *)&client_addr, &addrlen);
		printf("new client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		if(client_fd == -1)
		{
			printf("accept error\n");
		}
		else
		{
			count++;
			/* init mutex and cond */
			pthread_mutex_init(&t_lock[count], NULL);
			pthread_cond_init(&t_cond[count], NULL);

			/* producer_thread create */
			pthread_create(&thread_id, NULL, producer_thread, (void *)&client_fd);
			pthread_detach(thread_id);
			/* consumer_thread create */
			pthread_create(&thread_id, NULL, consumer_thread, (void *)&client_fd);
			pthread_detach(thread_id);
		}
	}
	return 0;
}

