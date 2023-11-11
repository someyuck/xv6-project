#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NFORK 10
#define IO 5

// set SP of CPU processes to 50, I/O processes to 25

int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    printf("usage: pbstest <option> (0 or 1)\n");
    exit(0);
  }
  if(argv[1][0] == '0')
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