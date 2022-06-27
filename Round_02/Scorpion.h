#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>

class Motor {
  private:
    int IN1;
    int IN2;
    int EN;
    int PWM = 0;

  public:
    Motor(int EN, int IN1, int IN2) {
      this -> IN1 = IN1;
      this -> IN2 = IN2;
      this -> EN = EN;
      init();
    };

    void init() {
      // sets the pins as outputs:
      pinMode(IN1, OUTPUT);
      pinMode(IN2, OUTPUT);
      pinMode(EN, OUTPUT);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);

      //analogWrite(EN, 255);
    }

    void Forward() {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
    };

    void Backward() {
      //Serial.println("Backward");
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    };

    void Release() {
      //Serial.println("Stop");
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
    };

    void Status() {
      Serial.println("Pin used : ");
      Serial.println(IN1);
      Serial.println(IN2);
      Serial.println(EN);
    }
    int Speed(int PWM ) {
      analogWrite(EN, PWM);
    }
};



#endif
