// Line follower Robot : Scorpion 2.0
// By : Md. Hasibur Rahman, KYAU
#include "Scorpion.h"
#include <SPI.h>
#include <Wire.h>
#include <HCSR04.h>

String Default_Turn = "right",  Object = "Not Found";
String Track_Color = "black";
//String Track_Color = "white";

//***VARIABLES FOR IR SENSOR
int IRA = A0, IRB = A1, IRC = A2, IRD = A3, IRE = A4, IR_RIGHT = A5, IR_LEFT = A6, IR_FRONT = 2;//IR Pins
int A = 0, B = 0, C = 0, D = 0, E = 0, R = 0, L = 0, F = 0, AIR = 0, RL = 0; //IR variable for store value

#define BUZZER 13

//***VARIABLES FOR SONAR SENSOR
int S1Trig = 9, S1Echo = 10, S2Trig = 11, S2Echo = 12;//Sonar Sensor Pins
int SNR_R = 0, SNR_L = 0;//Store sonar data
HCSR04 SNR_RIGHT(S1Trig, S1Echo); //Right Sonor - initialisation class HCSR04 (trig pin - input , echo pin - output)
HCSR04 SNR_LEFT(S2Trig, S2Echo); //Left Sonor - initialisation class HCSR04 (trig pin - input , echo pin - output)

//***VARIABLES FOR MOTOR DRIVER
int ENA = 5, IN1 = 3, IN2 = 4, ENB = 6, IN3 = 7, IN4 = 8;//Pins For Motor Driver
Motor MotorR(ENA, IN1, IN2);  // Right Motor - (IN1, IN2, en, pwm channel)// Motor1 declaration
Motor MotorL(ENB, IN3, IN4);  // Left Motor - (inputpIN1, inputpIN2, enablepin, pwmChannel[0-18])// Motor2 declaration

void setup() {
  Serial.begin(9600);
  Serial.println("Loading...");
  pinMode(BUZZER, OUTPUT);
  pinMode(IR_FRONT, INPUT);
  MotorR.Forward();
  MotorL.Forward();
  Beep(3, 200);

  while (false) {
    ReadIR();
    ReadSonar();
  }

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

      else if (AIR == 5)// White space
      {
        Straight();
        delay(100);
        ReadSonar();//Find cave
        if (SNR_R <= 30 && SNR_L <= 30) {
          FollowCave();
        }
        else {
          FindTrack();
        }
      }
    }

    else if (RL == 2) {
      if (AIR == 0) {
        delay(50);
        ReadIR();
        (AIR == 0 && RL == 2) ? Brake() : DefaultTurn();
      }
    }

    else if (RL == 1) {
      (R == 1) ? _90dRight() : _90dLeft();
    }
  }
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

//*** Straight Forward
void Straight() {
  MotorL.Speed(200);
  MotorR.Speed(255);//Right motor is bit damaged thats why used more duty cycle than right motor
}

//*** Smooth Left Turn
void SmoothLeft() {
  MotorL.Speed(120);
  MotorR.Speed(255);
}

//*** Smooth Right Turn
void SmoothRight() {
  MotorL.Speed(255);
  MotorR.Speed(150);
}


//*** Medium Left Turn
void MedLeft() {
  MotorL.Speed(50);
  MotorR.Speed(255);
}


//*** Medium Right Turn
void MedRight() {
  MotorL.Speed(255);
  MotorR.Speed(80);
}

//hard left - ok
void HardLeft() {
  MotorL.Speed(0);
  MotorR.Speed(255);
}

//Hard right
void HardRight() {
  MotorL.Speed(200);
  MotorR.Speed(0);

}

//*** Sharp Left Turn
void SharpLeft() {
  MotorR.Forward(); MotorL.Backward();
  MotorR.Speed(100);
  MotorL.Speed(100);
  while (C == 1) {
    ReadIR();
  }
  MotorL.Forward(); MotorR.Forward();
}

