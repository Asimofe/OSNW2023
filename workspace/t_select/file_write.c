//fwrite.c

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
        int fd, ret;
        char buffer[1024];

        if(argc != 2){
                printf("usage : %s file \n", argv[0]);
                exit(0);
        }
        //파일을 쓰기 전용으로 연다.
        fd = open(argv[1], O_WRONLY|O_APPEND);

        if(fd < 0){
                perror("file erorr ");
                exit(0);
        }

        while(1){

                //한줄 처리를 위해서 fgets를 사용
                fgets(buffer, sizeof(buffer), stdin);

                //파일에 기록 
                ret = write(fd, buffer, strlen(buffer));

                if(ret < 0){
                        perror("write error ");
                        exit(0);
                }
                //quit문자열은 종료
                if(!strcmp(buffer, "quit\n")) break;
        }

        close(fd);
        return 0;
}
