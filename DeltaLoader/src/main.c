#include "Sio.h"
#include "Fifo.h"
#include "SysOut.h"
#include "SysTimer.h"
#include "SysInt.h"

#include "stm32f0xx_hal.h"
#include "halstuff.h"

#include "Sbus.h"
#include "Rx.h"
#include "Track.h"
#include "Mast.h"

#define VERSION               "DeltaLoader_0.01"

#define RX_FIFO_LEN           32
#define TX_FIFO_LEN           128

#define SBUS_CH_POWER         4     // Turn Loader off
#define SBUS_CH_LIGHTS        5
#define SBUS_CH_RTD           6     // Return to dig

#define BATT_MEAN_SHIFT       10
#define BATT_MEAN_CNT         (1 << BATT_MEAN_SHIFT)
//#define BATT_UNDERVOLT_THRES  2715  /* about 7,5V */
#define BATT_UNDERVOLT_THRES  2535  /* about 7,0V */

#define IDLE_DELTA_THRES      100
#define IDLE_TIME_SEC         (4 * 60)

int gStdInOutFd;

int gStdInOutHdl;
static int sSbusHdl;

static void InitTimers(void);
static void InitUart(void);
static void FatalErrHalt(void);


static void PowerOn(void) {

   HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, 1);
}

static void PowerOff(void) {

   HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, 0);
}

/* Called every 1ms */
static void CheckIdle(void) {

   int maxDelta;
   static int sMsCnt;
   static int sIdleSecs;

   sMsCnt++;
   if (sMsCnt < 1000) {
      return;
   }
   sMsCnt = 0;

   /* We come here once every second */
   /* Check for rx idle */
   maxDelta = SbusMaxDelta();
/*   SysOutStr("\n\rmaxDelta: ");
   SysOutInt32(maxDelta);*/
   if (maxDelta < IDLE_DELTA_THRES) {         
//      SysOutStr("\n\rIdle");
      sIdleSecs++;
   } else {
//      SysOutStr("\n\rActive");
      sIdleSecs = 0;
   }
   SbusClearMinMax();
   if (sIdleSecs > IDLE_TIME_SEC) {
      SysOutStr("\n\rOFF (idle): ");
      PowerOff();
   }
}

/* Must not be called too often. ADC needs to convert between calls */
/* Called every 1ms */
static void CheckVoltage(void) {

   uint32_t adc;
   static int sFirst = 1;
   static uint32_t sSum;
   uint32_t battVolt;
   static int sCheckCnt;

   /* On the very first call only start the conversion */
   if (sFirst) {
      sFirst = 0;
      HAL_ADC_Start(&hadc);
      return;
   }

   /* Read value from last conversion and start next conversion */
   adc = HAL_ADC_GetValue(&hadc);
   HAL_ADC_Start(&hadc);

   /* Mean calculation */
   sSum += adc;
   sCheckCnt++;
   if (sCheckCnt >= BATT_MEAN_CNT) {
      battVolt = sSum >> BATT_MEAN_SHIFT;
      sCheckCnt = 0;
      sSum = 0;

      SysOutStr("\n\rBatt: ");
      SysOutDec32(battVolt);

      if (battVolt < BATT_UNDERVOLT_THRES) {
         SysOutStr("\n\rOFF (batt): ");
         PowerOff();
      }    
   }
/*
   if (SysGetTimeMs() % 50 == 0) {
      SysOutStr("\n\rADC: ");
      SysOutInt32(adc);
   }
*/
}

int main(void) {

   char b;
   int nr;
   eSbusFrameState sbusState;
   uint32_t timeMs;
   static uint32_t sLastTimeMs;

   halstuffInit();

   InitUart();
   InitTimers();
   
   /* Some delay to prevent re-poweron if there is still energy 
      in the motors... */
   SysOutStr("Power on delay...\n\r");
   HAL_Delay(500);
   SysOutStr("done\n\r");

   /* Keep the power switch on */
   PowerOn();

   /* Motors */
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

   /* Bucket */
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
   /* Wait a bit to let bucket come to default position */
   HAL_Delay(500);
   
   /* Lift */
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

   /* Turn on green LED */
   HAL_GPIO_WritePin(LED_PPM_GPIO_Port, LED_PPM_Pin, 1);
   
   SbusClearMinMax();
   MastInit();

   while (1) {
      timeMs = SysGetTimeMs();

      nr = SioRead(sSbusHdl, &b, 1);
      if (nr == 1) {
         sbusState = SbusProcess(b);
         if (sbusState == eFrameComplete) {
            TrackUpdate();
            MastUpdate();
         }
      }

      if (timeMs != sLastTimeMs) {
         sLastTimeMs = timeMs;
         CheckVoltage();
         CheckIdle();
      }
   }
}

/* Return a 32bit 1us counter.
 * Needs to be called at least once every 65ms. To ensure this, just
 * call it in 1ms timer interrupts handler for example */
static uint32_t GetCounter32(void *unused) {

   static uint16_t sLastLow;
   static uint16_t sHigh;
   uint16_t low;
   uint32_t ret;
   unsigned int flags;

   /* read 16 bit counter-value from TIM14 */
   low = TIM14->CNT;

   /* Lock */
   flags = SysIntLock();

   /* check for 16 bit overflow */
   if (low < sLastLow) {
      sHigh++;
   }
   sLastLow = low;

   ret = ((uint32_t)sHigh << 16) | low;

   /* Unlock */
   SysIntUnlock(flags);

   return ret;
}

static void InitTimers(void) {

   /* TIM14: 1usec counter */
   SysTimerConfig(GetCounter32, 0);
}

static void InitUart(void) {

   static char rxData[RX_FIFO_LEN];
   static char txData[TX_FIFO_LEN];
   static TFifo rxFifo;
   static TFifo txFifo;
   int hdl;

   FifoInit(&rxFifo, rxData, sizeof(rxData));
   FifoInit(&txFifo, txData, sizeof(txData));
   
   hdl = SioOpen(SIO1, 100000, &rxFifo, &txFifo);
   if (hdl == -1) {
      FatalErrHalt();
   }
   
   gStdInOutHdl = hdl;
   sSbusHdl = hdl;

   SysOutStr("\n\r"VERSION"\n\r");
}

static void FatalErrHalt(void) {
     
   while (1) {}
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  SysTimerTickExpired();
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1) {}
}

void MemManage_Handler(void)
{
  while (1) {}
}

void BusFault_Handler(void)
{
  while (1) {}
}

void UsageFault_Handler(void)
{
  while (1) {}
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}