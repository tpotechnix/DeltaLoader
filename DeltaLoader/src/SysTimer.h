#ifndef _SYSTIMER_H
#define _SYSTIMER_H

#include <stdint.h>

typedef uint32_t (*TGetTimeUsFunc)(void *p);

uint32_t SysGetTimeMs(void);
uint32_t SysGetTimeUs(void);

int SysTimerConfig(TGetTimeUsFunc pGetTimeUsFunc, void *pGetTimeUsArg);
void SysTimerTickExpired(void);

#endif /* _SYSTIMER_H */

