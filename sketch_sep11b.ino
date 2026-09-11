#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "ESP32-Servo";
const char* password = "12345678";

WebServer server(80);

Servo servo1;
Servo servo2;

const int servo1Pin = 18;
const int servo2Pin = 19;

bool running = false;

int stepNumber = 0;
unsigned long lastAction = 0;

const unsigned long delayTime = 1000;


// ---------------- WEB PAGE ----------------

void handleRoot() {

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <title>ESP32 Servo Control</title>

  <style>

    body {
      font-family: Arial;
      text-align: center;
      margin-top: 80px;
    }

    button {
      font-size: 25px;
      padding: 15px 40px;
      margin: 10px;
      border-radius: 10px;
      border: none;
    }

    .start {
      background: green;
      color: white;
    }

    .stop {
      background: red;
      color: white;
    }

  </style>
</head>

<body>

<h1>ESP32 Servo Control</h1>

<p>Servo Sequence Controller</p>

<a href="/start">
<button class="start">START</button>
</a>

<a href="/stop">
<button class="stop">STOP</button>
</a>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}


// ---------------- START ----------------

void handleStart() {

  running = true;

  stepNumber = 0;

  servo1.write(0);
  servo2.write(0);

  lastAction = millis();

  server.sendHeader("Location", "/");
  server.send(303);
}


// ---------------- STOP ----------------

void handleStop() {

  running = false;

  servo1.write(0);
  servo2.write(0);

  server.sendHeader("Location", "/");
  server.send(303);
}


// ---------------- SERVO SEQUENCE ----------------

void runSequence() {

  if (!running)
    return;

  unsigned long currentTime = millis();

  if (currentTime - lastAction < delayTime)
    return;

  lastAction = currentTime;

  switch (stepNumber) {

    case 0:
      // Servo 1 → 180°
      servo1.write(180);
      stepNumber = 1;
      break;

    case 1:
      // Servo 2 → 180°
      servo2.write(180);
      stepNumber = 2;
      break;

    case 2:
      // Servo 1 → 0°
      servo1.write(0);
      stepNumber = 3;
      break;

    case 3:
      // Servo 2 → 0°
      servo2.write(0);
      stepNumber = 0;
      break;
  }
}


// ---------------- SETUP ----------------

void setup() {

  Serial.begin(115200);

  // Servo setup
  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);

  servo1.attach(servo1Pin, 500, 2500);
  servo2.attach(servo2Pin, 500, 2500);

  servo1.write(0);
  servo2.write(0);

  // Create ESP32 Wi-Fi network
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("ESP32 Access Point Started");

  Serial.print("WiFi Name: ");
  Serial.println(ssid);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Web server
  server.on("/", handleRoot);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);

  server.begin();

  Serial.println("Web server started");
}


// ---------------- LOOP ----------------

void loop() {

  server.handleClient();

  runSequence();
}