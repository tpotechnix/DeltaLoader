#include "Mast.h"
#include "Sbus.h"
#include "Rx.h"
#include "SysOut.h"
#include "SysTimer.h"

#include "stdint.h"

#define SERVO_CH_CURL   0
#define SERVO_CH_LIFT   1

#define SBUS_CH_CURL    0
#define SBUS_CH_LIFT    1

#define LIFT_MIN        -31000
#define LIFT_MAX        31000
#define CURL_MIN        -36350
#define CURL_MAX        35200

#define SCALE_LIFT_SPEED_DIV   1000000
#define SCALE_CURL_SPEED_DIV   350000

#define LIFT_POS_INITIAL       -24700
#define CURL_POS_INITIAL       -26185

#define DEADBAND    2000
struct RxCurvePt sDeadband[] = {
   RXPT(-32768, -32768),
   RXPT(-DEADBAND / 2, 0),
   RXPT(DEADBAND / 2, 0),
   RXPT(32768, 32768),
};

void MastInit(void) {

   RxInitKd(RXCURVE(sDeadband));
}

void MastUpdate(void) {

   int liftin;
   int curlin;
   int lift;
   int curl;
   int liftInc;
   int curlInc;
   uint32_t ts;
   uint32_t delta;
   static uint32_t sTsLast;
   static int sFirst = 1;
   static int sLiftPos = LIFT_POS_INITIAL;
   static int sCurlPos = CURL_POS_INITIAL;
   
   ts = SysGetTimeUs();
   if (sFirst) {
      sTsLast = ts;
      sFirst = 0;
      return;
   }
   
   liftin = SbusGetRx(SBUS_CH_LIFT);
   curlin = SbusGetRx(SBUS_CH_CURL);

   /* Add some deadband */
   lift = RxCurve(liftin, RXCURVE(sDeadband));
   curl = RxCurve(curlin, RXCURVE(sDeadband));

   delta = ts - sTsLast;
   sTsLast = ts;
   liftInc = (int)delta * lift / SCALE_LIFT_SPEED_DIV;
   curlInc = (int)delta * curl / SCALE_CURL_SPEED_DIV;

   sLiftPos += liftInc;
   sCurlPos += curlInc;
   LIMIT_RX(sLiftPos, LIFT_MIN, LIFT_MAX);
   LIMIT_RX(sCurlPos, CURL_MIN, CURL_MAX);

   RxSetServo(SERVO_CH_LIFT, sLiftPos);
   RxSetServo(SERVO_CH_CURL, sCurlPos);

   if ((SbusFrameCnt() % 50) == 0) {
  /*    SysOutStr("\n\r");

      SysOutStr("\n\rlift: ");
      SysOutDec32(liftin);

      SysOutStr("\n\rlift: ");
      SysOutDec32(lift);

      SysOutStr("\n\rdelta: ");
      SysOutDec32(delta);
      SysOutStr("\n\rliftInc: ");
      SysOutDec32(liftInc);
*/
      SysOutStr("\n\rliftPos: ");
      SysOutDec32(sLiftPos);
      SysOutStr("  liftPWM: ");
      SysOutDec32(RxGetServoPWM(SERVO_CH_LIFT));
      SysOutStr("  curlPos: ");
      SysOutDec32(sCurlPos);
      SysOutStr("  curlPWM: ");
      SysOutDec32(RxGetServoPWM(SERVO_CH_CURL));
   }
}