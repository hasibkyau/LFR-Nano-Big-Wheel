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
String Current_Decision = " ";
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
#define IRE A6
#define IRF A7
int A = 0, B = 0, C = 0, D = 0, E = 0, F = 1, AIR; //IR variable for store value

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
const int TST = 500; // Track searching time (FM - 180)
const int _90dTtime = 0; // time need for turning 90 degree
const int _180Ttime = 0; // time need for turning 180 degree
const int TBT = 100; // time before turning (FM - 120)
const int TAT = 50; // time after taking turn for distracting from current track (FM - 35)

//For asynchronous function
unsigned long TimeCount;
unsigned long CurrentTime;
unsigned long TimeLap;

//#define touch_pin_numer T0
//const int VALUE_THRESHOLD = 30;
//int TOUCH_SENSOR_VALUE, dt = 1;//// default turn (1 = right, 0   = left)

////GPIO PINS
//int BUZZER = 15; // BUZZER

//Motor Driver pins
int ENA = 3, IN1 = 4, IN2 = 5, ENB = 6, IN3 = 7, IN4 = 8;// For Motor Driver

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
  Beep(3, 500);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  //  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  //  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //    Serial.println(F("SSD1306 allocation failed"));
  //    for (;;); // Don't proceed, loop forever
  //  }
  // delay(1000);

  MotorR.Forward();
  MotorL.Forward();
  delay(1000);

  int spd = 170;

  while (true) {
    ReadIR();
    //ReadSonar();
    //Straight();
    ShowMessges();
    //
    MotorR.Forward();
    MotorL.Backward();
    MotorR.Speed(200);
    MotorL.Speed(200);
    delay(100);
    //spd += 5;
    //
    //(spd == 255) ? spd = 255 : spd = spd ;

  }
  //  delay(1000);
}

//*** Default turn
void DefaultTurn() {
  (Track_Color == "right") ? _90dRight() : _90dLeft();
}

void loop() {
  ReadIR(); // reading IR data
  //ReadSonar();
  FollowTrack();
  ShowMessges();

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
  MotorR.Speed(R_max_speed);//left motor is bit damaged thats why used more duty cycle than right motor
  MotorL.Speed(L_max_speed);
}

//*** Smooth Left Turn - ok
void SmoothLeft() {
  MotorL.Speed(L_max_speed - 20);
  MotorR.Speed(R_max_speed);
}

//*** Smooth Right Turn - ok
void SmoothRight() {
  MotorL.Speed(L_max_speed);
  MotorR.Speed(R_max_speed - 35);
}


//*** Medium Left Turn - ok
void MedLeft() {
  MotorL.Speed(L_max_speed - 55);
  MotorR.Speed(R_max_speed);
}


//*** Medium Right Turn - ok
void MedRight() {
  MotorL.Speed(max_speed);
  MotorR.Speed(R_max_speed - 40);
}

//hard left - ok
void HardLeft() {
  MotorL.Speed(max_speed - 95);
  MotorR.Speed(R_max_speed);
}

//Hard right - ok
void HardRight() {
  MotorL.Speed(max_speed);
  MotorR.Speed(R_max_speed - 75);

}

//*** Sharp Left Turn - ok
void SharpLeft() {
  MotorL.Release();
  MotorR.Speed(180);
  MotorL.Speed(180);
  do {
    ReadIR();
  }
  while ( C != 0);
  MotorL.Forward();
}

//*** Sharp Right Turn - ok
void SharpRight() {
  MotorR.Release();
  MotorR.Speed(180);
  MotorL.Speed(180);
  do {
    ReadIR();
  }
  while ( C != 0);
  MotorR.Forward();
}

//*** 90d left turn
void _90dLeft() {
//  Straight();
//  delay(TBT);
  Neutral(); delay(100);
  MotorR.Forward(); MotorL.Backward();
  MotorL.Speed(170); MotorR.Speed(170);
  //delay(TAT);// if this is a 4 line it will distrac from the middle line within 10 mili second
  do {
    ReadIR();
  }
  while (!(AIR == 4 && C == 0));
  Neutral(); delay(10);
  MotorL.Forward(); MotorR.Forward();
}

//*** 90d Right Turn
void _90dRight() {
//  Straight();
//  delay(TBT);
  //(AIR < 5)?Straight():Neutral(); //if there is multiple line it will take the middle line
  Neutral(); delay(10);
  MotorL.Forward(); MotorR.Backward();
  MotorL.Speed(170); MotorR.Speed(170);
  //MotorL.Speed(max_speed); MotorR.Speed(0);
  //delay(TAT);// if this is a 4 line it will distrac from the middle line within 10 mili second
  do {
    ReadIR();
  }
  while (!(AIR == 4 && C == 0));
  Neutral(); delay(10);
  MotorL.Forward(); MotorR.Forward();
}

