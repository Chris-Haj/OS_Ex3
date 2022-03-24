#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int running = 1;

void handler(){
    printf("I don't want to quit\n");
}

//int c, char *argv[]
int main() {

    signal(SIGINT,handler);
    while(1);
    return 0;
}
