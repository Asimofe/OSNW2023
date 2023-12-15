#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <stdlib.h>
#include <fcntl.h>


#define MAXLINE    1024

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
	int retval;
    char buf[MAXLINE];

	fd_set readfds;
	struct timeval timeout;
	timeout.tv_sec = 300;
	timeout.tv_usec = 0;

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(3600);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }
	while(1){
		memset(buf, 0x00, MAXLINE);

		/* fd set */
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		FD_SET(server_sockfd, &readfds);

		retval = select(256, &readfds, NULL, NULL, &timeout);
		if(retval == -1){
				perror("fail");
				return 0;
		}else if(retval == 0){
			perror("time out");
			return 0;
		}
		else{
			if(FD_ISSET(0, &readfds)){					
				if( read(0, buf, MAXLINE) <= 0 ){
					perror("read error");
					return 0;
				}
				write(server_sockfd, buf, MAXLINE);
			}
			else if(FD_ISSET(server_sockfd, &readfds)){
				if(read(server_sockfd, buf, MAXLINE) <= 0){
					perror("read error");
					return 0;
				}
				if(strcmp(buf, "quit") == 0){
					return 1;
				}	
				printf("Total String : %s\n" , buf);
			}
		}
    		
	}
    return 0;
}

