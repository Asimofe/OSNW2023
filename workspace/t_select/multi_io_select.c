//multiIO_select.c 

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define FD_SIZE 3
#define BUF_SIZE 8

int main(int argc, char *argv[]){
        int i, n, ret, fd_count, end_count = 0;
        int fds[FD_SIZE];
        char buf[BUF_SIZE] = {0,};
        struct timeval tv;

        fd_set retfds, readfds;

        //3개의 파일의 이름을 받는다.
        if(argc != 4){
                printf("usage : %s file1 file2 file3\n", argv[0]);
                return 0;
        }

        //readfds를 공집합으로 만든다. 
        FD_ZERO(&readfds);

        for(i = 0; i < FD_SIZE; i++){
                fds[i] = open(argv[i+1],O_RDONLY);
                if(fds[i] >= 0){
                        //readfds 집합에 파일디스크립터 fds[i]를 추가한다. 
                        FD_SET(fds[i], &readfds);
                        //맨 마지막 파일열린 파일 디스크립터가 가장 큰 값을 갖는다.
                        fd_count = fds[i];
                }
        }

        while(1){

                //readfds를 retfds로 옮기는 이유는 select를 통해서 readfds가 바뀔 수 있다.
                retfds = readfds;

                //열린 파일 디스크립터의 최대값 +1
                ret = select(fd_count + 1, &retfds, NULL, NULL, NULL);

                if(ret == -1){
                        perror("select error ");
                        exit(0);
                }

                for(i = 0; i < FD_SIZE; i++){
                        //만약 fds[i]에 대해서 읽을 준비가 된 파일 디스크립터이면 
                        if(FD_ISSET(fds[i], &retfds)){
                                //읽고 출력해준다. 
                                while((n = read(fds[i], buf, BUF_SIZE)) > 0){

                                        //quit가 들어오면 fds[i] read 종료
                                        if(!strcmp(buf,"quit\n")){
                                                //readfds에서 지우고 
                                                FD_CLR(fds[i], &readfds);
                                                //파일디스크립터 close
                                                close(fds[i]);

                                                end_count++;
                                                if(end_count == FD_SIZE) 
                                                        exit(0);
                                                continue;
                                        }

                                        printf("fd[%d] - %s",fds[i], buf);
                                        memset(buf, 0x00, BUF_SIZE);
                                }
                        }
                }

        }
}
