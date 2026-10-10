#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <DabbleESP32.h>

// MOTOR PINS

// Left motor group
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

// Right motor group
const int ENB = 33;
const int IN3 = 14;
const int IN4 = 13;

// ULTRASONIC PINS

const int TRIG_LEFT = 34;
const int ECHO_LEFT = 35;

const int TRIG_CENTER = 5;
const int ECHO_CENTER = 19;

const int TRIG_RIGHT = 21;
const int ECHO_RIGHT = 22;

// SETTINGS

const int MOTOR_SPEED = 200;

const unsigned long SENSOR_INTERVAL = 250;
const unsigned long ECHO_TIMEOUT = 25000;

unsigned long lastSensorRead = 0;

float leftDistance = -1;
float centerDistance = -1;
float rightDistance = -1;

// SETUP

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);

  pinMode(TRIG_CENTER, OUTPUT);
  pinMode(ECHO_CENTER, INPUT);

  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  digitalWrite(TRIG_LEFT, LOW);
  digitalWrite(TRIG_CENTER, LOW);
  digitalWrite(TRIG_RIGHT, LOW);

  stopCar();

  Dabble.begin("ESP32_RC_Car");

  Serial.println("RC Car + 3 Ultrasonic Sensors");
  Serial.println("Connect Dabble GamePad");
  Serial.println("--------------------------------");
}

// MAIN LOOP

void loop() {
  // Process Bluetooth commands
  Dabble.processInput();

  // Drive the car
  if (GamePad.isUpPressed()) {
    drive(1, 1);
  }
  else if (GamePad.isDownPressed()) {
    drive(-1, -1);
  }
  else if (GamePad.isLeftPressed()) {
    drive(-1, 1);
  }
  else if (GamePad.isRightPressed()) {
    drive(1, -1);
  }
  else {
    stopCar();
  }

  // Read all sensors periodically
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = millis();

    leftDistance = readDistance(TRIG_LEFT, ECHO_LEFT);

    delay(35);

    centerDistance = readDistance(TRIG_CENTER, ECHO_CENTER);

    delay(35);

    rightDistance = readDistance(TRIG_RIGHT, ECHO_RIGHT);

    printDistances();
  }
}

// ================= SENSOR FUNCTION =================

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  unsigned long duration =
      pulseIn(echoPin, HIGH, ECHO_TIMEOUT);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;
}

// ================= DISPLAY READINGS =================

void printDistances() {
  Serial.print("LEFT: ");

  if (leftDistance >= 0) {
    Serial.print(leftDistance, 1);
    Serial.print(" cm");
  } else {
    Serial.print("No echo");
  }

  Serial.print(" | CENTER: ");

  if (centerDistance >= 0) {
    Serial.print(centerDistance, 1);
    Serial.print(" cm");
  } else {
    Serial.print("No echo");
  }

  Serial.print(" | RIGHT: ");

  if (rightDistance >= 0) {
    Serial.print(rightDistance, 1);
    Serial.println(" cm");
  } else {
    Serial.println("No echo");
  }
}

// ================= MOTOR CONTROL =================

void drive(int leftDirection, int rightDirection) {
  // Left motor group
  digitalWrite(IN1, leftDirection > 0 ? HIGH : LOW);
  digitalWrite(IN2, leftDirection > 0 ? LOW : HIGH);

  // Right motor group
  digitalWrite(IN3, rightDirection > 0 ? HIGH : LOW);
  digitalWrite(IN4, rightDirection > 0 ? LOW : HIGH);

  analogWrite(ENA, MOTOR_SPEED);
  analogWrite(ENB, MOTOR_SPEED);
}

void stopCar() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
