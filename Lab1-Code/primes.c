#include "kernel/types.h"
#include "user/user.h"

int sqrt(int num){
    int i,res;
    if(num == 0) return 0;
    for(i=1; i<num; i++){
        res = i*i;
        if(res>num) return i-1;
    }
    exit(1);
}

int prime(int num){
    int i;
    if(num == 2 || num == 1 || num == 3) return 1;
    for(i=2; i<=sqrt(num); i++){
        if(num%i == 0){
            return 0;
        }
    }
    return 1;
}

void vecprint(int *vec){
    int i;
    for(i=0; vec[i]; i++){
        printf("%d ", vec[i]);
    }
    printf("\n");
}

void newproc(int *vec, int num){
    int fd[2], pid, res, i;
    if(pipe(fd) == -1){
        printf("pipe alloc error\n");
    }
    printf("prime %d\n", vec[0]);
    if(!vec[1]) exit(0);
    for(i=1; vec[i]; i++){
        if(vec[i]%num != 0){
            write(fd[1], vec+i, 1);
        }
    }
    close(fd[1]);

    if((pid=fork()) == 0){
        int newvec[40], ptr=0;
        memset(newvec, 0, sizeof(newvec));
        while(read(fd[0], newvec+ptr++, 1) != 0);
        close(fd[0]);
        newproc(newvec, num+1);
    }else{
        pid = wait(&res);
        // printf("%d pid has killed\n", pid);
        exit(0);
    }
}

int main(){
    int i, vec[40];
    memset(vec, 0, sizeof(vec));
    for(i=2; i<37; i++){
        vec[i-2] = i;
    }
    newproc(vec, 2);
    exit(0);
}