#ifndef SIO_H_
#define SIO_H_

#include "Fifo.h"

#define SIO0   0
#define SIO1   1

int SioOpen(int sioNr, int baud, TFifo *rxFifo, TFifo *txFifo);

int SioRead(int hdl, char *buf, int len);
int SioWrite(int hdl, char *buf, int len);

int SioNumRead(int hdl);
int SioNumWrite(int hdl);

int SioClose(int hdl);

#endif /*SIO_H_*/