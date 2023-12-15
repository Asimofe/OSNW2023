#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define PORTNUM 3600
#define MAXLINE 1024
#define MAXSTR 4096

union semun{
	int val;
};

typedef struct cal_data {
	int left_num;
	int right_num;
	char op;
	int result;
	short error;
	char data[128];
	struct tm sever_time;
}cdata;

int shmid;
int semid;

void *shared_memory = NULL;

cdata *shm_data;
time_t now_time;

struct tm *current_time;
struct sembuf semopen = {0, -1, SEM_UNDO};
struct sembuf semclose = {0, 1, SEM_UNDO};

void timer_handler(int signo){
	time(&now_time);
	current_time = localtime(&now_time);
	shm_data->sever_time = *current_time;
	if(semop(semid, &semclose, 1) == -1){
		perror("semop error");
		exit(0);
	}
	alarm(5);
}

int main(int argc, char **argv)
{
	int listen_fd,client_fd;
	pid_t pid;
	socklen_t addrlen;
	int readn;
	struct sockaddr_in client_addr, server_addr;
	int shm_key_index = 1000;
	int sem_key_index = 3000;
	char buf[MAXLINE];

	union semun sem_union;

	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind error");
		return 1;
	}
	if(listen(listen_fd, 5) == -1)
	{
		perror("listen error");
		return 1;
	}

	signal(SIGCHLD, SIG_IGN);

	/* parent Server */
	while(1)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
		if(client_fd == -1)
		{
			printf("accept error\n");
			break;
		}

		shmid = shmget((key_t)shm_key_index, sizeof(cdata), 0666|IPC_CREAT);
		if(shmid == -1){
			perror("shmget failed(main) : ");
			return 1;
		}

		semid = semget((key_t)sem_key_index, 1, IPC_CREAT|0666);
		if(semid == -1){
			perror("shmget failed : ");
			return 1;
		}

		sem_union.val = 0;
		if(semctl(semid, 0, SETVAL, sem_union) == -1){
			printf("semctl error\n");
			return 0;
		}

		pid = fork();
		if(pid == 0)
		{
			close(listen_fd);
			pid = fork();

			/* Producer server */
			if(pid == 0){
				printf("Connected_producer: %s(%d)\n", inet_ntoa(client_addr.sin_addr), getpid());
				cdata local_data;
				char *token;

				shared_memory = shmat(shmid, NULL, 0);
				if(shared_memory == (void *)-1){
					perror("shmat failed : ");
					return 1;
				}

				shm_data = (cdata *)shared_memory;

				/* 시그널 SIGALRM이 발생하면 timer_handler 함수 호출 */
				signal(SIGALRM, timer_handler);
				

				while(1){
					memset(buf, 0x00, MAXLINE);

					/* 사용자로부터 데이터 read */
					if(read(client_fd , buf, MAXLINE) <= 0){
						printf("Read Error(Producer)\n");
						return 0;
					}
					alarm(0);

					/* 사용자 입력 토큰화 및 구조체 cal_data에 저장 */
					printf("Read Data: %s", buf);
					token = strtok(buf, " \n");
					if(token != NULL) local_data.left_num = atoi(token);
					token = strtok(NULL, " \n");
					if(token != NULL) local_data.right_num = atoi(token);
					token = strtok(NULL, " \n");
					if(token != NULL) local_data.op = token[0];
					token = strtok(NULL , " \n");
					if(token != NULL) strcpy(local_data.data , token);

					/* 사용자가 quit를 입력시 종료 */
					if(strcmp(local_data.data, "quit") == 0){
						strcpy(shm_data->data, local_data.data);
						if(semop(semid, &semclose, 1) == -1){
							perror("semop error");
							return 0;
						}
						return 1;
					}

					/* 구조체 cal_data에 저장된 사용자의 입력으로 사칙연산 수행 */
					local_data.result = 0; local_data.error = 0;

					switch(local_data.op){
						case '+':
							local_data.result = local_data.left_num + local_data.right_num;
							break;
						case '-':
							local_data.result = local_data.left_num - local_data.right_num;
							break;
						case '*':
						case 'x':
							local_data.result = local_data.left_num * local_data.right_num;
							break;
						case '/':
							if(local_data.right_num == 0){
								local_data.error = 1;
								break;
							}
							local_data.result = local_data.left_num / local_data.right_num;
							break;
						default:
							local_data.error = -1;
					}
				
					time_t now_time;
					time(&now_time);
					struct tm *current_time = localtime(&now_time);
					local_data.sever_time = *current_time;
					
					/* 사칙연산 결과, 문자열을 shm에 write */
					shm_data->left_num = local_data.left_num;
					shm_data->right_num = local_data.right_num;
					shm_data->op = local_data.op;
					shm_data->result = local_data.result;
					shm_data->error = local_data.error;
					strcpy(shm_data->data, local_data.data);
					shm_data->sever_time = local_data.sever_time;

					/* Consumer에서 read가능하도록 설정 */
					if(semop(semid, &semclose, 1) == -1){
						perror("semop error : ");
						return 1;
					}
					alarm(5);
				}
				close(client_fd);
				return 1;
			}
			/* Consumer server */
			else{
				printf("Connected_consumer: %s(%d)\n", inet_ntoa(client_addr.sin_addr), getpid());
				cdata local_data;
				char result_buf[20], time_buf[50];

				shmid = shmget((key_t)shm_key_index, sizeof(int), 0666);
				if(shmid == -1){
					perror("shmget failed : ");
					exit(0);
				}
				semid = semget((key_t)sem_key_index, 0, 0666);
				if(semid == -1){
					perror("semget failed : ");
					exit(0);
				}
				shared_memory = shmat(shmid, NULL, 0);
				if(shared_memory == (void *)-1){
					perror("shmat failed : ");
					exit(0);
				}
				shm_data = (cdata *)shared_memory;

				while(1){

					memset(buf, 0x00, MAXLINE);
					memset(result_buf, 0x00, 20);
					memset(time_buf, 0x00, 50);

					/* shared memory wait and open */
					if(semop(semid, &semopen, 1) == -1){
						perror("semop error : ");
					}

					/* producer server에서 shm에 write한 내용을 read */
					local_data.left_num = shm_data->left_num;
					local_data.right_num = shm_data->right_num;
					local_data.op = shm_data->op;
					strcpy(local_data.data, shm_data->data);
					local_data.result = shm_data->result;
					local_data.error = shm_data->error;
					local_data.sever_time = shm_data->sever_time;
					
					/* 사용자가 quit를 입력시 종료 */
					if(strcmp(local_data.data, "quit") == 0){
						write(client_fd, "quit", 5);
						return 1;
					}

					
					/* make output string */
					sprintf(buf, "%d %d %c ", local_data.left_num, local_data.right_num, local_data.op);
					if(local_data.error == 0){
						sprintf(result_buf, "%d ", local_data.result);
						strcat(buf, result_buf);
					}else if(local_data.error == -1){
						strcat(buf, "Calculate error ");
					}else{
						strcat(buf, "Can't divide zero ");
					}
					//strftime(time_buf, sizeof(time_buf), "%a %b %d %H:%M:%S %Y ", &(local_data.sever_time));
					strftime(time_buf, sizeof(time_buf), "%Y %b %d %a %H:%M:%S ", &(local_data.sever_time));
					strcat(buf, time_buf);

					/* write client */
					write(client_fd, buf, strlen(buf));
				}

			}
			return 0;
		}
		else if( pid > 0){
			shm_key_index++;
			sem_key_index++;
			close(client_fd);
		}
	}
	return 0;
}