//*** 180d turn on place
void _180dTurn() {
  //Serial.println("Taking U turn"); delay(2000);
  Neutral(); // Both motor stop with neutral gear
  delay(10);
  MotorL.Forward(); MotorR.Backward();// Rotate on place
  MotorR.Speed(max_speed); MotorL.Speed(max_speed);
  do {
    ReadIR();
  }
  while (AIR == 5); // finish 180d ?
  Brake(); // Both motor stop with neutral gear
  Neutral();
  delay(10);// for refreshin dirver logic
  MotorL.Forward(); MotorR.Forward();
}

//int Beep(int n, int dly) {
//  int i = 0;
//  while (i < n) {
//    digitalWrite(BUZZER, HIGH);
//    delay(dly);
//    digitalWrite(BUZZER, LOW);
//    delay(dly);
//    i++;
//  }
//}
//
//Follow track
void FollowTrack() {
  if (AIR == 4)//On track
  {
    (A == 0) ? SharpLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? MedRight() : (E == 0) ? SharpRight() : ReadIR();
    //(A == 0) ? HardLeft() : (B == 0) ? MedLeft() : (C == 0 ) ? Straight() : ( D == 0 ) ? MedRight() : HardRight();
  }
  else if (AIR == 3) //
  {
    (C + D == 0) ? SmoothRight() : (D + E == 0) ? HardRight() : (C + B == 0) ? SmoothLeft() : (A + B == 0) ? HardLeft() : ReadIR();
  }
  else if (AIR == 2 || AIR == 1) {
    Straight();
    delay(TBT);
    (A == 1) ? _90dRight() : _90dLeft();
  }
  else if (AIR == 0)//multiple line
  {
    Straight();
    delay(TBT);
    DefaultTurn();
  }
  else if (AIR == 5)// White space
  {
    //Serial.println("white space!");delay(500);
    Straight(); //go 14cm forward
    TimeCount = millis();
    CurrentTime = TimeCount;
    do {
      TimeLap = TimeCount - CurrentTime;
      TimeCount = millis();
      Serial.print("TimeLap:");
      Serial.println(TimeLap);
      ReadIR();
    }
    while ((TimeLap <= TST) && AIR == 5); //if any way found or the time is up
    //delay(TST);
    (AIR == 5) ? _180dTurn() : Brake();
    TimeLap = 0;
  }
}


//***Avoid obstacle if found
//for default turn == left
void AvoidObstacle() {
  int max_spd, min_spd;
  if (Track_Color == "right") {
    max_spd = 0;
    min_spd = 255;
  }
  else {
    max_spd = 255;
    min_spd = 0;
  }
  Brake();
  Serial.println("Obstacle found!"); delay(500);
  MotorL.Speed(min_spd); MotorR.Speed(max_spd);// Right turn
  delay(TST);
  MotorR.Speed(min_spd); MotorL.Speed(max_spd);// Left turn
  delay(TST);
  MotorR.Speed(255); MotorL.Speed(255);// Straight forward
  delay(TST);
  MotorR.Speed(min_spd); MotorL.Speed(max_spd);// Right turn
  delay(TST);
  do {
    MotorR.Speed(255); MotorL.Speed(255);// Straight
    ReadIR();
  }
  while (AIR == 5);
  //Brake();
  Serial.println("Obstacle skiped!"); delay(500);
}

//*** Read all IR sensor
void ReadIR() {
  A = analogRead(IRA); A = A / 1000; //(A == 0) ? A = 0 : A = 1; // 0 = black, 1 = white
  B = analogRead(IRB); B = B / 1000; //(B == 0) ? B = 0 : B = 1;// 0 = black, 1 = white
  C = analogRead(IRC); C = C / 1000; //(C == 0) ? C = 0 : C = 1;// 0 = black, 1 = white
  D = analogRead(IRD); D = D / 1000; //(D == 0) ? D = 0 : D = 1;// 0 = black, 1 = white
  E = analogRead(IRE); E = E / 1000; //(E == 0) ? E = 0 : E = 1;// 0 = black, 1 = white
  F = analogRead(IRF); F = F / 1000; //(F == 0) ? F = 0 : F = 1;// 0 = black, 1 = white
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
  Serial.print(":F=");
  Serial.print(F);
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

void ShowMessges() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Display Motor Speed
  display.setCursor(0, 0);
  display.print(Track_Color);
  display.print(" Line :");

  display.print(" Def. ");
  display.print(Default_Turn);


  // Display IR value
  display.setCursor(0, 10);
  display.print("A:"); display.print(String(A));
  display.print(" B:"); display.print(String(B));
  display.print(" C:"); display.print(String(C));
  display.print(" D:"); display.print(String(D));
  display.print(" E:"); display.print(String(E));

  // Display Sonar Value;
  display.setCursor(0, 20);
  display.print("S2:"); display.print(String(SonarB));
  display.print(" S1:"); display.print(String(SonarA));

  // Display Decision Value;
  display.setCursor(20, 30);
  display.print("**Decision**");

  display.setCursor(0, 40);
  display.print("Object : "); display.print(Object);

  display.setCursor(0, 50);
  display.print("Going : "); display.print(Current_Decision);

  display.display();                    // displays content in buffer
}


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
