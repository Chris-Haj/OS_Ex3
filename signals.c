#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
int prime(int n){
    int flag = 0;
    if (n == 0 || n == 1)
        flag = 1;

    for (int i = 2; i <= n / 2; ++i) {
        if (n % i == 0) {
            flag = 1;
            break;
        }
    }


    if (flag == 0)
        return 1;
    else
        return 0;
}

bool isPrime(int num)
{
    int i; bool flag = 0;
// check if num can be devided
    for(i = 2; i <= num/2; i++) {
        if( num%i == 0) {
            flag = 1;
            break;
        }
    }
    if(flag == 0) return true; // num is prime
    return false; // num is not prime
}



int main(){
    srand(time(NULL));
    int fd[2];
    if(pipe(fd)==-1){
        perror("pipe error");
        exit(0);
    }
    pid_t pid = fork();
    if(pid==0){
        int childpid = getpid();
        close(fd[0]);
        write(fd[1],&childpid, sizeof(childpid));
        while(1){
            int random = rand();
            if(prime(random)==1){
                break;
            }
        }
        close(fd[1]);
    }
    else if(pid<0){
        perror("fork error");
        exit(1);
    }
    else{
        int childpid;
        close(fd[1]);
        read(fd[0],&childpid, sizeof(childpid));
        printf("Father pid is %d, child pid is %d\n",getpid(),childpid);
        int random = rand();

    }
    return 0;
}