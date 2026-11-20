#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ap_ssid = "TrashBot_Proto";
const char* ap_pass = "12345678";

ESP8266WebServer server(80);

// Motor Control Pins (Wemos D1 R1)
const int IN1 = D5; // Left Forward
const int IN2 = D6; // Left Backward
const int IN3 = D7; // Right Forward
const int IN4 = D2; // Right Backward

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>TrashBot AP Control</title>
  <style>
    body { background: #0f172a; color: #f8fafc; font-family: sans-serif; text-align: center; margin: 0; padding: 20px; user-select: none; }
    h2 { margin-bottom: 10px; }
    #status { color: #38bdf8; font-weight: bold; margin-bottom: 25px; }
    .grid { display: grid; grid-template-columns: repeat(3, 95px); grid-gap: 12px; justify-content: center; }
    button {
      height: 85px; font-size: 18px; font-weight: bold; border: none; border-radius: 14px;
      background: #1e293b; color: white; box-shadow: 0 4px 6px rgba(0,0,0,0.4); cursor: pointer;
      touch-action: manipulation;
    }
    button:active { background: #2563eb; transform: scale(0.96); }
    .stop { background: #dc2626; }
    .stop:active { background: #991b1b; }
  </style>
</head>
<body>
  <h2>TrashBot Prototype</h2>
  <div id="status">State: STOP</div>
  <div class="grid">
    <div></div>
    <button onmousedown="send('F')" onmouseup="send('S')" ontouchstart="send('F')" ontouchend="send('S')">FWD</button>
    <div></div>
    <button onmousedown="send('L')" onmouseup="send('S')" ontouchstart="send('L')" ontouchend="send('S')">LEFT</button>
    <button class="stop" onclick="send('S')">STOP</button>
    <button onmousedown="send('R')" onmouseup="send('S')" ontouchstart="send('R')" ontouchend="send('S')">RIGHT</button>
    <div></div>
    <button onmousedown="send('B')" onmouseup="send('S')" ontouchstart="send('B')" ontouchend="send('S')">BACK</button>
    <div></div>
  </div>
  <script>
    function send(cmd) {
      document.getElementById('status').innerText = 'State: ' + cmd;
      fetch('/cmd?c=' + cmd).catch(() => {});
    }
    // Keyboard WASD / Arrow key support for laptop testing
    let activeKey = null;
    document.addEventListener('keydown', (e) => {
      if (e.repeat) return;
      const map = { ArrowUp:'F', w:'F', ArrowDown:'B', s:'B', ArrowLeft:'L', a:'L', ArrowRight:'R', d:'R', ' ':'S' };
      if (map[e.key]) { activeKey = e.key; send(map[e.key]); }
    });
    document.addEventListener('keyup', (e) => {
      if (e.key === activeKey) { activeKey = null; send('S'); }
    });
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopCar();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.println("\nAccess Point Started!");
  Serial.print("WiFi Name: "); Serial.println(ap_ssid);
  Serial.print("Open in Browser: http://"); Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    server.send_P(200, "text/html", index_html);
  });

  server.on("/cmd", []() {
    if (server.hasArg("c")) {
      char cmd = toupper(server.arg("c")[0]);
      if (cmd == 'F') moveForward();
      else if (cmd == 'B') moveBackward();
      else if (cmd == 'L') turnLeft();
      else if (cmd == 'R') turnRight();
      else stopCar();
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}