#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <DabbleESP32.h>

// L298N pins
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

const int ENB = 33;
const int IN3 = 14;
const int IN4 = 13;

// PWM settings
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;  // 0-255
const int MOTOR_SPEED = 200;   // PWM value: 200/255

// ESP32 PWM channels
const int CHANNEL_A = 0;
const int CHANNEL_B = 1;

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, MOTOR_SPEED);
  ledcWrite(ENB, MOTOR_SPEED);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(ENA, MOTOR_SPEED);
  ledcWrite(ENB, MOTOR_SPEED);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, MOTOR_SPEED);
  ledcWrite(ENB, MOTOR_SPEED);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(ENA, MOTOR_SPEED);
  ledcWrite(ENB, MOTOR_SPEED);
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configure PWM
  ledcAttach(ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(ENB, PWM_FREQ, PWM_RESOLUTION);

  stopMotors();

  Dabble.begin("DragCar");

  Serial.println("DragCar controller ready!");
  Serial.println("Motor PWM speed: 180/255");
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
