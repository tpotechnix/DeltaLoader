#include "Sio.h"
#include "Fifo.h"
#include "SysInt.h"

#include "stm32f0xx_hal.h"

#include <stdbool.h>

#define NUM_SIO         2

#define VALID_HDL(hdl)  (((hdl) > 0) && \
                         ((hdl) < NUM_SIO) && \
                          (sSio[(hdl)].isOpen))
#define REG(reg)        (sio->base->reg)

struct Sio {
   bool isOpen;
   TFifo *rxFifo;
   TFifo *txFifo;
   USART_TypeDef *base;
   IRQn_Type irq;
   int baudrate;   
   int recvCharOvrFlwCnt;
   int recvErrorCnt;
};

static struct Sio sSio[NUM_SIO] = {
   { .base = 0, .irq = 0 },      // Dummy because there is no USART1
   { .base = USART1, .irq = USART1_IRQn}
};


int SioOpen(int sioNr, int baud,  TFifo *rxFifo, TFifo *txFifo) {

   struct Sio *sio;

   if ((sioNr >= NUM_SIO) || (sSio[sioNr].base == 0) || (sSio[sioNr].isOpen)) {
      return -1;
   }

   sio = &sSio[sioNr];

   sio->isOpen = true;
   sio->baudrate = baud;
   sio->rxFifo = rxFifo;
   sio->txFifo = txFifo;

   FifoClear(rxFifo);
   FifoClear(txFifo);

   /* Disable UART - This clears old status and buffers */
   CLEAR_BIT(REG(CR1), USART_CR1_UE);

   HAL_NVIC_EnableIRQ(sio->irq);

   /* Enable Error Interrupts */
   SET_BIT(REG(CR1), USART_CR1_PEIE);
   SET_BIT(REG(CR3), USART_CR3_EIE);

   /* Disable TX-Int */
   CLEAR_BIT(REG(CR1), USART_CR1_TXEIE);

   /* Enable RX-Int */
   SET_BIT(REG(CR1), USART_CR1_RXNEIE);

   /* Enable UART */
   SET_BIT(REG(CR1), USART_CR1_UE);

   return sioNr;     // simply use sioNr as handle for now
}

static void TxStart(struct Sio *sio) {

   SET_BIT(REG(CR1), USART_CR1_TXEIE);
}

int SioRead(int hdl, char *buf, int len) {

   int fifoBytes;
   
   if (!VALID_HDL(hdl)) {
      return -1;
   }

   fifoBytes = FifoBytesFilled(sSio[hdl].rxFifo);
   if (len > fifoBytes) {
      len = fifoBytes;
   }

   return FifoReadBuf(sSio[hdl].rxFifo, buf, len);
}

int SioWrite(int hdl, char *buf, int len) {

   int numWritten = 0;

   if (!VALID_HDL(hdl)) {
      return -1;
   }

   if (len > 0) {
      numWritten = FifoWriteBuf(sSio[hdl].txFifo, buf, len);

      TxStart(&sSio[hdl]);
   }

   return numWritten;
}

/* Number of bytes received, but not read by application */
int SioNumRead(int hdl) {

   if (!VALID_HDL(hdl)) {
      return -1;
   }
   
   return FifoBytesFilled(sSio[hdl].rxFifo);
}

/* Number of bytes not yet sent */
int SioNumWrite(int hdl) {

   if (!VALID_HDL(hdl)) {
      return -1;
   }
   
   return FifoBytesFilled(sSio[hdl].txFifo);
}

int SioClose(int hdl) {

   unsigned flags;
   struct Sio *sio;

   if (!VALID_HDL(hdl)) {
      return -1;
   }
   sio = &sSio[hdl];

   /* Disable RX interrupt - keep TX open to send ev. remaining chars */
   flags = SysIntLock();
   CLEAR_BIT(REG(CR1), USART_CR1_RXNEIE);
   SysIntUnlock(flags);

   sSio[hdl].isOpen = false;

   return 0;
}

static void SioIntHandler(struct Sio *sio) {

   uint32_t isr;
   uint32_t errorFlags;
   int nr;
   char data;

   isr = REG(ISR);

   /* Rx interrupt */
   if (isr & USART_ISR_RXNE) {
      data = REG(RDR);
      nr = FifoWriteByte(sio->rxFifo, data);
      if (nr < 1) {
         sio->recvCharOvrFlwCnt++;
      }
   }

   /* Transmitter ready */
   if (isr & USART_ISR_TXE) {
      if (FifoReadByte(sio->txFifo, &data) == 1) {
         /* send data byte */
         REG(TDR) = data;
      }

      /* all sent? */
      if (FifoBytesFilled(sio->txFifo) == 0) {
         /* disable TXE interrupt */
         CLEAR_BIT(REG(CR1), USART_CR1_TXEIE);
      }
   }

   /* check for errors */
   errorFlags = (isr & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE));
   if (errorFlags) {
      sio->recvErrorCnt++;

      /* clear errors */
      REG(ICR) = errorFlags;
   }
}

/*
void USART0_IRQHandler(void) {
   SioIntHandler(&sSio[0]);
}
*/
void USART1_IRQHandler(void) {
   SioIntHandler(&sSio[1]);
}
/*
void USART2_IRQHandler(void) {
   SioIntHandler(&sSio[2]);
}

void USART3_IRQHandler(void) {
   SioIntHandler(&sSio[3]);
}
*/


