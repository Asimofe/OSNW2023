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
    if (argc != 4)
    {
   	 printf("Usage : %s [num1] [num2] [op]\n", argv[0]);
   	 return 1;
    }

    memset((void *)&sdata, 0x00, sizeof(sdata));
    sdata.left_num = atoi(argv[1]);
    sdata.right_num = atoi(argv[2]);
    sdata.op = argv[3][0];

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

    len = sizeof(sdata);
    sdata.left_num = htonl(sdata.left_num);
    sdata.right_num = htonl(sdata.right_num);
    sbyte = write(s, (char *)&sdata, len);
    if(sbyte != len)
    {
   	 return 1;
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
    printf("%d %c %d = %d\n", ntohl(sdata.left_num), sdata.op, ntohl(sdata.right_num), ntohl(sdata.result)); 

    close(s);
    return 0;
}
