# 题目要求

在这个作业中，您将添加一个系统调用`sysinfo`，它收集有关正在运行的系统的信息。系统调用采用一个参数：一个指向`struct sysinfo`的指针（参见 _kernel/sysinfo.h_）。内核应该填写这个结构的字段：`freemem`字段应该设置为空闲内存的字节数，`nproc`字段应该设置为`state`字段不为`UNUSED`的进程数。我们提供了一个测试程序`sysinfotest`；如果输出“**sysinfotest: OK**”则通过。
# testcall()
sysinfotest.c由三部分组成，首先来看testcall()
``` c
void
sinfo(struct sysinfo *info) {
  if (sysinfo(info) < 0) {
    printf("FAIL: sysinfo failed");
    exit(1);
  }
}

void
testcall() {
  struct sysinfo info;
  
  if (sysinfo(&info) < 0) {
    printf("FAIL: sysinfo failed\n");
    exit(1);
  }
  if (sysinfo((struct sysinfo *) 0xeaeb0b5b00002f5e) !=  0xffffffffffffffff) {
    printf("FAIL: sysinfo succeeded with bad argument\n");
    exit(1);
  }
}
```
通过这段代码可以知道，作者其实是想让我们实现报错则返回-1，并且传递的sysinfo指针地址还需要存在于对应进程的虚拟地址空间之中。
``` c
uint64
sys_sysinfo(void){
  uint64 va;
  struct proc *p;
  argaddr(0, &va);
  p = myproc();
  if(!walkaddr(p->pagetable, va)){
    return -1;
  }
  return 0;
}
```
我们首先通过argaddr()获取系统调用的第一个参数，然后获取当前占用cpu的进程。
通过提供一个页表跟虚拟地址，walkaddr()函数可以将虚拟地址转换到对应的物理地址，若页表无法转换，则返回0。
至此，第一个testcall测试已经可以通过。

# testmem()
testmem函数首先调用countfree，查看下面countfree代码可以知道，代码首先是获取了当前进程结构体内的sz字段，然后重复调用sbrk(PGSIZE)进行内存页的申请，直到内存页被申请完毕，sbrk会返回-1，然后跳出循环。
然后通过sysinfo系统调用来获取当前剩余内存。因为刚刚的while循环已经将剩余内存页全部申请完毕，所以这里的可用空间应该为0。
然后调用sbrk(-((uint64)sbrk(0) - sz0));来恢复最开始的sz
``` c
int
countfree()
{
  uint64 sz0 = (uint64)sbrk(0);
  struct sysinfo info;
  int n = 0;
  while(1){
    if((uint64)sbrk(PGSIZE) == 0xffffffffffffffff){
      break;
    }
    n += PGSIZE;
  }
  sinfo(&info);
  if (info.freemem != 0) {
    printf("FAIL: there is no free mem, but sysinfo.freemem=%d\n",
      info.freemem);
    exit(1);
  }
  sbrk(-((uint64)sbrk(0) - sz0));
  return n;
}
```
testmem剩余部分就是首先获取当前freemem，然后调用sbrk(PGSIZE)来使进程获取一块内存页，然后再获取freemem，比较当前freemem是否等于第一次freemem-PGSIZE，然后再调用sbrk(-PGSIZE)，使进程释放一块内存页，然后比较两个freemem是否相等。
获取freemem可以通过直接遍历内核中用于维护内存页的链表kmem.freelist来获得。（注意：题目要获取的是字节数，而不是剩余内存页数，所以应该return 内存页数 * PGSIZE）
``` c
// sysproc.c
uint64
sys_sysinfo(void){
  uint64 va, cache;
  struct proc *p;
  argaddr(0, &va);
  p = myproc();
  if(!walkaddr(p->pagetable, va)){
    return -1;
  }
  cache = gfreemem();
  copyout(p->pagetable, va, (char*)&cache, 8);
  return 0;
}
// kalloc.c
uint64 gfreemem(){
  struct run *r;
  int a;
  r = kmem.freelist;
  a = 0;
  while(r){
    a++;
    r = r->next;
  }
  return a * PGSIZE;
}
```

# testproc()

内核源码的proc.c中，有一个用于维护所有进程的数组proc\[NPROC\]，在内核初始化的时候，会调用procinit函数对其进行初始化。
我们需要获取到当前进程的数量，所以只需要遍历proc\[NPROC\]数组，获取其中state为UNUSED的进程数，然后用NPROC减去未使用的进程数，就是当前使用的进程数。
``` c
// proc.c
void
procinit(void)
{
  struct proc *p;
  
  initlock(&pid_lock, "nextpid");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");

      // Allocate a page for the process's kernel stack.
      // Map it high in memory, followed by an invalid
      // guard page.
      char *pa = kalloc();
      if(pa == 0)
        panic("kalloc");
      uint64 va = KSTACK((int) (p - proc));
      kvmmap(va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
      p->kstack = va;
  }
  kvminithart();
}
// sysproc.c
uint64
sys_sysinfo(void){
  uint64 va, cache;
  struct proc *p;
  argaddr(0, &va);
  p = myproc();
  if(!walkaddr(p->pagetable, va)){
    return -1;
  }
  cache = gfreemem();
  copyout(p->pagetable, va, (char*)&cache, 8);
  cache = gnproc();
  copyout(p->pagetable, va+8, (char*)&cache, 8);
  return 0;
}
// proc.c
uint64 gnproc(){
  struct proc *p;
  uint32 res=0;
  // 遍历proc[NPROC]，获取未使用进程的数量
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED){
      res++;
    }
  }
  return NPROC - res;
}
```