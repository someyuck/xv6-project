#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/*
modified schedulertest.c

**USE WITH CPUS=1**

Creates 5 processes, each of which just consume cpu time.
prints the process running at each tick, along with the priority queue number.

I've modified kernel/trap.c (lines 89 to 96, 107 to 109, and 117 to 119) and kernel/proc.c (lines 611 to 617) for this purpose, which will print
the PID and priority queue number of the process that was running at that tick.

Copy the output data and enter it into a file ../../mlfqAnalysis.txt (create it if not present)
Also remove the first line, as it represents the process of this program and not the 5 children it creates, which we want to analyse.
Then run ../..mlfqtest.py
*/

#define N 5

int main()
{
  int n;
  int wtime, rtime;
  for(n = 0; n < N; n++)
  {
    int pid = fork();
    if(pid < 0)
    {
      printf("fork failed\n");
      exit(1);
    }
    if(pid == 0)
    {
      for(long int i = 0 ; i < 10000000000 ; i++) 
      {
      }
      // printf("process %d finished\n", n);
      exit(0);
    }
  }
  for(; n > 0 ; n--)
  {
    waitx(0, &wtime, &rtime);
  }

  exit(0);
}