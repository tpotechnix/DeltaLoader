#include "Track.h"
#include "Sbus.h"
#include "Rx.h"
#include "SysOut.h"

#define SBUS_CH_SPEED     2
#define SBUS_CH_DIR       3

#define SERVO_CH_TRACK_L  2
#define SERVO_CH_TRACK_R  3

void TrackUpdate(void) {

   int speed;
   int dir;
   int left;
   int right;

   /* simple track mixer - Revers is inverted! */
   /* Idea to get sterring correct for backwards:
      -> Use flipflop for steering inversion.
      -> set when Speed is sightly reverse and Sir is around zero */
   speed = SbusGetRx(SBUS_CH_SPEED);
   dir = SbusGetRx(SBUS_CH_DIR);

   left = speed + dir;
   right = speed - dir;
   LIMIT_RX_DEF(left);
   LIMIT_RX_DEF(right);

   RxSetServo(SERVO_CH_TRACK_L, left);
   RxSetServo(SERVO_CH_TRACK_R, right); 
/*
   if ((SbusFrameCnt() % 20) == 0) {
      SysOutStr("\n\r");

      SysOutStr("\n\rSpd: ");
      SysOutDec32(speed);

      SysOutStr("\n\rDir: ");
      SysOutDec32(dir);

      SysOutStr("\n\rL: ");
      SysOutDec32(left);
      SysOutStr(" ");
      SysOutDec32(leftPwm);
      SysOutStr("\n\rR: ");
      SysOutDec32(right);
      SysOutStr(" ");
      SysOutDec32(rightPwm);

   }*/
}