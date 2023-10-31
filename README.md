[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/DLipn7os)
# Intro to Xv6
OSN Monsoon 2023 mini project 2

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the first 3 specifications. 
- work inside the `networks/` directory for the Specification 4 as mentioned in the assignment.
- Instructions to test the xv6 implementations are given in the `initial_xv6/README.md` file. 

- You are free to delete these instructions and add your report before submitting. 

<<<<<<< HEAD
# Scheduling Report (Specification 3)

To view the report containing the description of my implementation of the two scheduling algorithms, the comparison of performance of all three schedulers and the MLFQ line graph for processes, click [here](/initial-xv6/README.md)

# Networking Report (Specification 4)

To view the report for the difference between my implementation of data sequencing and acknowledgents and that of the standard TCP one, and how I would extend my implementation to incorporate flow control, click [here](/networks/README.md)

# Assumptions
1. For `sigalarm`, the `handler` function will have no arguments defined (taking a hint from the `alaramtest.c` file). I have allowed the return type to be anything (by using `void`) but it will not be used anyway.
2. For `sigalarm`, to disable any alarm handling, pass any negative value to the `handler` or `interval` argument of `sigalarm()`
3. For FCFS scheduling, note that on running `usertests`, the test `preempt` will not work (if 1 <`CPUS` <= 3) as FCFS requires preemption to be disabled. Moreover, in this test the difference in creation times of the parent and its three children is not enough to count as a whole CPU tick, so once the parent process (the process for the `preempt` test) goes to sleep during the `read` syscall, it will never be woken up again, and its three children will be scheduled to run on the CPUS (as many as can be obviously). Since the parent never becomes runnable again, it is never scheduled even, so it can never kill the children and the test won't pass as the children essentially loop forever. Similarly, for 1 CPU, it will fail `usertests` on the test `killstatus`.
4. Because we're using macros to change the scheduler, I am using the `touch` command to provide for the `make` command to change the files using those macros, so as to to not have to do a `make clean` before changing the scheduler (or the number of CPUS for that matter).
5. I have modified `proc.c:procdump()` to print the creation time of the process as well, and the priority queue of the process if MLFQ is used as the scheduler.
6. For MLFQ, I also demote a process if it went for I/O, just before using up its whole slice, and so the kernel only got back control after the whole time slice was used up.
=======
- Answer all the theoretical/analysis-based questions (for PBS scheduler and the concurrency questions) in a single `md `file.
- You may delete these instructions and add your report before submitting. 

# CoW fork implementation

- in kalloc, set the reference count of the page to one.
- in uvmcopy, simply copy over parent's page table to the child, with PTE_W cleared for both. do not copy memory. increase reference count of page by 1.
- in uvmunmap, decrease reference count of page by 1. -- ARE YOU SURE IT'S HERE?
- in kfree, free only if reference count of page is 0.
- handle page faults in usertrap, by copying the page for the faulting process, updating its pte and adding the pte_w flag, decrease the ref count for the original page.

- use sfence_vma() to flush the TLB whenever any page table is changed
page fault codes: https://pdos.csail.mit.edu/6.828/2019/lec/l-usingvm.pdf


# Assumptions
1. If the scheduler is not set to `PBS` and the system call `set_priority` is used, or if `set_priority` is passed an invalid `pid` or `new_priority`, it will simply return -1.
2. `In case two or more processes have the same priority, we use the number of times the process has been scheduled to break the tie.` I assume here that a process that has been scheduled for a lesser number of times will be chosen.
<<<<<<< HEAD
>>>>>>> 94d8205 (added gitignore)
=======
3. For CoW fork, I mark the pages of the parent as read-only when it forks a child, even if some of those pages had write permission.
>>>>>>> d4a821c (implemented CoW fork, not working rn)
