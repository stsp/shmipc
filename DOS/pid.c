#include <dpmi.h>
#include "../pid.h"

unsigned my_pid(void)
{
  __dpmi_regs r = {};

  r.x.ax = 0x82;  // dosemu's getpid
  __dpmi_simulate_real_mode_interrupt(0xe6, &r);
  return r.x.ax | (r.x.bx << 16);
}
