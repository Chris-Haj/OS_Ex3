#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


void handler(){
    pid_t pid =fork();

    if(pid==0){
        printf("\nI don't want to quit\n");
        exit(0);
    }
    else if(pid<0){
        perror("fork error");
        exit(1);
    }
    else{
        wait(NULL);
    }
}

//int c, char *argv[]
int main() {

    signal(SIGINT,handler);
    while(1);
    return 0;
}
