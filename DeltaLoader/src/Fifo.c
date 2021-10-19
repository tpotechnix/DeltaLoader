#include "Fifo.h"
#include "SysInt.h"
#include <stdbool.h>

void FifoInit(TFifo *fifo, char *data, int len) {

   fifo->data = data;
   fifo->len = len;
   fifo->rd = 0;
   fifo->wr = 0;
}

void FifoClear(TFifo *fifo) {

   fifo->wr = 0;
   fifo->rd = 0;
}

int FifoBytesFilled(TFifo *fifo) {
   
   if (fifo->wr >= fifo->rd) {
       return (fifo->wr - fifo->rd);
   } else {
       return (fifo->len - (fifo->rd - fifo->wr));
   }
}

int FifoBytesFree(TFifo *fifo) {
   
   return (fifo->len - FifoBytesFilled(fifo) - 1);
}

bool FifoIsEmpty(TFifo *fifo) {
   
   return (fifo->rd == fifo->wr);
}

bool FifoIsFull(TFifo *fifo) {
   
   return (FifoBytesFree(fifo) == 0);
}

bool FifoWriteByte(TFifo *fifo, char byte) {
    
    if (FifoBytesFree(fifo) == 0) {
        return false;
    }
    
    fifo->data[fifo->wr] = byte;
    fifo->wr++;
    fifo->wr %= fifo->len;

    return true;
}

bool FifoReadByte(TFifo *fifo, char *pByte) {

    if (FifoBytesFilled(fifo) == 0) {
        return false;
    }
    
    *pByte = fifo->data[fifo->rd];
    fifo->rd++;
    fifo->rd %= fifo->len;
    
    return true;
}

int FifoWriteBuf(TFifo *fifo, char *buf, int len) {

   int i = 0;

   while ((i < len) && FifoWriteByte(fifo, buf[i])) {
      i++;
   } 
   
   return i;
}

int FifoReadBuf(TFifo *fifo, char *buf, int maxlen) {
    
    int i = 0;
    
    while ((i < maxlen) && FifoReadByte(fifo, &buf[i])) {
        i++;
    }
    
    return i;
}

/* Variants with Interrupts locked */
void FifoClearLocked(TFifo *fifo) {

   unsigned flags;

   flags = SysIntLock();
   FifoClear(fifo);
   SysIntUnlock(flags);
}

int FifoBytesFilledLocked(TFifo *fifo) {
    
   int rv;
   unsigned flags;
   
   flags = SysIntLock();
   rv = FifoBytesFilled(fifo);
   SysIntUnlock(flags);
   
   return rv;
}

int FifoBytesFreeLocked(TFifo *fifo) {
    
   int rv;
   unsigned flags;
   
   flags = SysIntLock();
   rv = FifoBytesFree(fifo);
   SysIntUnlock(flags);
   
   return rv;
}

bool FifoWriteByteLocked(TFifo *fifo, char byte) {
    
   bool rv;
   unsigned flags;
   
   flags = SysIntLock();
   rv = FifoWriteByte(fifo, byte);
   SysIntUnlock(flags);
   
   return rv;
}

bool FifoReadByteLocked(TFifo *fifo, char *pByte) {
    
   bool rv;
   unsigned flags;
   
   flags = SysIntLock();
   rv = FifoReadByte(fifo, pByte);
   SysIntUnlock(flags);
   
   return rv;
}

int FifoWriteBufLocked(TFifo *fifo, char *buf, int len) {
    
   int rv;
   unsigned flags;
   
   flags = SysIntLock();
   rv = FifoWriteBuf(fifo, buf, len);
   SysIntUnlock(flags);
   
   return rv;
}

int FifoReadBufLocked(TFifo *fifo, char *buf, int len) {
    
   int rv;
   unsigned flags;
   
   flags = SysIntLock();
   rv = FifoReadBuf(fifo, buf, len);
   SysIntUnlock(flags);
   
   return rv;
}



