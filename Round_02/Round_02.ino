// Line follower Robot : Scorpion 2.0
// By : Md. Hasibur Rahman, KYAU

//Libraries
#include "Scorpion.h"
#include <SPI.h>
#include <Wire.h>
#include <HCSR04.h>

String Default_Turn = "right";
String Track_Color = "black";
String Object = "Not Found";

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

//Sonar Sensor
#define S1Trig 9
#define S1Echo 10
#define S2Trig 11
#define S2Echo 12
HCSR04 SonarR(S1Trig, S1Echo); //Right Sonor - initialisation class HCSR04 (trig pin - input , echo pin - output)
HCSR04 SonarL(S2Trig, S2Echo); //Left Sonor - initialisation class HCSR04 (trig pin - input , echo pin - output)
int SonarA, SonarB;

//Speed and time tuner
const int TST = 250; // Track searching time (FM - 180)
const int _90dTtime = 0; // time need for turning 90 degree
const int _180Ttime = 0; // time need for turning 180 degree
const int TBT = 150; // time before turning (FM - 120)
const int TAT = 0; // time after taking turn for distracting from current track (FM - 35)

//For asynchronous function
unsigned long TimeCount;
unsigned long CurrentTime;
unsigned long TimeLap;


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
  Serial.println("Loading...");
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
        //(A == 0) ? _90dLeft() : (B == 0) ? SharpLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? SharpRight() : (E == 0) ? _90dRight() : ReadIR();
        //(A == 0) ? SharpLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? MedRight() : (E == 0) ? SharpRight() : ReadIR();
      }
      else if (AIR == 3) //
      {
        (C + D == 0) ? SmoothRight()  : (D + E == 0) ? HardRight() : (C + B == 0) ? SmoothLeft() : (A + B == 0) ? HardLeft() : ReadIR();
        //(C + D == 0) ? HardRight() : (A + C == 0) ? _90dLeft() : (C + E == 0) ? _90dRight() : (D + E == 0) ? _90dRight() : (C + B == 0) ? HardLeft() : (A + B == 0) ? _90dLeft() : ReadIR();
        //(C + D == 0) ? SmoothRight() : (D + E == 0) ? HardRight() : (C + B == 0) ? SmoothLeft() : (A + B == 0) ? HardLeft() : ReadIR();
      }

      //  else if (AIR == 2) {
      //    (C + D + E == 0) ? SharpRight() : (A + B + C == 0) ? SharpLeft() : ReadIR();
      //  }
      //  else if (AIR == 1) {
      //    A == 0 ? SharpLeft() : SharpRight();
      //  }

      else if (AIR == 0)//multiple line
      {
        MotorL.Speed(0);
        MotorR.Speed(0);
      }
      else if (AIR == 5)// White space
      {
        // Go forward for finding track
        MotorL.Speed(200);
        MotorR.Speed(255);
        AsyncWait(400); // go 15 cm
        if (AIR == 5) { // still white space?
          U_Turn(1600); // search sides
          if (AIR == 5) { // still white space?
            MotorL.Speed(200);
            MotorR.Speed(255);
            AsyncWait(400); // go more 15 cm
            if (AIR == 5) { // if still white space take U turn and go straight to 30 cm
              U_Turn(800);
              AsyncWait(800);
            }
          }
        }
        //(AIR == 5) ? U_Turn(1800) : ReadIR(); // if there is no track turn back
      }

    }
    else if (RL == 2) {
      int interval = 100;
      int TimeCount = millis(); // time count
      int CurrentTime = TimeCount;
      do {
        TimeLap = TimeCount - CurrentTime;
        TimeCount = millis();
        ReadIR();
      }
      while (TimeLap < interval && RL == 2);
      ReadIR();
      (RL == 2) ? Brake() : DefaultTurn();
    }
    else if (RL == 1) {
      (R == 1) ? _90dRight() : _90dLeft();
    }
  }
  delay(1000);
}

