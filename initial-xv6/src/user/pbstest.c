#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/*
**USE WITH CPUS=1**

Creates 5 processes, each of which just consume cpu time.
prints the process running at each tick, along with its static and dynamic priority, along with other information.

Usage: run "pbstest 0" for basic testing where the processes simply use cpu time
       run "pbstest 1" to have each process periodically increment its static priority
       run "pbstest 2" to have each process periodically decrement its static priority

I've modified kernel/trap.c (lines 131 to 137) for this purpose, which will print
the PID and priorities of the process that was running at that tick.

Copy the output data and enter it into a file ../../pbsAnalysis.txt (create it if not present)
Also remove the first line, as it represents the process of this program and not the 5 children it creates, which we want to analyse.
Then run ../..pbstest.py
*/

#define N 5

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    printf("usage: pbstest <option>\n");
    exit(0);
  }

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
      int defStat = 50;
      for(long int i = 0 ; i < 10000000000 ; i++) 
      {
        if(i % 100000000 == 0)
        {
          if(argv[1][0] == '1')
          {
            if(defStat >= 0 && set_priority(getpid(), --defStat) == -1)
              // printf("set_priority error\n")
              ;
          }  
          else if(defStat <= 100 && argv[1][0] == '2')
          {
            if(set_priority(getpid(), ++defStat) == -1)
              // printf("set_priority error\n")
              ;
          }
        }
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