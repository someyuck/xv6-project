#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("usage: setpriority <pid> <priority>\n");
    exit(0);
  }

  uint64 pid = 0, newPriority = 0;
  int pidLen = strlen(argv[1]);
  int priLen = strlen(argv[2]);

  // convert strings to integers
  for (int i = 0; i < pidLen; i++)
    pid = pid * 10 + (argv[1][i] - '0');
  for (int i = 0; i < priLen; i++)
    newPriority = newPriority * 10 + (argv[2][i] - '0');

  printf("lens: %d %d\n", pid, newPriority);

  int ret = set_priority(pid, newPriority);
  if (ret == -1)
    printf("set_priority error\n");
  else
    printf("old priority of process: %d\n", ret);

  exit(0);
}