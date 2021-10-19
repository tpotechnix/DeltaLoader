#ifndef _FIFO_H_
#define _FIFO_H_

#include <stdbool.h>

typedef struct TFifo {
    char *data;
    int wr;
    int rd;
    int len;
} TFifo;   

void FifoInit(TFifo *fifo, char *data, int len);
void FifoClear(TFifo *fifo);
int FifoBytesFilled(TFifo *fifo);
int FifoBytesFree(TFifo *fifo);
bool FifoWriteByte(TFifo *fifo, char byte);
bool FifoReadByte(TFifo *fifo, char *pByte);
int FifoWriteBuf(TFifo *fifo, char *buf, int len);
int FifoReadBuf(TFifo *fifo, char *buf, int maxlen);

/* Variants with Interrupts locked */
void FifoClearLocked(TFifo *fifo);
int FifoBytesFilledLocked(TFifo *fifo);
int FifoBytesFreeLocked(TFifo *fifo);
bool FifoWriteByteLocked(TFifo *fifo, char byte);
bool FifoReadByteLocked(TFifo *fifo, char *pByte);
int FifoWriteBufLocked(TFifo *fifo, char *buf, int len);
int FifoReadBufLocked(TFifo *fifo, char *buf, int len);

#endif