//*** Default turn
void DefaultTurn() {
  (Track_Color == "right") ? _90dRight() : _90dLeft();
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
  MotorL.Speed(200);
  MotorR.Speed(255);//Right motor is bit damaged thats why used more duty cycle than right motor
}

//*** Smooth Left Turn - ok
void SmoothLeft() {
  MotorL.Speed(120);
  MotorR.Speed(255);
}

//*** Smooth Right Turn - ok
void SmoothRight() {
  MotorL.Speed(255);
  MotorR.Speed(150);
}


//*** Medium Left Turn - ok
void MedLeft() {
  MotorL.Speed(50);
  MotorR.Speed(255);
}


//*** Medium Right Turn - ok
void MedRight() {
  MotorL.Speed(255);
  MotorR.Speed(80);
}

//hard left - ok
void HardLeft() {
  MotorL.Speed(0);
  MotorR.Speed(255);
}

//Hard right - ok
void HardRight() {
  MotorL.Speed(200);
  MotorR.Speed(0);

}

//*** Sharp Left Turn - ok
void SharpLeft() {
  MotorR.Forward(); MotorL.Backward();
  MotorR.Speed(100);
  MotorL.Speed(100);


  while (C == 1) {
    ReadIR();
  }

  //  Neutral();
  //  delay(10);

  MotorL.Forward(); MotorR.Forward();
}

//*** Sharp Right Turn - ok
void SharpRight() {
  MotorL.Forward(); MotorR.Backward();
  MotorR.Speed(120);
  MotorL.Speed(100);


  while (C == 1) {
    ReadIR();
  }

  //  Neutral();
  //  delay(10);

  MotorL.Forward(); MotorR.Forward();
}

//*** 90d left turn
void _90dLeft() {
  Serial.println("_90dLeft");
  while (AIR != 5) {
    ReadIR();
    Straight();
  }
  MotorR.Forward(); MotorL.Backward();
  MotorL.Speed(100); MotorR.Speed(120);
  AsyncWait(800);
  //  while (AIR == 5) {
  //    ReadIR();
  //  }
  //  Neutral();
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d Right Turn
void _90dRight() {
  Serial.println("_90dRight");
  while (AIR != 5) {
    ReadIR();
    Straight();
  }
  MotorR.Backward(); MotorL.Forward();
  MotorL.Speed(100); MotorR.Speed(120);
  AsyncWait(800);
  //  while (AIR == 5) {
  //    ReadIR();
  //  }
  //  Neutral();
  MotorL.Forward(); MotorR.Forward();
}

//*** U_Turn turn on place
void U_Turn(int Time) {
  Neutral(); // Both motor stop with neutral gear
  delay(10);
  MotorL.Forward(); MotorR.Backward();// Rotate on place
  MotorR.Speed(120); MotorL.Speed(100);
  AsyncWait(Time); // [1800mls for 360degree]Turning until it found the track or it is on position of 180 degree
  Brake();
  ReadIR();
  MotorL.Forward(); MotorR.Forward();

  //  if(AIR == 5)
  //  {
  //      MotorL.Forward(); MotorR.Forward();
  //      MotorL.Speed(200); MotorR.Speed(200);
  //      AsyncWait(500); // go ahead until you find the track
  //  }

}


//Follow track
void FollowTrack() {

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

void ReadSonar() {
  SonarA = SonarR.dist();
  delay(68);
  SonarB = SonarL.dist();
  delay(68);
  Serial.print(" :SonarA= ");
  Serial.print(SonarA);
  Serial.print(" :SonarB=");
  Serial.println(SonarB);
}

void AsyncWait(int interval) {
  int TimeCount = millis(); // time count
  int CurrentTime = TimeCount;
  do {
    TimeLap = TimeCount - CurrentTime;
    TimeCount = millis();
    ReadIR();
  }
  while (TimeLap < interval && AIR == 5); // if track found within time or the time is out then break the loop
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
