#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 3600
#define IP "127.0.0.1"
#define MAX_BUF 1024

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
    struct sockaddr_in addr;
    int s;
    int len;
    int sbyte, rbyte;
    struct cal_data sdata;
    char input[MAX_BUF];

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
    {
   	 return 1;
    }
   
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if ( connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
    {
   	 printf("fail to connect\n");
   	 close(s);
   	 return 1;
    }
    
    while(1){
	    
	    ssize_t input_byte = read(0, input, sizeof(input));
	    if(input_byte == -1){
		    perror("read");
		    exit(-1);
	
	    }
	    input[input_byte -1] = '\0';
	    if(sscanf(input, "%d %d %c", &sdata.left_num, &sdata.right_num, &sdata.op) == 3){
	    }
	    else{
		    printf("Usage: [num1] [num2] [op]\n");
		    return 1;
	    }
		 
	
	    len = sizeof(sdata);
	    sdata.left_num = htonl(sdata.left_num);
	    sdata.right_num = htonl(sdata.right_num);
	    
	    sbyte = write(s, (char *)&sdata, len);
	    if(sbyte != len)
	    {
	   	 return 1;
	    }
	    if(sdata.op == '$'){
		    printf("Disconnected\n");
		    break;
	    }

	    rbyte = read(s, (char *)&sdata, len);
	    if(rbyte != len)
	    {
	   	 return 1;
	    }
	    if (ntohs(sdata.error != 0))
	    {
	   	 printf("CALC Error %d\n", ntohs(sdata.error));
	    }
	    printf("%d %c %d = %d\n", ntohl(sdata.left_num), 
			    sdata.op, ntohl(sdata.right_num), ntohl(sdata.result)); 
	
    
    }
    close(s);
    
    return 0;
}
