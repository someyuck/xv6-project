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
  return 0; // not reached
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
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
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
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

// these 4 steps are implemented in trap.c:usertrap() :
// trap into kernel every 'interval' CPU ticks (we anyways trap at every tick)
// then trap into user space to execute the handler function (as alarm timer goes off)
// trap back into kernel 
// restore previous state (call sigreturn()) and trap back into user space to continue executing
uint64 sys_sigalarm(void)
{
  uint64 interval;
  uint64 handler;
  argaddr(0, &interval);
  argaddr(1, &handler);
  struct proc *p = myproc();
  p->alarmInterval = interval;
  p->handler = handler;
  return 0;
}

uint64 sys_sigreturn(void)
{
  struct proc *p = myproc();
  memmove(p->trapframe, p->breakoffTF, PGSIZE); // restore original trapframe
  kfree((void*)p->breakoffTF);
  p->breakoffTF = 0;
  p->isAlarmOn = 0;
  p->ticksElapsed = 0;
  return p->trapframe->a0;
}

uint64 
sys_set_priority(void)
{
  uint64 pid;
  uint64 newPriority;
  argaddr(0, &pid);
  argaddr(1, &newPriority);
  if(pid <= 0 || newPriority < 0 || newPriority > 100)
    return -1;

  int ret = -1; // assumption
  #ifdef PBS

  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if(p->pid == pid)
    {
      ret = p->StaticPriority;
      p->StaticPriority = newPriority;
      p->RBI = DefaultRBI;
      updateDP(p);
      release(&p->lock);
      break;
    }
    release(&p->lock);
  }
  if(newPriority < ret)
    yield();
  #endif

  return ret;
}