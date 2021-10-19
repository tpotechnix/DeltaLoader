#include "string.h"

#include "SysInt.h"
#include "SysTimer.h"

static uint32_t        sActTick = 0;
static TGetTimeUsFunc  spGetTimeUsFunc = 0;
static void            *spGetTimeUsArg = 0;

uint32_t SysGetTimeMs(void) {

   return sActTick;
}

uint32_t SysGetTimeUs(void) {

   if (spGetTimeUsFunc != 0) {
      return (spGetTimeUsFunc(spGetTimeUsArg));
   } else {
      return 0;
   }
}

int SysTimerConfig(TGetTimeUsFunc pGetTimeUsFunc, void *pGetTimeUsArg) {

   spGetTimeUsFunc = pGetTimeUsFunc;
   spGetTimeUsArg  = pGetTimeUsArg;

   return 0;
}

void SysTimerTickExpired(void) {

   sActTick++;
}
