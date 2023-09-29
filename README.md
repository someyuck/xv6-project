[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/DLipn7os)
# Intro to Xv6
OSN Monsoon 2023 mini project 2

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the first 3 specifications. 
- work inside the `networks/` directory for the Specification 4 as mentioned in the assignment.
- Instructions to test the xv6 implementations are given in the `initial_xv6/README.md` file. 

- You are free to delete these instructions and add your report before submitting. 

## Implementation
1. for MLFQ scheduling, I add a field to `struct proc` that will indicate the priority queue number (0 to 3), another field to track ticks used in current time slice, and another field to track waiting time (ticks)
--- `proc.h` : adding new fields, and macros to store time slices and waiting time limits per PQ.
--- `proc.c` : `allocproc()` : adding new processes to the end of PQ0.
                `scheduler()` : if currently running process has exhausted time slice or relinquished control (say for i/o), search for a process in the highest non-empty PQ (in RR fashion) to run. Reset wait time to 0 for the selected process.
--- `trap.h` : `usertrap()` : update ticks (running ticks if it is running and waiting ticks (iterate over the `proc` array for this) if waiting (runnable)). push a process to next higher PQ if wait time exceeds limit, and reset wait time to 0.



## Assumptions
1. For `sigalarm`, the `handler` function will have no arguments defined (taking a hint from the `alaramtest.c` file). I have allowed the return type to be anything (by using `void`) but it will not be used anyway.
2. For `sigalarm`, to disable any alarm handling, pass any negative value to the `handler` or `interval` argument of `sigalarm()`
3. For FCFS scheduling, note that on running `usertests`, the test `preempt` will not work (if `CPUS` <= 3) as FCFS requires preemption to be disabled. Moreover, in this test the difference in creation times of the parent and its three children is not enough to count as a whole CPU tick, so once the parent process (the process for the `preempt` test) goes to sleep during the `read` syscall, it will never be woken up again, and its three children will be scheduled to run on the CPUS (as many as can be obviously). Since the parent never becomes runnable again, it is never scheduled even, so it can never kill the children and the test won't pass as the children essentially loop forever.
4. Because we're using macros to change the scheduler, I am using the `touch` command to procide for the `make` command to change the files using those macros, so as to to not have to do a `make clean` before changing the macro.
5. I have modified `proc.c:procdump()` to print the creation time of the process as well.