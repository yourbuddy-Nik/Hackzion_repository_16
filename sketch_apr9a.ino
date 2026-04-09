#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Servo.h>

// Pins
#define GAS_SENSOR_PIN 34
#define LED_PIN 33
#define BUZZER_PIN 25
#define RELAY_FAN_PIN 26
#define SERVO_PIN 27

// Gas detection
const int GAS_THRESHOLD = 300;

// Blynk Auth Token
char auth[] = "Your_Blynk_Auth_Token";

// Wi-Fi credentials
char ssid[] = "vivo T3x 5G";
char pass[] = "Nikhil@@@";

// Servo
Servo gasServo;

// Alert sent flag
bool alertSent = false;

void setup() {
  Serial.begin(115200);

  // Pins setup
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_FAN_PIN, LOW);

  // Servo
  gasServo.attach(SERVO_PIN);
  gasServo.write(0);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run(); // Blynk process

  int gasValue = analogRead(GAS_SENSOR_PIN);
  Serial.println("Gas sensor: " + String(gasValue));

  // Send gas value to Blynk gauge
  Blynk.virtualWrite(V5, gasValue);

  if (gasValue > GAS_THRESHOLD) {
    // Emergency
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RELAY_FAN_PIN, HIGH);
    gasServo.write(90);

    // Update Blynk LEDs
    Blynk.virtualWrite(V1, 255); // LED ON
    Blynk.virtualWrite(V2, 255); // Fan ON
    Blynk.virtualWrite(V3, 255); // Buzzer ON
    alertSent = true;

  } else {
    // Safe
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_FAN_PIN, LOW);
    gasServo.write(0);

    // Update Blynk LEDs
    Blynk.virtualWrite(V1, 0); // LED OFF
    Blynk.virtualWrite(V2, 0); // Fan OFF
    Blynk.virtualWrite(V3, 0); // Buzzer OFF
    alertSent = false;
  }

  delay(1000);
}