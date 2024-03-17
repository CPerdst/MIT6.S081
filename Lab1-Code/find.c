#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"


void fmtpath(char *path){
    char *p;
    p = path + strlen(path);
    if(*(--p) == '/'){
        *p = '\x00';
    }
}
//path fmt: ./ ../ / /.. 
char* getname(char *path){
    static char buf[DIRSIZ+1], *p;
    for(p=path+strlen(path); p>=path && *p!='/'; p--);
    if(strlen(++p) > DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(buf), ' ', DIRSIZ-strlen(buf));
    return buf;
}

char* findstr(char *str, char *s){
    char *p;
    int flag, i, slen=strlen(s);
    p = str;
    if(slen > strlen(str)) return (char *)0;
    while(p + slen <= str + strlen(str)){
        flag = 1;
        for(i=0; i<slen; i++){
            if(*(p+i) != *(s+i)){
                flag = 0;
                break;
            }
        }
        if(flag){
            return p;
        }
        p++;
    }
    return (char *)0;
}

void find(char *path, char *name){
    int fd;
    struct stat st;
    char buf[512], *p;
    struct dirent de;
    int flag=0;
    if((fd = open(path, 0)) < 0){
        fprintf(2, "error: open %s failed\n", path);
        exit(1);
    }
    if(fstat(fd, &st) < 0){
        fprintf(2, "error: fetch stat failed\n");
        close(fd);
        exit(1);
    }
    fmtpath(path);
    if(st.type == T_DIR){
        if(sizeof(path)+1+DIRSIZ > sizeof(buf)){
            printf("lenth too long\n");
            exit(1);
        }
        strcpy(buf, path);
        // 遍历目录
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0 || flag++ < 2)
                continue;
            // 检查是否为目录
            // 是目录则首先判断该目录是否含有指定字符串（若有，打印目录地址），然后迭代该目录
            // 非目录则判断是否含有指定字符串（若有，则打印地址）
            p = buf + strlen(path);
            *p++ = '/';
            strcpy(p, de.name);
            *(p+strlen(de.name)) = '\x00';
            if(stat(buf, &st) < 0){
                printf("error: open %s failed\n", path);
            }
            if(findstr(de.name, name) != (char *)0){
                printf("%s\n", buf);
            }
            if(st.type == T_DIR){
                find(buf, name);
            }
        }
    }else{
        fprintf(2, "error: %s is not a dir.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]){
    int pid, res, *a;
    // //find -> ls .
    a=&res;
    if(argc < 2){
        if((pid = fork()) == 0){
            char *argv1[] = {"./ls", "."};
            if(exec("./ls", argv1) < 0){
                printf("error: ls .\n");
                exit(1);
            }
            exit(0);
        }
        wait(&res);
        exit(0);
    }
    //find src -> ls src
    if(argc < 3){
        if((pid = fork()) == 0){
            char *argv2[] = {"./ls", argv[1]};
            if(exec("./ls", argv2) < 0){
                printf("error: ls .\n");
                exit(1);
            }
            exit(0);
        }
        wait(a);
        exit(0);
    }

    if(argc > 3){
        printf("usage: find path name\n");
        exit(1);
    }
    find(argv[1], argv[2]);

    exit(0);
}