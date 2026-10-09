#include <WiFi.h>
#include <WiFiUdp.h>

// =====================================
// WIFI SETTINGS
// =====================================

const char* WIFI_SSID = "Drag";
const char* WIFI_PASSWORD = "Jitesh2007";

// UDP port on which the ESP32 listens
const unsigned int UDP_PORT = 4210;

WiFiUDP udp;


// =====================================
// L298N PIN CONFIGURATION
// =====================================

// Left motor group
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

// Right motor group
const int ENB = 14;
const int IN3 = 32;
const int IN4 = 33;


// =====================================
// PWM CONFIGURATION
// =====================================

const int PWM_FREQUENCY = 1000;
const int PWM_RESOLUTION = 8;

// PWM range: 0 to 255
const int MAX_PWM = 255;


// =====================================
// SAFETY SETTINGS
// =====================================

// Stop if no valid command arrives for 300 ms.
const unsigned long COMMAND_TIMEOUT_MS = 300;

unsigned long lastCommandTime = 0;

int throttle = 0;
int steering = 0;

bool commandReceived = false;


// =====================================
// MOTOR CONTROL
// =====================================

// Speed range: -255 to +255
//
// Positive = forward
// Negative = reverse
// Zero = stop

void setLeftMotor(int speed) {

  speed = constrain(speed, -MAX_PWM, MAX_PWM);

  if (speed > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
  else if (speed < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  ledcWrite(ENA, abs(speed));
}


void setRightMotor(int speed) {

  speed = constrain(speed, -MAX_PWM, MAX_PWM);

  if (speed > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else if (speed < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  ledcWrite(ENB, abs(speed));
}


void stopMotors() {

  setLeftMotor(0);
  setRightMotor(0);

}


// =====================================
// DIFFERENTIAL STEERING
// =====================================

// throttle: -100 to +100
// steering: -100 to +100
//
// Positive steering = turn right
// Negative steering = turn left

void drive(int throttleValue, int steeringValue) {

  throttleValue = constrain(throttleValue, -100, 100);
  steeringValue = constrain(steeringValue, -100, 100);

  // Convert percentages to PWM units.
  int baseSpeed = map(
    throttleValue,
    -100, 100,
    -MAX_PWM, MAX_PWM
  );

  int turn = map(
    steeringValue,
    -100, 100,
    -MAX_PWM, MAX_PWM
  );

  // Differential steering.
  int leftSpeed = baseSpeed + turn;
  int rightSpeed = baseSpeed - turn;

  // Keep both motor commands in range.
  leftSpeed = constrain(leftSpeed, -MAX_PWM, MAX_PWM);
  rightSpeed = constrain(rightSpeed, -MAX_PWM, MAX_PWM);

  setLeftMotor(leftSpeed);
  setRightMotor(rightSpeed);
}


// =====================================
// WIFI CONNECTION
// =====================================

void connectToWiFi() {

  Serial.print("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

}


// =====================================
// UDP COMMAND PROCESSING
// =====================================

// Expected packet format:
//
// T:70,S:-25
//
// T = throttle percentage (-100 to +100)
// S = steering percentage (-100 to +100)
//
// Also accepts:
//
// STOP
//
// This is a custom format. Configure your phone
// controller to send this format if supported.

bool parseCommand(String packet) {

  packet.trim();

  if (packet == "STOP") {
    throttle = 0;
    steering = 0;
    return true;
  }

  int tIndex = packet.indexOf("T:");
  int sIndex = packet.indexOf("S:");

  if (tIndex == -1 || sIndex == -1) {
    return false;
  }

  int commaIndex = packet.indexOf(',', tIndex);

  if (commaIndex == -1 || sIndex < commaIndex) {
    return false;
  }

  String throttleText = packet.substring(
    tIndex + 2,
    commaIndex
  );

  String steeringText = packet.substring(
    sIndex + 2
  );

  throttleText.trim();
  steeringText.trim();

  // Reject empty values.
  if (throttleText.length() == 0 ||
      steeringText.length() == 0) {
    return false;
  }

  // Convert only after checking the fields.
  throttle = constrain(throttleText.toInt(), -100, 100);
  steering = constrain(steeringText.toInt(), -100, 100);

  return true;
}


// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  // Configure motor direction pins.
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configure PWM outputs.
  if (!ledcAttach(ENA, PWM_FREQUENCY, PWM_RESOLUTION)) {
    Serial.println("Failed to configure ENA PWM!");
    while (true) {
      delay(1000);
    }
  }

  if (!ledcAttach(ENB, PWM_FREQUENCY, PWM_RESOLUTION)) {
    Serial.println("Failed to configure ENB PWM!");
    while (true) {
      delay(1000);
    }
  }

  stopMotors();

  connectToWiFi();

  udp.begin(UDP_PORT);

  Serial.print("UDP listening on port ");
  Serial.println(UDP_PORT);

  Serial.println();
  Serial.println("Waiting for UDP commands...");
  Serial.println("Expected format: T:70,S:-25");
  Serial.println("Emergency stop command: STOP");

}


// =====================================
// MAIN LOOP
// =====================================

void loop() {

  // Read incoming UDP packet.
  int packetSize = udp.parsePacket();

  if (packetSize > 0) {

    // Limit packet size to avoid oversized input.
    char incomingPacket[128];

    int len = udp.read(
      incomingPacket,
      sizeof(incomingPacket) - 1
    );

    if (len > 0) {

      incomingPacket[len] = '\0';

      String packet = String(incomingPacket);

      Serial.print("Received: ");
      Serial.println(packet);

      if (parseCommand(packet)) {

        lastCommandTime = millis();
        commandReceived = true;

        drive(throttle, steering);

      }
      else {
        Serial.println("Invalid command format.");
      }

    }

  }


  // Communication failsafe.
  if (
    commandReceived &&
    millis() - lastCommandTime > COMMAND_TIMEOUT_MS
  ) {

    stopMotors();

    throttle = 0;
    steering = 0;

    commandReceived = false;

    Serial.println("Command timeout! Motors stopped.");

  }


  // Reconnect if Wi-Fi is lost.
  if (WiFi.status() != WL_CONNECTED) {

    stopMotors();

    Serial.println("Wi-Fi disconnected. Reconnecting...");

    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();

    while (
      WiFi.status() != WL_CONNECTED &&
      millis() - startTime < 10000
    ) {
      delay(250);
    }

    if (WiFi.status() == WL_CONNECTED) {

      Serial.println("Wi-Fi reconnected!");
      Serial.print("ESP32 IP address: ");
      Serial.println(WiFi.localIP());

      udp.begin(UDP_PORT);

    }

  }

}