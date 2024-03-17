#include "kernel/types.h"
#include "user/user.h"

// int main(){
//     // int fd_F = pipe(0
//     int pid;
//     int fd[2];
//     if(pipe(fd) < 0){
//         printf("error: pipe\n");
//         exit(1);
//     }
//     if((pid = fork()) == 0){
//         char buf[50];
//         while(read(fd[0], buf, 50) != 0){
//             printf("child has received father's msg...\nsending response to father...\n");
//             if(strcmp(buf, "hello child!") == 0){
//                 if(write(fd[1], "hello father!", 13) != 13){
//                     printf("error: child write\n");
//                     exit(1);
//                 }
//                 printf("%d: received ping\n", getpid());
//                 break;
//             }
//         }
//         exit(0);
//     }else{
//         char buf[50];
//         if(write(fd[1], "hello child!", 12) != 12){
//             printf("error: father write\n");
//             exit(1);
//         }
//         printf("father has sent msg to child...\nreceiving child msf...\n");
//         while (read(fd[0], buf, 50) != 0){
//             printf("received child's msg...\n");
//             if(strcmp(buf, "hello father!") == 0){
//                 printf("%d: received pong\n", getpid());
//                 break;
//             }
//         }
//     }
//     exit(0);
// }


void perror(const char *fmt){
    printf(fmt);
    exit(1);
}

int main(){
    int pid, res;
    int Fpipe[2], Cpipe[2];
    if((res = pipe(Fpipe)) < 0){
        printf("Error: open Fpipe error\n");
    }
    if((res = pipe(Cpipe)) < 0){
        printf("Error: open Cpipe error\n");
    }
    if((pid = fork()) == 0){
        //Child Code
        char buf[50];
        memset(buf, 0 ,sizeof(buf));
        while(read(Fpipe[0], buf, 3) == 3){
            if(strcmp(buf, "msg") != 0){
                printf("Child msg received fault\n");
                exit(1);
            }
            printf("%d: received ping\n", getpid());
            sleep(20);
            if(write(Cpipe[1], "msg", 3) != 3){
                printf("Child write error\n");
                exit(1);
            }
            break;
        }
        exit(0);
    }else{
        //Father Code
        char buf[50];
        memset(buf, 0 ,sizeof(buf));
        if(write(Fpipe[1], "msg", 3) != 3){
            printf("Father write error\n");
            exit(1);
        }
        while(read(Cpipe[0], buf, 50) == 3){
            if(strcmp(buf, "msg") != 0){
                printf("Father msg received fault\n");
                exit(1);
            }
            printf("%d: received pong\n", getpid());
            break;
        }
    }
    exit(0);
}
