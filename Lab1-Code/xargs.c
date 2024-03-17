#include "kernel/types.h"
#include "user/user.h"

#define BUFSIZ 20

// void transarg(char *arg, char *cmd[], int argc){
//     int pid, res, len=strlen(arg), i;
//     char buf[1024], p=buf;
//     for(i=0; i<argc; i++){
//         len+=strlen(cmd[i]);
//     }
//     if(len >= 1024){
//         fprintf(2, "error: buf's len too long.\n");
//         exit(1);
//     }

//     if((pid = fork()) == 0){
//         if(exec() < 0){
//             fprintf(2, "error: exec error occured.\n");
//             exit(1);
//         }
//     }
//     wait(&res);
//     return;
// }

//  cmd args... | xargs cmd
//  目的：将cmd args...的输出作为标准输入，
//  然后按行读入传递给xargs后的cmd，并且以
//  1、循环按行读入

int getline(char *buf, char *arg, int max){
    int numChars = 0;
    if(buf == 0 || arg == 0 || max <= 0)
        return -1;
    while(*buf != '\0' && *buf != '\n' && numChars < max){
        *arg++ = *buf++;
        numChars++;
    }
    *arg = '\0';
    if(*buf == '\n'){
        numChars++;
    }
    return numChars;
}

void xargs(int argc, char **argv){
    char  *args[32], buffer[BUFSIZ+1], arg[BUFSIZ+1], cache[BUFSIZ+1], combine[BUFSIZ*2+2], *p1, *p2;
    int size=0, len, i;
    memset(buffer, 0, sizeof buffer);
    memset(arg, 0, sizeof arg);
    memset(cache, 0, sizeof cache);
    memset(combine, 0, sizeof combine);
    // read(0, buffer, BUFSIZ);
    // printf("%d: %s", getline(buffer, arg, BUFSIZ), arg);
    for(i=1; i<argc && argv[i]; i++){
        args[i-1] = argv[i];
    }
    while(read(0, buffer, BUFSIZ) > 0){
        // printf("buffer: %s\n",buffer);
        size = 0;
        p1=buffer;
        while((len = getline(p1+size, arg, BUFSIZ-size)) > 0){
            size+=len;
            if(strlen(arg) == len){
                memset(cache, 0, sizeof cache);
                strcpy(cache, arg);
                // printf("copy successfully\n");
            }else{
                if(*cache){
                    p1 = cache;
                    p2 = combine;
                    while (*p1)
                        *p2++ = *p1++;
                    p1 = arg;
                    while (*p1)
                        *p2++ = *p1++;
                    // printf("asd\n");
                    // printf("%p : %d : %d : %s\n", p1, size, len, combine);
                    args[i-1] = combine;
                    if(fork() == 0){
                        exec(args[0], args);
                    }else{
                        wait(0);
                    }
                    memset(combine, 0, sizeof combine);
                    memset(cache, 0, sizeof cache);
                }else{
                    // printf("dsa\n");
                    // printf("%p : %d : %d : %s\n", p1, size, len, arg);
                    args[i-1] = arg;
                    if(fork() == 0){
                        exec(args[0], args);
                    }else{
                        wait(0);
                    }
                }
                
                
            }
            memset(arg, 0, sizeof arg);
        }
        memset(buffer, 0, sizeof buffer);
    }
}

int main(int argc, char **argv){
    // int fd[2];
    // if(pipe(fd) < 0){
    //     fprintf(2, "error.\n");
    //     exit(1);
    // }
    // if(fork() == 0){
    //     close(0);
    //     dup(fd[0]);
    //     close(fd[0]);
    //     close(fd[1]);
    //     xargs(argc, argv);
    // }else{
    //     write(fd[1], "aaaa aaaaa\nbbbbbbbb\ncccccccccc\nsdhac\n", 31);
    //     close(fd[0]);
    //     close(fd[1]);
    //     wait(0);
    // }
char  *args[32], buffer[BUFSIZ+1], arg[BUFSIZ+1], cache[BUFSIZ+1], combine[BUFSIZ*2+2], *p1, *p2;
    int size=0, len, i;
    memset(buffer, 0, sizeof buffer);
    memset(arg, 0, sizeof arg);
    memset(cache, 0, sizeof cache);
    memset(combine, 0, sizeof combine);
    // read(0, buffer, BUFSIZ);
    // printf("%d: %s", getline(buffer, arg, BUFSIZ), arg);
    for(i=1; i<argc && argv[i]; i++){
        args[i-1] = argv[i];
    }
    while(read(0, buffer, BUFSIZ) > 0){
        // printf("buffer: %s\n",buffer);
        size = 0;
        p1=buffer;
        while((len = getline(p1+size, arg, BUFSIZ-size)) > 0){
            size+=len;
            if(strlen(arg) == len){
                memset(cache, 0, sizeof cache);
                strcpy(cache, arg);
                // printf("copy successfully\n");
            }else{
                if(*cache){
                    p1 = cache;
                    p2 = combine;
                    while (*p1)
                        *p2++ = *p1++;
                    p1 = arg;
                    while (*p1)
                        *p2++ = *p1++;
                    // printf("asd\n");
                    // printf("%p : %d : %d : %s\n", p1, size, len, combine);
                    args[i-1] = combine;
                    if(fork() == 0){
                        exec(args[0], args);
                    }else{
                        wait(0);
                    }
                    memset(combine, 0, sizeof combine);
                    memset(cache, 0, sizeof cache);
                }else{
                    // printf("dsa\n");
                    // printf("%p : %d : %d : %s\n", p1, size, len, arg);
                    args[i-1] = arg;
                    if(fork() == 0){
                        exec(args[0], args);
                    }else{
                        wait(0);
                    }
                }
                
                
            }
            memset(arg, 0, sizeof arg);
        }
        memset(buffer, 0, sizeof buffer);
    }
    exit(0);
}