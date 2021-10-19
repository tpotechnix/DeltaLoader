#ifndef _SYSINT_H
#define _SYSINT_H

unsigned SysIntLock(void);
void SysIntUnlock(unsigned flags);
void SysIntEnable(void);

#endif /* _SYSINT_H */
