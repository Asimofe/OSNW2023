#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAXLINE 1024
#define PORTNUM 3600
#define CLIENTMAX 3600

fd_set readfds, allfds;

typedef struct cal_data{
	int data1,data2;
	char op;
	int result;
	short error;
}cal_data;

typedef struct connection{
	int index;
	int client_fd;
	char data[MAXLINE];
	cal_data calData;
	struct connection *next;
}node;

node *head = NULL;
node *tail = NULL;

/* 연결리스트(클라이언트 리스트)에 새로운 클라이언트 추가 */
void addConnection(int index, int client_fd){
	node *box = (node*)malloc(sizeof(node));
	box->index = index;
	box->client_fd = client_fd;
	box->next = NULL;
	box->calData.error = -2;
	if(head == NULL){
		head = box;
		tail=box;
		return ;
	}
	else{
		tail->next = box;
		tail = box;
	}
}

/* 각 클라이언트에 저장되어 있는 문자열 데이터를 이어 붙여서 반환 */
void getString(char *str){
	node *temp = head;
	while(temp != NULL){
		if(temp->calData.error != -2){
			strcat(str, temp->data);
			strcat(str, " ");
		}
		temp = temp->next;
	}
}

/* 클라이언트 종료 시 index를 통해서 해당 클라이언트 삭제 */
void deleteConnection(int index){
	node *temp = head;
	node *prev = NULL;
	if(temp != NULL && temp->index == index){
		head = temp->next;
		FD_CLR(temp->client_fd, &readfds);
		close(temp->client_fd);
		free(temp);
		printf("index(%d) - disconnected\n", index);
		return ;
	}
	prev = temp;
	temp = temp->next;
	while(temp != NULL){
		if(temp->index == index){
			if(tail->index == temp->index){
				prev->next = NULL;
				tail = prev;
				printf("index(%d) - disconnected\n", index);
			}
			else{
				prev->next = temp->next;
				printf("index(%d) - disconnected\n", index);
			}
			FD_CLR(temp->client_fd, &readfds);
			close(temp->client_fd);
			free(temp);
			return ;
		}
		prev = temp;
		temp = temp->next;
	}
	printf("Not Found!!\n");
}

/* 문자열을 자르고 계산한 후 데이터를 저장 */
void calculate(node *temp, char *buf){
	char *token;
	
	memset(temp->data , 0x00, MAXLINE);
	memset(&(temp->calData) , 0x00, sizeof(cal_data));

	/* data slicing */
	token = strtok(buf, " \n");
	if(token != NULL) temp->calData.data1 = atoi(token);
	token = strtok(NULL, " \n");
	if(token != NULL) temp->calData.data2 = atoi(token);
	token = strtok(NULL, " \n");
	if(token != NULL) temp->calData.op = token[0];
	token = strtok(NULL, " \n");
	if(token != NULL) strcpy(temp->data , token);

	/* quit process */
	if(strcmp(temp->data , "quit") == 0){
		write(temp->client_fd, "quit" , 5);
		deleteConnection(temp->index);
		return ;
	}
	switch(temp->calData.op){
		case '+':
			temp->calData.result = temp->calData.data1 + temp->calData.data2;
			break;
		case '-':
			temp->calData.result = temp->calData.data1 - temp->calData.data2;
			break;
		case 'x':
			temp->calData.result = temp->calData.data1 * temp->calData.data2;
			break;
		case '/':
			if(temp->calData.data2 == 0){
				temp->calData.error = 1;
				break;
			}
			temp->calData.result = temp->calData.data1 / temp->calData.data2;
			break;
		default:
			temp->calData.error = -1;
	}
}


int main(int argc, char **argv){
	int listen_fd, client_fd;
	pid_t pid;
	socklen_t addrlen;
	char buf[MAXLINE];
	char wbuf[MAXLINE];
	struct sockaddr_in client_addr, server_addr;
	char *str = (char*)malloc(MAXLINE);
	int index = 0;
	int MAXFD = 0;

	struct timeval timeout;
	int retval;

	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void*)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("bind error");
		return 1;
	}
	if(listen(listen_fd, 5) == -1){
		perror("listen error");
		return 1;
	}
	signal(SIGCHLD, SIG_IGN);
	
	/* fd_set 초기설정 */
	FD_ZERO(&allfds);
	FD_ZERO(&readfds);
	FD_SET(listen_fd, &readfds);
	MAXFD = listen_fd;

	while(1)
	{
		memset(str, 0x00, MAXLINE);
		FD_ZERO(&allfds);
		node *temp;
		
		/* fd setting and timout setting */
		allfds = readfds;
		timeout.tv_sec = 300;
		timeout.tv_usec = 0;

		addrlen = sizeof(client_addr);
		
		//select
		retval = select(MAXFD+1, &allfds, NULL, NULL, &timeout);
		//select event process
		if(retval == -1){
			perror("select fail");
			return 1;
		}else if(retval == 0){
			printf("SELECT TIME OUT\n");
			continue;
		}else{
			if(FD_ISSET(listen_fd, &allfds)){
				// client accept
				client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
				if(client_fd == -1)
				{
					printf("accept error\n");
					break;
				}
				/* 새로운 클라이언트를 readfds에 추가 */
				FD_SET(client_fd, &readfds);

				/* 만약 client_fd가 MAXFD보다 크면 MAXFD 갱신 */
				if(client_fd > MAXFD) MAXFD = client_fd;
				
				addConnection(index, client_fd);
				printf("connected-%s / index : %d\n", inet_ntoa(client_addr.sin_addr), index);
				index++;
			}
			/* 모든 연결리스트를 순회하면서 FD_ISSET으로 확인 */
			temp = head;
			while(temp != NULL){
				if(FD_ISSET(temp->client_fd, &allfds)){
					memset(buf, 0x00, MAXLINE);
					if( (read(temp->client_fd, buf, MAXLINE)) <= 0){
						perror("read error");
						
					}
					printf("Read Data(from index(%d)) : %s", temp->index, buf);
					calculate(temp , buf);
					getString(str);
					// 모든 클라이언트에게 최종 문자열 전송
					// error값에 따라 서로 다른 형식으로 데이터 전송
					node *wtemp = head;
					while(wtemp != NULL){
						memset(wbuf, 0x00, MAXLINE);
						if(wtemp->calData.error == 1){
							sprintf(wbuf , "%d %d %c Zero-Division-Error %s", wtemp->calData.data1, wtemp->calData.data2, wtemp->calData.op, str); 
						}else if(wtemp->calData.error == -1){
							sprintf(wbuf , "%d %d %c Operator-Error %s", wtemp->calData.data1, wtemp->calData.data2, wtemp->calData.op, str);
						}
						else if(wtemp->calData.error == -2){
							sprintf(wbuf, "No-Input-Data %s" , str);
						}
						else{
							sprintf(wbuf, "%d %d %c %d %s", wtemp->calData.data1, wtemp->calData.data2, wtemp->calData.op, wtemp->calData.result, str);
						}
						write(wtemp->client_fd, wbuf, strlen(wbuf));
						wtemp = wtemp->next;
					}
					break;
				}
				temp = temp->next;
			}
		}
	}
	return 0;
}

