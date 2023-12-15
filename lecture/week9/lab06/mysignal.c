#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo)
{
    printf("SIGINT received : %d\n", signo);
}

int main()
{
    int i = 0;
    signal(SIGINT, (void *)sig_handler);
    //signal(SIGINT, SIG_IGN);


    while(1)
    {        
        printf("%d\n", i);
        i++; 
        sleep(1);
    }    
    return 1;
}
