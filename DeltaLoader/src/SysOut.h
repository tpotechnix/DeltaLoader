#ifndef _SYSOUT_H
#define _SYSOUT_H

#include <stdint.h>

void SysOutFlush(void);
void SysOutDec32(int32_t val);
void SysOutStr(char *buffer);
void SysOutChar(char c);

#endif /* _SYSOUT_H */
