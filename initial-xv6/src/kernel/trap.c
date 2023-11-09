#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

int AgingTimes[3] = {PQ1AgingTime, PQ2AgingTime, PQ3AgingTime};

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void)
{
  int which_dev = 0;

  if ((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();

  // save user program counter.
  p->trapframe->epc = r_sepc();

  if (r_scause() == 8)
  {
    // system call

    if (killed(p))
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sepc, scause, and sstatus,
    // so enable only now that we're done with those registers.
    intr_on(); 

    syscall();
  }
  else if ((which_dev = devintr()) != 0)
  {
    // ok
  }
  else if(r_scause() == 15) // store/write page fault
  {
    // handle page faults for CoW fork implementation

    // STVAL has the faulting virtual address, meaning its value and its page needs to be checked.
    // if the PTE is marked as CoW, create a copy and assign to p (faulting process) (just add PTE_W if refcount is only 1).
    // else if its read-only (and not CoW), meaning it was never marked with PTE_W, kill the process

    uint64 va = r_stval();
    if(va < 0 || va >= MAXVA)
    {
      printf("usertrap: invalid virtual address\n");
      setkilled(p);
      goto ifkilled;
    }

    pte_t *pte = walk(p->pagetable, va, 0);
    if(pte == 0 || !(*pte & PTE_V))
    {
      printf("usertrap: page not present\n");
      setkilled(p);
      goto ifkilled;
    }
    if(!(*pte & PTE_COW))
    {
      printf("usertrap: store attempt to read-only memory\n");
      setkilled(p);
      goto ifkilled;
    }

    uint64 pa = PTE2PA(*pte);
    // allocate a new page and copy old page to this
    char *new;
    if((new = kalloc()) == 0)
    {
      printf("usertrap: kalloc: out of memory\n");
      setkilled(p);
      goto ifkilled;
    }

    uint64 flags = PTE_FLAGS(*pte);
    memmove(new, (char*)pa, PGSIZE);
    
    // change the page for the faulting process
    *pte = PA2PTE(new) | flags;
    *pte &= ~PTE_COW;
    *pte |= PTE_W;

    kfree((void*)pa); // for old page
    sfence_vma(); // flush the TLB
  }
  else
  {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    setkilled(p);
  }
ifkilled:
  if (killed(p))
    exit(-1);
  
  // give up the CPU if this is a timer interrupt. -- yield only if FCFS and PBS are not used
  if (which_dev == 2)
  {
    // if MLFQ is used:
    #ifdef MLFQ

    // code for mlfqtest.c
    if(strncmp(p->name, "mlfqtest", 16) == 0)
    {
      acquire(&p->lock);
      printf("%d %d %d run\n", ticks, p->pid, p->priorityQueue);
      release(&p->lock);
    }

    // update waiting time of runnable processes 
    for (int i = 0 ; i < NPROC; i++) // add assumption of order of pushing processes
    {
      acquire(&proc[i].lock);
      if (proc[i].state == RUNNABLE && &proc[i] != p) 
      {
        proc[i].waitingTicks++;
        // push process to end of next higher PQ if it waits more than its aging limit
        if (proc[i].priorityQueue > 0 && proc[i].waitingTicks >= AgingTimes[proc[i].priorityQueue - 1])
        {
          // code for mlfqtest.c
          if(strncmp(proc[i].name, "mlfqtest", 16) == 0)
            printf("%d %d %d before promotion\n", ticks-1, proc[i].pid, proc[i].priorityQueue);

          proc[i].priorityQueue--;
          proc[i].waitingTicks = 0;
          proc[i].curSliceRunTicks = 0;

          // code for mlfqtest.c
          if(strncmp(proc[i].name, "mlfqtest", 16) == 0)
            printf("%d %d %d after promotion\n", ticks, proc[i].pid, proc[i].priorityQueue);
        }
      }
      release(&proc[i].lock);
    }
    #endif

    #ifdef PBS
    // update WTime of RUNNABLE processes and STime of SLEEPING processes, and RTime of current process

    p->RTime++;
    updateRBI(p);
    updateRBI(p);

    // code for pbstest.c
    if(strncmp(p->name, "pbstest", 16) == 0)
    {
      acquire(&p->lock);
      printf("%d %d %d %d   %d %d %d\n", ticks, p->pid, p->StaticPriority, p->DynamicPriority, p->RBI, p->RTime, p->STime, p->WTime);
      release(&p->lock);
    }
  
    for (int i = 0 ; i < NPROC; i++)
    {
      acquire(&proc[i].lock);
      if(proc[i].state == RUNNABLE && &proc[i] != p)
        proc[i].WTime++;
      else if(proc[i].state == SLEEPING)
        proc[i].STime++;

      updateRBI(&proc[i]);
      updateDP(&proc[i]);
      release(&proc[i].lock);
    }
    #endif

    if (p->handler >= 0)
    {
      // update process' CPU ticks since last call to alarm handler
      if (p->alarmInterval >= 0)
        p->ticksElapsed = (p->ticksElapsed + 1) % p->alarmInterval;
      else
        p->ticksElapsed = p->ticksElapsed + 1;

      if (p->ticksElapsed == 0 && p->isAlarmOn == 0) // call alarm handler function
      {
        p->isAlarmOn = 1;
        p->breakoffTF = kalloc(); // save current trapframe to restore in sigreturn()
        memmove(p->breakoffTF, p->trapframe, PGSIZE);
        p->trapframe->epc = p->handler; // jump to handler
      }
    }
    #if !defined(FCFS) && !defined(PBS)
    yield();
    #endif
  }

  usertrapret();
}

//
// return to user space
//
void usertrapret(void)
{
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to uservec in trampoline.S
  uint64 trampoline_uservec = TRAMPOLINE + (uservec - trampoline);
  w_stvec(trampoline_uservec);

  // set up trapframe values that uservec will need when
  // the process next traps into the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp(); // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.

  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to userret in trampoline.S at the top of memory, which
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64))trampoline_userret)(satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();

  if ((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if (intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if ((which_dev = devintr()) == 0)
  {
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt. -- only if FCFS is not used
  #if !defined(FCFS) && !defined(PBS)
  if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();
  #endif

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void clockintr()
{
  acquire(&tickslock);
  ticks++;
  update_time();
  // for (struct proc *p = proc; p < &proc[NPROC]; p++)
  // {
  //   acquire(&p->lock);
  //   if (p->state == RUNNING)
  //   {
  //     printf("here");
  //     p->rtime++;
  //   }
  //   // if (p->state == SLEEPING)
  //   // {
  //   //   p->wtime++;
  //   // }
  //   release(&p->lock);
  // }
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr()
{
  uint64 scause = r_scause();

  if ((scause & 0x8000000000000000L) &&
      (scause & 0xff) == 9)
  {
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if (irq == UART0_IRQ)
    {
      uartintr();
    }
    else if (irq == VIRTIO0_IRQ)
    {
      virtio_disk_intr();
    }
    else if (irq)
    {
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if (irq)
      plic_complete(irq);

    return 1;
  }
  else if (scause == 0x8000000000000001L)
  {
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if (cpuid() == 0)
    {
      clockintr();
    }

    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  }
  else
  {
    return 0;
  }
}
