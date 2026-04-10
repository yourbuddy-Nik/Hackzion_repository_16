#define BLYNK_TEMPLATE_ID   "TMPL3p0jrKNIr"
#define BLYNK_TEMPLATE_NAME "SGLD System"
#define BLYNK_AUTH_TOKEN    "O21nZ_7SV5C16n6jvigbtmH0ZfjHCm50"

#include <WiFi.h>
#include <WebServer.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ── WIFI CREDENTIALS ─────────────────────────────
char ssid_sta[] = "hummmm";
char pass_sta[] = "singhcrrish";

const char* ssid_ap = "SGLD_System_Pro";
const char* pass_ap = "Safety123";

// ── PIN MAPPING ──────────────────────────────────
#define MQ2_PIN      34
#define RELAY_PIN    26
#define BUZZER_PIN   25
#define LED_PIN      33
#define SERVO_PIN    27

#define RELAY_ON     LOW
#define RELAY_OFF    HIGH

#define GAS_LIMIT    1200

// ── OBJECTS ──────────────────────────────────────
Servo gasServo;
WebServer server(80);
BlynkTimer timer;

int  currentGas = 0;
bool isDanger = false;
bool systemReady = false;

// ── RELAY SAFETY WRAPPER ─────────────────────────
void controlFan(bool state) {
  digitalWrite(RELAY_PIN, state ? RELAY_ON : RELAY_OFF);
}

// ── SAFE BOOT FUNCTION ────────────────────────────
void safeBoot() {
  pinMode(RELAY_PIN, OUTPUT);
  controlFan(false);   // FORCE FAN OFF
  delay(500);
}

// ── WEB PAGE ──────────────────────────────────────
String getHTML() {
  String color = isDanger ? "#ff4d4d" : "#2ecc71";

  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='1'>";
  html += "</head><body style='text-align:center;font-family:sans-serif;background:#1a1a1a;color:white;'>";

  html += "<h1>SGLD HYBRID MONITOR</h1>";
  html += "<div style='font-size:60px;color:" + color + ";'>" + String(currentGas) + "</div>";

  html += "<h2 style='color:" + color + ";'>STATUS: " + String(isDanger ? "DANGER" : "SAFE") + "</h2>";

  html += "<p>Cloud: " + String(Blynk.connected() ? "ONLINE" : "OFFLINE") + "</p>";

  html += "<button onclick='fetch(\"/reset\")'>RESET</button>";

  html += "</body></html>";
  return html;
}

// ── SAFETY SYSTEM ────────────────────────────────
void runSafetySystem() {
  if (!systemReady) return;

  currentGas = analogRead(MQ2_PIN);
  Serial.println(currentGas);

  if (currentGas > GAS_LIMIT) {

    isDanger = true;

    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);

    gasServo.write(90);   // close valve
    controlFan(true);     // 🔥 FAN ON ONLY HERE

    if (Blynk.connected()) {
      Blynk.virtualWrite(V0, currentGas);
      Blynk.virtualWrite(V1, "⚠ GAS LEAK");
      Blynk.logEvent("gas_leak", "Gas detected: " + String(currentGas));
    }

  } else {

    isDanger = false;

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    gasServo.write(0);    // open valve
    controlFan(false);    // 🔥 FAN OFF

    if (Blynk.connected()) {
      Blynk.virtualWrite(V0, currentGas);
      Blynk.virtualWrite(V1, "SAFE");
    }
  }
}

// ── RESET ─────────────────────────────────────────
void handleReset() {
  isDanger = false;

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  gasServo.write(0);
  controlFan(false);

  server.send(200, "text/plain", "RESET DONE");
}

// ── SETUP ─────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  gasServo.attach(SERVO_PIN);
  gasServo.write(0);

  // 🔥 CRITICAL FIX: Prevent relay boot trigger
  safeBoot();

  systemReady = false;

  // WiFi AP + STA
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid_ap, pass_ap);

  Blynk.config(BLYNK_AUTH_TOKEN);
  WiFi.begin(ssid_sta, pass_sta);

  server.on("/", []() {
    server.send(200, "text/html", getHTML());
  });

  server.on("/reset", handleReset);
  server.begin();

  timer.setInterval(500L, runSafetySystem);

  delay(1000); // stabilize boot

  systemReady = true;

  Serial.println("SYSTEM READY");
}

// ── LOOP ──────────────────────────────────────────
void loop() {
  timer.run();
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
}