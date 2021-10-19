
#include <string.h>
#include <stdlib.h>
#include "SysOut.h"

#include <stdint.h>

#include "Sio.h"
#include "SysOut.h"

extern int gStdInOutHdl;

static int WriteBuf(int fd, char *buffer, int nBytes) {

   int bytesLeft;
   int rv;

   bytesLeft = nBytes;

   while (bytesLeft > 0) {
      rv = SioWrite(fd, buffer, bytesLeft);
      if (rv < 0) {
         return rv;
      }
      bytesLeft = bytesLeft - rv;
      buffer = buffer + rv;
   }

   return nBytes;
}

void SysOutFlush(void) {

   int bytesLeft;

   /* Wait till printed completely */
   while (1) {
      bytesLeft = SioNumWrite(gStdInOutHdl);
      if (bytesLeft <= 0) {
         break;   /* done (or error) */
      }
   }
}

/*-----------------------------------------------------------------------------
*  Decimal output of int32_t
*/
void SysOutDec32(
   int32_t val
   ) {

   char buf[12];

   itoa(val, buf, 10);
   WriteBuf(gStdInOutHdl, buf, strlen(buf));
}

/*-----------------------------------------------------------------------------
*  Output one string
*/
void SysOutStr(char *buf) {

   WriteBuf(gStdInOutHdl, buf, strlen(buf));
}

/*-----------------------------------------------------------------------------
*  Output on char
*/
void SysOutChar(
   char c
   ) {

   WriteBuf(gStdInOutHdl, &c, 1);
}
