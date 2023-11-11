#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/*
modified schedulertest.c

**USE WITH CPUS=1** (only so that output is easier to comprehend)

There are two options with this program: `pbstest 0` and `pbstest 1`, entered on the command line.
Both cases print the process (pid) running at each tick, its static and dynamic priorities, along with the valus of RBI, RTime, STime, and WTime
I've modified and kernel/proc.c (lines 673 to 675) for this purpose.

1. pbstest 0
Creates 10 processes, the first 5 of which are I/O bound and the other five just consume cpu time.

2. pbstest 1.
Creates 5 processes, each of which just consumes CPU time.

Copy the output data and enter it into the file ../../pbstest.txt (create it if not present).
Also remove the lines containing the least pid (3 if pbstest is the first program to be run after xv6 boots up).
This is the pid of the process of the pbstest program, and is not part of the analysis.
pids are printed in the second column.
Change the name of the plot (and type of plot too if you want) in ../../pbstest.py, and then run it.
*/

#define NFORK 10
#define IO 5

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    printf("usage: pbstest <option> (0 or 1)\n");
    exit(0);
  }
  if(argv[1][0] == '0') // set SP of I/O processes to 25
  {
    int n, pid;
    int wtime, rtime;
    int twtime = 0, trtime = 0;
    for (n = 0; n < NFORK; n++)
    {
      pid = fork();
      if (pid < 0)
        break;
      if (pid == 0)
      {
        if (n < IO)
        {
          set_priority(getpid(), 25);
          sleep(200); // IO bound processes
        }
        else
        {
          for (volatile int i = 0; i < 1000000000; i++)
          {
          } // CPU bound process
        }
        // printf("Process %d finished\n", n);
        exit(0);
      }
    }
    for (; n > 0; n--)
    {
      if (waitx(0, &wtime, &rtime) >= 0)
      {
        trtime += rtime;
        twtime += wtime;
      }
    }
    printf("Average rtime %d,  wtime %d\n", trtime / NFORK, twtime / NFORK);
  }
  else if(argv[1][0] == '1') // 5 CPU bound processes with 3 distinct static priorities
  {
    int n, pid;
    int wtime, rtime;
    int twtime = 0, trtime = 0;
    for (n = 0; n < 5; n++)
    {
      pid = fork();
      if (pid < 0)
        break;
      if (pid == 0)
      {
        set_priority(getpid(), 25 + 5 * (n / 2)); // 25, 30, 35
        for (volatile int i = 0; i < 1000000000; i++)
        {
        } // CPU bound process
        // printf("Process %d finished\n", n);
        exit(0);
      }
    }
    for (; n > 0; n--)
    {
      if (waitx(0, &wtime, &rtime) >= 0)
      {
        trtime += rtime;
        twtime += wtime;
      }
    }
    printf("Average rtime %d,  wtime %d\n", trtime / NFORK, twtime / NFORK);
  }

  exit(0);
}