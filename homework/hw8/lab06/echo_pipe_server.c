#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define MAXLINE 1024
#define PORTNUM 3600

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	pid_t pid;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];
	struct sockaddr_in client_addr, server_addr;

	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==-1)
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
	
	//부모 프로세스에게 데이터를 전달하기 위해 pipe 사용
	int fd[3][2];
	for(int i = 0; i < 3; i++){
		pipe(fd[i]);
	}

	char cat_string[MAXLINE];
	memset(cat_string, 0x00, MAXLINE);

	for(int i = 0; i < 3; i++)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
		
		if(client_fd == -1)
		{
			printf("accept error\n");
			break;
		}
		
		pid = fork();
		//자식 프로세스
		if(pid == 0)
		{
			close(listen_fd);

			char result[MAXLINE];
			memset(result, 0x00, MAXLINE);
			int cycle = 0;
			

			while((readn = read(client_fd, buf, MAXLINE)) > 0)
			{
				if(strncmp(buf, "quit", 4) == 0) // "quit" 문자열을 입력 받으면 break
				{
					printf("Disconnected Client %d\n", client_addr.sin_port);
					break;
				}
				
				cycle++; // 몇 번째 데이터인지 카운트
				buf[strlen(buf) - 1] = '\0'; // 개행 문자 제거

				// 클라이언트의 IP 주소와 Port 번호, 데이터 내용 출력  
				printf("Client: %d, Read Data: %s\n", client_addr.sin_port,	buf);


				//부모 프로세스에게 데이터를 송신
				write(fd[i][1], buf, strlen(buf));
				
				memset(buf, 0x00, MAXLINE);
			}

			//부모 프로세스로부터 데이터를 전달받기 위해 pipe 사용
			sleep(1); // 부모 프로세스가 데이터를 전달할 수 있도록 1초간 sleep
			read(fd[i][0], result, MAXLINE);

			printf("Data From child Process: %s\n", result); // 이어진 문자열 출력
			write(client_fd, result, strlen(result)); // 이어진 문자열 client에 전송
			sleep(3); // client가 데이터를 받을 수 있도록 3초간 sleep

			close(client_fd);
			return 0;
		}
		// 부모 프로세스
		else if( pid > 0) {
			
			close(client_fd);
			memset(buf, 0x00, MAXLINE);

			//자식 프로세스가 전달한 데이터를 읽는다.
			read(fd[i][0], buf, MAXLINE);
			

			if(strncmp(buf, "quit", 4) == 0){ // "quit" 문자열을 파이프로 전달받으면 자식프로세스에게 전달
			
				write(fd[i][1], cat_string, strlen(cat_string));
			}
			else{
				//자식 프로세스가 전달한 데이터를 이어 붙인다.
				strcat(cat_string, buf);
				strcat(cat_string, " ");
				printf("cat_string: %s\n", cat_string);
			}
		}
	}
	for(int i = 0; i < 3; i++)
	{
		write(fd[i][1], cat_string, strlen(cat_string));
	}

	wait(NULL);

	return 0;
}
