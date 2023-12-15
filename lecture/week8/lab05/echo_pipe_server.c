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

#define MAXLINE 1024
#define PORTNUM 3600

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	pid_t pid;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];
	char *result;
	struct sockaddr_in client_addr, server_addr;
	int fd_list[3][2];
	int fd_list2[3][2];
	int i;

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
	
	int client_num = 0;
	result = malloc(1);
	result[0] = '\0';
	
	while(1)
	{
		if(client_num < 3) {
			addrlen = sizeof(client_addr);
			client_fd = accept(listen_fd,
				(struct sockaddr *)&client_addr, &addrlen);
			if(client_fd == -1)
			{
				printf("accept error\n");
				break;
			}
			printf("Connected Client %s(%d)\n", inet_ntoa(client_addr.sin_addr),
                                              client_addr.sin_port);
		
			
			if (pipe(fd_list[client_num]) < 0) {
                       		perror("pipe error : ");
                       		return 1;
        	 	}
         		if (pipe(fd_list2[client_num]) < 0) {
                      		perror("pipe error : ");
                      		return 1;
                	}
			client_num++;
			pid = fork();
		}
		if(pid == 0)
		{
			close( listen_fd );
			close(fd_list[client_num-1][0]);
			close(fd_list2[client_num-1][1]);
			memset(buf, 0x00, MAXLINE);
			if((readn = read(client_fd, buf, MAXLINE)) > 0)
			{
				printf("Read Data %s(%d) : %s",
                           	               inet_ntoa(client_addr.sin_addr),
                                               client_addr.sin_port,
                                               buf);
				write(fd_list[client_num-1][1], (void *)buf, sizeof(buf));
				memset(buf, 0x00, MAXLINE);
				if (read(fd_list2[client_num-1][0], (void *)buf, sizeof(buf)) > 0) {
					write(client_fd, buf, sizeof(buf));
                			close(client_fd);
					break;
				}	
			}
			return 0;
		}
		else if(pid > 0) {
			if (client_num < 3) {
			       close(client_fd);
			       continue;
			}

			for (i = 0; i < 3; i++) {
				if (read(fd_list[i][0], (void *)buf, sizeof(buf)) > 0) {
                                	buf[strlen(buf)-1] = ' ';
					result = strcat(result, buf);
				}
			}
			result[strlen(result)-1] = '\n';
			
			for(i = 0; i < 3; i++) {
				write(fd_list2[i][1], (void *)result, strlen(result));
			}
			printf("Broadcasting) message: %s", result);
			break;
		}
	}
	return 0;
}

