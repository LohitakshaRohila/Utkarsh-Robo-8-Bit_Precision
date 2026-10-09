#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>

// L298N pins
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

const int ENB = 14;
const int IN3 = 32;
const int IN4 = 33;

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  Dabble.begin("DragCar");

  Serial.println("DragCar controller ready!");
}

void loop() {
  Dabble.processInput();

  if (GamePad.isUpPressed()) {
    forward();
  }
  else if (GamePad.isDownPressed()) {
    backward();
  }
  else if (GamePad.isLeftPressed()) {
    left();
  }
  else if (GamePad.isRightPressed()) {
    right();
  }
  else {
    stopMotors();
  }

  delay(20);
}