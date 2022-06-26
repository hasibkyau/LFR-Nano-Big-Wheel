// Line follower Robot : Scorpion 2.0
// By : Md. Hasibur Rahman, KYAU

//Libraries
#include "Scorpion.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HCSR04.h>

String Default_Turn = "right";
String Track_Color = "black";
String Current_Decision = "Straight";
String Object = "Not Found";

//Oled display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//IR array
#define IRA A0
#define IRB A1
#define IRC A2
#define IRD A3
#define IRE A7
#define IR_RIGHT A4
#define IR_LEFT A5
int A = 0, B = 0, C = 0, D = 0, E = 0, F = 1, R = 0, L = 0, AIR = 0; //IR variable for store value

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
const int TBT = 0; // time before turning (FM - 120)
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
  Serial.println("TEST");
  Beep(3, 250);

  //display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  //delay(1000);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  //    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //      Serial.println(F("SSD1306 allocation failed"));
  //      for (;;); // Don't proceed, loop forever
  //    }

  //ShowMessges();
  delay(200);

  MotorR.Forward();
  MotorL.Forward();
  delay(200);

  int spd = 170;
  while (true) {
    ReadIR();
    if(R + L == 0){
    FollowTrack(); 
    }
    else if(R + L == 2){
      DefaultTurn();
    }
    else{
      (R == 1) ? _90dRight() : (L == 1) ? _90dLeft() : ReadIR();
    }
  }
  delay(1000);
}

//*** Default turn
void DefaultTurn() {
  (Track_Color == "right") ? _90dRight() : _90dLeft();
}

void loop() {
  ReadIR(); // reading IR data
  //ReadSonar();
  FollowTrack();
  //ShowMessges();

  //  if(F == 1){
  //    FollowTrack();
  //  }
  //  else{
  //    AvoidObstacle();
  //  }
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
  Current_Decision = "Straight";
  MotorR.Speed(R_max_speed);//left motor is bit damaged thats why used more duty cycle than right motor
  MotorL.Speed(L_max_speed);
}

//*** Smooth Left Turn - ok
void SmoothLeft() {
  Current_Decision = "Smooth Left";
  MotorL.Speed(150);
  MotorR.Speed(255);
}

//*** Smooth Right Turn - ok
void SmoothRight() {
  Current_Decision = "Smooth Right";
  MotorL.Speed(255);
  MotorR.Speed(150);
}


//*** Medium Left Turn - ok
void MedLeft() {
  Current_Decision = "Med Left";
  MotorL.Speed(100);
  MotorR.Speed(255);
}


//*** Medium Right Turn - ok
void MedRight() {
  Current_Decision = "Med Right";
  MotorL.Speed(255);
  MotorR.Speed(100);
}

//hard left - ok
void HardLeft() {
  Current_Decision = "Hard Left";
  MotorL.Speed(50);
  MotorR.Speed(255);
}

//Hard right - ok
void HardRight() {
  Current_Decision = "Hard Right";
  MotorL.Speed(255);
  MotorR.Speed(50);

}

//*** Sharp Left Turn - ok
void SharpLeft() {
  Current_Decision = "Sharp Left";
  MotorR.Speed(255);
  MotorL.Speed(0);
}

//*** Sharp Right Turn - ok
void SharpRight() {
  Current_Decision = "Sharp Right";
  MotorR.Speed(0);
  MotorL.Speed(255);
}

