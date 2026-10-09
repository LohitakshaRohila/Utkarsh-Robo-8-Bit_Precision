#include <WiFi.h>
#include <WiFiUdp.h>

// ========================================
// WIFI CONFIGURATION
// ========================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const unsigned int UDP_PORT = 4210;

WiFiUDP udp;

// ========================================
// L298N MOTOR DRIVER PINS
// ========================================

// Left-side motors
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

// Right-side motors
const int ENB = 14;
const int IN3 = 32;
const int IN4 = 33;

// ========================================
// PWM CONFIGURATION
// ========================================

const int PWM_FREQUENCY = 1000;
const int PWM_RESOLUTION = 8;

// PWM range: 0 to 255

// ========================================
// MOTOR CONTROL VALUES
// ========================================

int throttle = 0;
int steering = 0;

int leftSpeed = 0;
int rightSpeed = 0;

// ========================================
// MOTOR CONTROL FUNCTION
// ========================================

void setMotor(
    int enablePin,
    int inputPin1,
    int inputPin2,
    int speed
) {
    speed = constrain(speed, -255, 255);

    if (speed > 0) {
        digitalWrite(inputPin1, HIGH);
        digitalWrite(inputPin2, LOW);

        ledcWrite(enablePin, speed);
    }
    else if (speed < 0) {
        digitalWrite(inputPin1, LOW);
        digitalWrite(inputPin2, HIGH);

        ledcWrite(enablePin, -speed);
    }
    else {
        ledcWrite(enablePin, 0);

        digitalWrite(inputPin1, LOW);
        digitalWrite(inputPin2, LOW);
    }
}

// ========================================
// DRIVE FUNCTION
// ========================================

void driveCar(int throttleValue, int steeringValue) {

    throttleValue = constrain(throttleValue, -100, 100);
    steeringValue = constrain(steeringValue, -100, 100);

    // Convert throttle to PWM range
    int baseSpeed = map(
        abs(throttleValue),
        0,
        100,
        0,
        255
    );

    if (throttleValue < 0) {
        baseSpeed = -baseSpeed;
    }

    // Steering adjustment
    int turnAmount = map(
        steeringValue,
        -100,
        100,
        -255,
        255
    );

    // Differential steering
    leftSpeed = baseSpeed + turnAmount;
    rightSpeed = baseSpeed - turnAmount;

    // Keep motor commands in range
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    // Apply motor commands
    setMotor(ENA, IN1, IN2, leftSpeed);
    setMotor(ENB, IN3, IN4, rightSpeed);
}

// ========================================
// WIFI SETUP
// ========================================

void connectWiFi() {

    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    Serial.println("Wi-Fi connected!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
}

// ========================================
// UDP COMMAND HANDLER
// ========================================

void receiveUDP() {

    int packetSize = udp.parsePacket();

    if (packetSize <= 0) {
        return;
    }

    char packet[64];

    int len = udp.read(packet, sizeof(packet) - 1);

    if (len <= 0) {
        return;
    }

    packet[len] = '\0';

    int receivedThrottle;
    int receivedSteering;

    // Expected format:
    // T:70,S:50

    int parsed = sscanf(
        packet,
        "T:%d,S:%d",
        &receivedThrottle,
        &receivedSteering
    );

    if (parsed == 2) {

        throttle = constrain(
            receivedThrottle,
            -100,
            100
        );

        steering = constrain(
            receivedSteering,
            -100,
            100
        );

        driveCar(throttle, steering);

        Serial.print("Throttle: ");
        Serial.print(throttle);

        Serial.print(" | Steering: ");
        Serial.print(steering);

        Serial.print(" | Left PWM: ");
        Serial.print(leftSpeed);

        Serial.print(" | Right PWM: ");
        Serial.println(rightSpeed);
    }
}

// ========================================
// SETUP
// ========================================

void setup() {

    Serial.begin(115200);

    // Configure motor direction pins
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Configure PWM channels
    ledcAttach(ENA, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttach(ENB, PWM_FREQUENCY, PWM_RESOLUTION);

    // Initialize motor outputs
    setMotor(ENA, IN1, IN2, 0);
    setMotor(ENB, IN3, IN4, 0);

    // Connect to Wi-Fi
    connectWiFi();

    // Start UDP listener
    udp.begin(UDP_PORT);

    Serial.print("UDP listening on port ");
    Serial.println(UDP_PORT);

    Serial.println("RC car ready.");
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {

    receiveUDP();

}
