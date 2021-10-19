#include "SysInt.h"

unsigned __attribute__((naked)) SysIntLock(void) {

   /* Read PRIMASK and lock interrupts */
   __asm("mrs    r0, PRIMASK\n"
         "cpsid  i\n"       /* disable ints */
         "bx     lr");
   /* to avoid comiler warning - bx is return in assembly */
   return 0;  /* never reached */
}

void __attribute__((naked)) SysIntUnlock(unsigned flags) {

   __asm("msr  PRIMASK, r0\n"
         "bx   lr");
}

void SysIntEnable(void) {

   __asm("cpsie i");
}
