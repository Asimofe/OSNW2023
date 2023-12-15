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
#include <fcntl.h>

#define MAXBUF 1024
#define MAXLINE 1024
#define PORTNUM 3600

int flag = 0;

int main(int argc, char **argv)
{
	int listen_fd, client_fd[3];
	pid_t pid;
	socklen_t addrlen;
	int readn, i = 0, c_num = 0;
	char buf[MAXLINE];
	char temp[MAXLINE];
	struct sockaddr_in client_addr, server_addr;

    int rfd, wfd, rfd_child, wfd_child;

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

	for(int i = 0; i<3; i++){
		addrlen = sizeof(client_addr);
		client_fd[i] = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
		if(client_fd[i] == -1)
		{
			printf("accept error\n");
			break;
		}
		printf("Client Connected %s(%d)\n", inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
	}
	while(flag < 4)
	{
		
        // 파이프 생성
        mkfifo("/tmp/PtoC", S_IRUSR|S_IWUSR);
        mkfifo("/tmp/CtoP", S_IRUSR|S_IWUSR);

		pid = fork();
		if(pid == 0) //Child Process
		{
			close( listen_fd );
			memset(buf, 0x00, MAXLINE);

            if((rfd_child = open("/tmp/PtoC", O_RDWR)) == -1)
            {
                perror("rfd error");
                return 0;
            }
            if ((wfd_child = open("/tmp/CtoP", O_RDWR)) == -1)
            {
                perror("wfd error");
                return 0;
            }

            // 클라이언트로부터 문자열 수신
            if((read(client_fd[c_num], buf, MAXLINE)) <= 0){
                perror("Read Fail");
                return 0;
            }

            buf[strlen(buf)-1] = ' ';
			printf("Read Data %s(%d) : %s\n", inet_ntoa(client_addr.sin_addr), getpid(), buf);
			

            write(wfd_child, buf, strlen(buf));
			flag++;
			
			//close(client_fd[c_num]);
			return 0;

		}
		else if( pid > 0){ // Parent Process
			
			if( (rfd = open("/tmp/CtoP", O_RDWR)) == -1){
				perror("rfd error - parent");
				return 0;
			}
			if( (wfd = open("/tmp/PtoC", O_RDWR)) == -1){
				perror("wfd error - parent");
				return 0;
			}
			read(rfd, buf, MAXLINE);

			strcat(temp, buf);
			// i++;
			if(flag == 3){
				strcat(temp, "\n");
				for(int i = 0; i<3; i++){
					write(client_fd[i], temp, strlen(temp));
				}
				break;
			}
		}
		c_num++;
		close(client_fd[c_num]);
	}
	return 0;
}