//*** Sharp Right Turn
void SharpRight() {
  MotorL.Forward(); MotorR.Backward();
  MotorR.Speed(120);
  MotorL.Speed(100);
  while (C == 1) {
    ReadIR();
  }
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d left turn
void _90dLeft() {
  Serial.println("_90dLeft");
  Straight();
  AsyncTurn(300);//go straight until AIR = 0 or the time out
  MotorR.Forward(); MotorL.Backward();
  MotorL.Speed(100); MotorR.Speed(120);
  AsyncWait(800); //turn until the track is found or 800mls exceed
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d Right Turn
void _90dRight() {
  Serial.println("_90dRight");
  Straight();
  AsyncTurn(300);//go straight until AIR = 0 or the time out
  MotorR.Backward(); MotorL.Forward();
  MotorL.Speed(100); MotorR.Speed(120);
  AsyncWait(800);//turn until the track is found or 800mls exceed
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
}

//*** Read all IR sensor
void ReadIR() {
  A = analogRead(IRA); A = A / 600; //(A == 0) ? A = 0 : A = 1; // 0 = black, 1 = white
  B = analogRead(IRB); B = B / 600; //(B == 0) ? B = 0 : B = 1;// 0 = black, 1 = white
  C = analogRead(IRC); C = C / 600; //(C == 0) ? C = 0 : C = 1;// 0 = black, 1 = white
  D = analogRead(IRD); D = D / 600; //(D == 0) ? D = 0 : D = 1;// 0 = black, 1 = white
  E = analogRead(IRE); E = E / 600; //(E == 0) ? E = 0 : E = 1;// 0 = black, 1 = white
  R = analogRead(IR_RIGHT); R = R / 600; //(D == 0) ? D = 0 : D = 1;// 0 = white, 1 = black
  L = analogRead(IR_LEFT); L = L / 600; //(E == 0) ? E = 0 : E = 1;// 0 = white, 1 = black
  F = digitalRead(IR_FRONT); //F = F / 600; //(E == 0) ? E = 0 : E = 1;// 0 = white, 1 = black
  if (Track_Color == "white") {
    A = abs(A - 1);
    B = abs(B - 1);
    C = abs(C - 1);
    D = abs(D - 1);
    E = abs(E - 1);
    R = abs(R - 1);
    L = abs(L - 1);
  }
  RL = R + L;
  AIR = A + B + C + D + E;

  if (F == 0) {
    //Brake();
    Serial.println(" ");
    Serial.println("Obstacle Found");
    SkipObject
  }


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
  Serial.print(":Right=");
  Serial.print(R);
  Serial.print(":Left=");
  Serial.print(L);
  Serial.print(":Front=");
  Serial.print(F);
  Serial.print(":AIR=");
  Serial.print(AIR);

}

void ReadSonar() {
  SNR_R = SNR_RIGHT.dist() - 4; //sonar is 4 cm inside
  delay(10);
  SNR_L = SNR_LEFT.dist() - 4;//sonar is 4 cm inside
  delay(10);
  Serial.print(" :SNR_R= ");
  Serial.print(SNR_R);
  Serial.print(" :SNR_L=");
  Serial.println(SNR_L);
}

void AsyncWait(unsigned long interval) {
  unsigned long TimeCount = millis();;
  unsigned long CurrentTime = TimeCount;
  unsigned long TimeLap;
  do {
    TimeLap = TimeCount - CurrentTime;
    TimeCount = millis();
    ReadIR();
  }
  while (TimeLap < interval && AIR == 5); // if track found within time or the time is out then break the loop
}

void AsyncTurn(unsigned long interval) {
  unsigned long TimeCount = millis();;
  unsigned long CurrentTime = TimeCount;
  unsigned long TimeLap;
  do {
    TimeLap = TimeCount - CurrentTime;
    TimeCount = millis();
    ReadIR();
  }
  while (TimeLap < interval && AIR != 5); // if track found within time or the time is out then break the loop
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

void FindTrack() {
  // Go forward for finding track
  Straight();
  AsyncWait(400); // go 15 cm
  if (AIR == 5) { // still white space?
    U_Turn(1600); // search sides
    if (AIR == 5) { // still white space?
      Straight();
      AsyncWait(400); // go more 15 cm
      if (AIR == 5) { // if still white space take U turn and go straight to 30 cm
        U_Turn(800);
        Straight();
        AsyncWait(800);
      }
    }
  }
}

void FollowCave() {
  // Follow cave untill the track is not found (untlil AIR != 5)
  while (AIR == 5) {
    ReadIR();
    ReadSonar();
    int RoadWidth, SideSpace;
    RoadWidth = SNR_R + SNR_L; // total side gap
    SideSpace = (RoadWidth / 2); // average side gap for each side

    //if (SNR_R <= 40 || SNR_L <= 40) { //when the sensor can count distance. Go by the middle of path
    if (true) {
      if (SNR_R > SideSpace) { // car is not in middle of the.
        MedRight();
      }
      else if (SNR_L > SideSpace) { // car is not in middle of the walls
        MedLeft();
      }
      else { // car is now in middle of the walls
        Straight();
      }
    }
  }
}

//***skiping object
void SkipObject(){
  //1. turn 90 degree right
  MotorL.Forward(); MotorR.Backward();// Rotate on place
  MotorR.Speed(120); MotorL.Speed(100);
  delay(400);
  
  //2. Go straight 20 cm
  Straight();
  delay(300);
  
  //3. Turn 90 degree left
  MotorR.Forward(); MotorL.Backward();// Rotate on place
  MotorR.Speed(120); MotorL.Speed(100);
  delay(400);
  
  //4. Go straight 40 cm
  Straight();
  delay(600);

  //5. Turn 90 degree left
  MotorR.Forward(); MotorL.Backward();// Rotate on place
  MotorR.Speed(120); MotorL.Speed(100);
  delay(400);

  //6. go forward until find the track
  ReadIR();
  while(AIR == 5){
    ReadIR();
    Straight();
  }

  //7. Turn right after finding the track
  _90dRight();
}