//*** 90d left turn
void _90dLeft() {
  Serial.println("_90dLeft");
  Current_Decision = "90d Left";
  Straight();
  delay(250);
  MotorR.Forward(); MotorL.Backward();
  MotorL.Speed(80); MotorR.Speed(80);
  //delay(TAT);// if this is a 4 line it will distrac from the middle line within 10 mili second
  while(!(AIR == 4 && C == 0)){
    ReadIR();
  }
  Neutral();
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d Right Turn
void _90dRight() {
  Serial.println("_90dRight");
  Current_Decision = "90d Right";
  Straight(); 
  delay(250); 
  MotorL.Forward(); MotorR.Backward();
  MotorL.Speed(80); MotorR.Speed(80);
  //delay(TAT);// if this is a 4 line it will distrac from the middle line within 10 mili second
    while(!(AIR == 4 && C == 0)){
    ReadIR();
  }
  Neutral();
  MotorL.Forward(); MotorR.Forward();
}

//*** 180d turn on place
void _180dTurn() {
  Current_Decision = "180d Turn";
  //Serial.println("Taking U turn"); delay(2000);
  Neutral(); // Both motor stop with neutral gear
  delay(10);
  MotorL.Forward(); MotorR.Backward();// Rotate on place
  MotorR.Speed(100); MotorL.Speed(100);
  do {
    ReadIR();
  }
  while (AIR == 5); // finish 180d ?
  Brake(); // Both motor stop with neutral gear
  Neutral();
  delay(10);// for refreshin dirver logic
  MotorL.Forward(); MotorR.Forward();
}


//Follow track
void FollowTrack() {
  if (AIR == 4)//On track
  {
    (A == 0) ? SharpLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? MedRight() : (E == 0) ? SharpRight() : ReadIR();
    //(A == 0) ? _90dLeft() : (B == 0) ? SharpLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? SharpRight() : (E == 0) ? _90dRight() : ReadIR();
    //(A == 0) ? SharpLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? MedRight() : (E == 0) ? SharpRight() : ReadIR();
  }
  else if (AIR == 3) //
  {
    (C + D == 0) ? SmoothRight()  : (D + E == 0) ? HardRight() : (C + B == 0) ? SmoothLeft() : (A + B == 0) ? HardLeft() : (A + C == 0) ? _90dLeft() : (C + E == 0) ? _90dRight() : ReadIR();
    //(C + D == 0) ? HardRight() : (A + C == 0) ? _90dLeft() : (C + E == 0) ? _90dRight() : (D + E == 0) ? _90dRight() : (C + B == 0) ? HardLeft() : (A + B == 0) ? _90dLeft() : ReadIR();
    //(C + D == 0) ? SmoothRight() : (D + E == 0) ? HardRight() : (C + B == 0) ? SmoothLeft() : (A + B == 0) ? HardLeft() : ReadIR();
  }
  else if (AIR == 2) {
    (C + D + E == 0) ? _90dRight() : (A + B + C == 0) ? _90dLeft() : ReadIR();
  }
  else if (AIR == 1) {
    A == 0 ? _90dLeft() : _90dRight();
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


//*** Read all IR sensor
void ReadIR() {
  A = analogRead(IRA); A = A / 600; //(A == 0) ? A = 0 : A = 1; // 0 = black, 1 = white
  B = analogRead(IRB); B = B / 600; //(B == 0) ? B = 0 : B = 1;// 0 = black, 1 = white
  C = analogRead(IRC); C = C / 600; //(C == 0) ? C = 0 : C = 1;// 0 = black, 1 = white
  D = analogRead(IRD); D = D / 600; //(D == 0) ? D = 0 : D = 1;// 0 = black, 1 = white
  E = analogRead(IRE); E = E / 600; //(E == 0) ? E = 0 : E = 1;// 0 = black, 1 = white
  R = analogRead(IR_RIGHT); R = R / 600; //(D == 0) ? D = 0 : D = 1;// 0 = white, 1 = black
  L = analogRead(IR_LEFT); L = L / 600; //(E == 0) ? E = 0 : E = 1;// 0 = white, 1 = black
  AIR = A + B + C + D + E;
  //ShowMessges();
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

//void ShowMessges() {
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//
//  // Display Motor Speed
//  display.setCursor(0, 0);
//  display.print(Track_Color);
//  display.print(" Line :");
//
//  display.print(" Def. ");
//  display.print(Default_Turn);
//
//
//  // Display IR value
//  display.setCursor(0, 10);
//  display.print("A:"); display.print(String(A));
//  display.print(" B:"); display.print(String(B));
//  display.print(" C:"); display.print(String(C));
//  display.print(" D:"); display.print(String(D));
//  display.print(" E:"); display.print(String(E));
//
//  // Display Sonar Value;
//  display.setCursor(0, 20);
//  display.print("S2:"); display.print(String(SonarB));
//  display.print(" S1:"); display.print(String(SonarA));
//
//  // Display Decision Value;
//  display.setCursor(20, 30);
//  display.print("**Decision**");
//
//  display.setCursor(0, 40);
//  display.print("Object : "); display.print(Object);
//
//  display.setCursor(0, 50);
//  display.print("Going : "); display.print(Current_Decision);
//
//  display.display();                    // displays content in buffer
//}


void AsyncWait(int interval) {
  TimeCount = millis();
  CurrentTime = TimeCount;
  do {
    TimeLap = TimeCount - CurrentTime;
    TimeCount = millis();
    Serial.print("TimeLap:");
    Serial.println(TimeLap);
    delay(100);
  }
  while (TimeLap <= interval);
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
