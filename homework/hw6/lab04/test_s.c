#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

//#define PORT 3600

struct cal_data
{
        int left_num;
        int right_num;
        char op;
        int result;
        short int error;
};

int main(int argc, char **argv)
{
        struct sockaddr_in client_addr, sock_addr;
        int listen_sockfd, client_sockfd;
        int addr_len;
        struct cal_data rdata;
        int left_num, right_num, cal_result;
        short int cal_error;

	unsigned char *bytePtr;
        int size;

        if( (listen_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("Error ");
                return 1;
        }

        memset((void *)&sock_addr, 0x00, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_addr.sin_port = htons(atoi(argv[1]));

        if( bind(listen_sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
        {
                perror("Error ");
                return 1;
        }

        if(listen(listen_sockfd, 5) == -1)
        {
                perror("Error ");
                return 1;
        }
	addr_len = sizeof(client_addr);
        for(;;)
        {
                client_sockfd = accept(listen_sockfd,(struct sockaddr *)&client_addr, &addr_len);
                if(client_sockfd == -1)
                {
                        perror("Error ");
                        return 1;
                }
		printf("New Client Connect : %s\n", inet_ntoa(client_addr.sin_addr));
		
		//-----------------------------------------------------------------
		while(1){

                	read(client_sockfd, (void *)&rdata, sizeof(rdata));
			if(rdata.op != '$'){
			rdata.result = 0;
			//수신한 데이터를 hexdump로 출력
               		bytePtr = (unsigned char *)&rdata;
	               	size = sizeof(struct cal_data);
			printf("r: ");
              	 	for(int i = 0; i < size; i++){
                       		printf("%02x ", bytePtr[i]);
                	}
	                printf("\n");
			//--------------------------------



                	cal_result = 0;
	                cal_error = 0;

        	        left_num = ntohl(rdata.left_num);
                	right_num = ntohl(rdata.right_num);

	                switch(rdata.op)
        	        {
                	        case '+':
                        	        cal_result = left_num + right_num;
                                	break;
	                        case '-':
        	                        cal_result = left_num  - right_num;
	                                break;
	                        case 'x':
	                                cal_result = left_num * right_num;
	                                break;
	                        case '/':
	                                if(right_num == 0)
	                                {
	                                        cal_error = 2;
	                                        break;
	                                }
	                                cal_result = left_num / right_num;
	                                break;
				default:
	                                cal_error = 1;
		
	                }
	                rdata.result = htonl(cal_result);
	                rdata.error = htons(cal_error);
			
			//연산 결과 출력
			printf("%d %c %d = %d\n", left_num, rdata.op, right_num, cal_result);
			
			//송신할 데이터 hexdump로 출력
			printf("s: ");
			for(int i = 0; i < size; i++){
				printf("%02x ", bytePtr[i]);
			}
			printf("\n");
			//-----------------------------
	                write(client_sockfd, (void *)&rdata, sizeof(rdata));
			}
			else if(rdata.op == '$'){
				printf("Client Disconnected\n");
				break;
			}

		}
		close(client_sockfd);
		//---------------------------------------------------------------
        }

        close(listen_sockfd);
        return 0;
}

