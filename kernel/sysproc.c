#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}
uint64
sys_getsyscallcount(void){
struct proc *p=myproc();
return p->syscall_count;
}

extern struct proc proc[NPROC]; // Declare the process table

uint64
sys_getstatus(void) {
    int pid;
    uint64 status_addr; // Use uint64 for user address
    char status[16];

    // Fetch arguments
    argint(0, &pid);
    argaddr(1, &status_addr);

    // Iterate through the process table
    struct proc *p = 0;
    for (p = proc; p < &proc[NPROC]; p++) {
        if (p->pid == pid) {
            // Get the process state as a string
            safestrcpy(status, p->state == UNUSED ? "unused" :
                              p->state == SLEEPING ? "sleeping" :
                              p->state == RUNNABLE ? "runnable" :
                              p->state == RUNNING ? "running" :
                              p->state == ZOMBIE ? "zombie" : "unknown", 16);

            // Copy the status string back to user space
            if (copyout(myproc()->pagetable, status_addr, (char *)status, sizeof(status)) < 0)
                return -1;
            return 0;
        }
    }
    return -1; // PID not found
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
uint64
sys_procdump(void) {
    procdump();
    return 0;
}