// Line follower Robot : Scorpion 2.0
// By : Md. Hasibur Rahman, KYAU

//Libraries
#include "Scorpion.h"
#include <SPI.h>
#include <Wire.h>
#include <HCSR04.h>

String Default_Turn = "right";
String Track_Color = "black";

//IR array
#define IRA A0
#define IRB A1
#define IRC A2
#define IRD A3
#define IRE A7
#define IR_RIGHT 2
#define IR_LEFT 9
int A = 0, B = 0, C = 0, D = 0, E = 0, F = 1, R = 0, L = 0, AIR = 0, RL = 0; //IR variable for store value

#define BUZZER 13

//Speed and time tuner
const int TST = 250; // Track searching time (FM - 180)
const int _90dTtime = 0; // time need for turning 90 degree
const int _180Ttime = 0; // time need for turning 180 degree
const int TBT = 150; // time before turning (FM - 120)
const int TAT = 0; // time after taking turn for distracting from current track (FM - 35)

//Motor Driver pins
int ENA = 5, IN1 = 3, IN2 = 4, ENB = 6, IN3 = 7, IN4 = 8;// For Motor Driver

//Variables
int DutyCycle = 0, min_speed = 200, med_speed = 205, high_speed = 210, max_speed = 255, R_max_speed = 255, L_max_speed = 225;


//Using class "Motor" {methods = Forward, Backward, Stop, Speed, Status}
Motor MotorR(ENA, IN1, IN2);  // Right Motor - (IN1, IN2, en, pwm channel)
Motor MotorL(ENB, IN3, IN4);  // Left Motor - (inputpIN1, inputpIN2, enablepin, pwmChannel[0-18])

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(IR_LEFT, INPUT);
  Serial.println("Loading.....");
  Beep(3, 250);

  delay(200);

  MotorR.Forward();
  MotorL.Forward();
  delay(200);

  int spd = 170;
  while (true) {
    ReadIR();
    if (RL == 0) {
      if (AIR == 4)//On track
      {
        (A == 0) ? SharpLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? MedRight() : (E == 0) ? SharpRight() : ReadIR();
      }
      else if (AIR == 3) //
      {
        (C + D == 0) ? SmoothRight()  : (D + E == 0) ? HardRight() : (C + B == 0) ? SmoothLeft() : (A + B == 0) ? HardLeft() : ReadIR();
      }

      else if (AIR == 0)//multiple line
      {
        MotorL.Speed(0);
        MotorR.Speed(0);
      }

      else if (AIR == 5)// White space
      {
        Straight();
      }

    }
    else {
      (RL == 2) ? DefaultTurn() : (R == 1) ? _90dRight() : (L == 1) ? _90dLeft() : ReadIR();
    }
  }
  delay(1000);
}








//*** Default turn
void DefaultTurn() {
  (Default_Turn == "right") ? _90dRight() : _90dLeft();
}

void loop() {
}

//*** Car speed 0 with with forward gear
void Brake() {
  MotorR.Speed(0);
  MotorL.Speed(0);
}

//*** Car stop and neutral
void Neutral() {
  MotorR.Release();
  MotorL.Release();
}

//*** Straight Forward - ok
void Straight() {
  MotorR.Speed(R_max_speed);//left motor is bit damaged thats why used more duty cycle than right motor
  MotorL.Speed(L_max_speed);
}

//*** Smooth Left Turn - ok
void SmoothLeft() {
  MotorL.Speed(100);
  MotorR.Speed(255);
}

//*** Smooth Right Turn - ok
void SmoothRight() {
  MotorL.Speed(255);
  MotorR.Speed(100);
}


//*** Medium Left Turn - ok
void MedLeft() {
  MotorL.Speed(50);
  MotorR.Speed(255);
}


//*** Medium Right Turn - ok
void MedRight() {
  MotorL.Speed(255);
  MotorR.Speed(50);
}

//hard left - ok
void HardLeft() {
  MotorL.Speed(0);
  MotorR.Speed(200);
}

//Hard right - ok
void HardRight() {
  MotorL.Speed(200);
  MotorR.Speed(0);

}

//*** Sharp Left Turn - ok
void SharpLeft() {
  Neutral();
  delay(10);
  MotorR.Speed(80);
  MotorL.Speed(80);
  MotorR.Forward(); MotorL.Backward();
  while (C == 1) {
    ReadIR();
  }
  Neutral();
  delay(10);
  MotorL.Forward(); MotorR.Forward();
}

//*** Sharp Right Turn - ok
void SharpRight() {
  Neutral();
  delay(10);
  MotorR.Speed(80);
  MotorL.Speed(80);
  MotorL.Forward(); MotorR.Backward();
  while (C == 1) {
    ReadIR();
  }
  Neutral();
  delay(10);
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d left turn
void _90dLeft() {
  Serial.println("_90dLeft");
  Straight();
  delay(TBT);
  ReadIR();
  MotorR.Forward(); MotorL.Backward();
  MotorL.Speed(150); MotorR.Speed(150);
  //delay(TAT);// if this is a 4 line it will distrac from the middle line within 10 mili second
  //  while (!(AIR == 4 && C == 0)) {
  while (AIR == 5) {
    ReadIR();
  }
  Neutral();
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d Right Turn
void _90dRight() {
  Serial.println("_90dRight");
  Straight();
  delay(TBT);
  ReadIR();
  MotorL.Forward(); MotorR.Backward();
  MotorL.Speed(150); MotorR.Speed(150);
  //delay(TAT);// if this is a 4 line it will distrac from the middle line within 10 mili second
  //  while (!(AIR == 4 && C == 0)) {
  while (AIR == 5) {
    ReadIR();
  }
  Neutral();
  MotorL.Forward(); MotorR.Forward();
}

//*** Read all IR sensor
void ReadIR() {
  A = analogRead(IRA); A = A / 600; //(A == 0) ? A = 0 : A = 1; // 0 = black, 1 = white
  B = analogRead(IRB); B = B / 600; //(B == 0) ? B = 0 : B = 1;// 0 = black, 1 = white
  C = analogRead(IRC); C = C / 600; //(C == 0) ? C = 0 : C = 1;// 0 = black, 1 = white
  D = analogRead(IRD); D = D / 600; //(D == 0) ? D = 0 : D = 1;// 0 = black, 1 = white
  E = analogRead(IRE); E = E / 600; //(E == 0) ? E = 0 : E = 1;// 0 = black, 1 = white
  R = digitalRead(IR_RIGHT); //R = R / 600; //(D == 0) ? D = 0 : D = 1;// 0 = white, 1 = black
  L = digitalRead(IR_LEFT); //L = L / 600; //(E == 0) ? E = 0 : E = 1;// 0 = white, 1 = black
  RL = R + L;
  AIR = A + B + C + D + E;

  Serial.println(" ");
  Serial.print(":A=");
  Serial.print(A);
  Serial.print(":B=");
  Serial.print(B);
  Serial.print(":C=");
  Serial.print(C);
  Serial.print(":D=");
  Serial.print(D);
  Serial.print(":E=");
  Serial.print(E);
  Serial.print(":R=");
  Serial.print(R);
  Serial.print(":L=");
  Serial.print(L);
  Serial.print(":AIR=");
  Serial.print(AIR);
}

int Beep(int n, int dly) {
  int i = 0;
  while (i < n) {
    digitalWrite(BUZZER, HIGH);
    delay(dly);
    digitalWrite(BUZZER, LOW);
    delay(dly);
    i++;
  }
}
