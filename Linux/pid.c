#include <unistd.h>
#include "../pid.h"

unsigned my_pid(void)
{
  return getpid();
}
