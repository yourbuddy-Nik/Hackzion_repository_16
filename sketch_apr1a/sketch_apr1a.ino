#define BLYNK_TEMPLATE_ID "TMPL3p0jrKNIr"
#define BLYNK_TEMPLATE_NAME "SGLD System"
#define BLYNK_AUTH_TOKEN "O21nZ_7SV5C16n6jvigbtmH0ZfjHCm50"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// WiFi credentials
char ssid[] = "1234";
char pass[] = "Nikhil@@@";

// Pins
#define MQ2_PIN 34
#define RELAY_PIN 26
#define BUZZER_PIN 25
#define LED_PIN 33
#define SERVO_PIN 27

int gasThreshold = 2000;

Servo myServo;
BlynkTimer timer;

bool alertSent = false;

void checkGas() {
  int gasValue = analogRead(MQ2_PIN);
  Serial.println(gasValue);

  if (gasValue > gasThreshold) {

    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);

    myServo.write(90);

    if (!alertSent) {
      Blynk.logEvent("gas_alert", "⚠️ Gas Leakage Detected!");
      alertSent = true;
    }

  } else {

    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    myServo.write(0);

    alertSent = false;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, checkGas);
}

void loop() {
  Blynk.run();
  timer.run();
}