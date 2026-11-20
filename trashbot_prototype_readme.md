# 🗑️ TrashBot Prototype — Wireless Differential Drive Testbed

A lightweight, Wi-Fi-controlled robotic car prototype built using a **Wemos D1 R1 (ESP8266)** and an **L298N Dual H-Bridge Motor Driver**. This prototype serves as a rapid hardware testbed to validate trajectory and movement commands (`FWD`, `BACK`, `LEFT`, `RIGHT`, `STOP`) before deploying onto a full omnidirectional trash-catching robot.

---

## 📋 Features

* **Standalone Wi-Fi Access Point (AP):** Hosts its own local Wi-Fi network (`TrashBot_Proto`) with no external router or internet connection required.
* **Built-in Web Controller:** Serves a responsive, touch-and-keyboard-friendly control interface on Port `80` (`http://192.168.4.1`).
* **Low-Latency HTTP API:** Exposes simple `/cmd?c=<X>` GET endpoints so external computer-vision scripts (e.g., Python + OpenCV / YOLOv8) can drive the chassis programmatically.
* **Differential Drive Kinematics:** Uses two independent rear BO gear motors for forward, reverse, and zero-radius pivot turns, paired with a passive front wheel.

---

## 🛠️ Hardware Components

| Component | Quantity | Notes |
| :--- | :---: | :--- |
| **Wemos D1 R1 (ESP8266)** | 1 | Main microcontroller & Wi-Fi Access Point host |
| **L298N Motor Driver Module** | 1 | Dual H-bridge driver for Left & Right drive channels |
| **BO Gear Motors + Wheels** | 3 | 2 active rear drive wheels + 1 passive front wheel |
| **Battery Pack (7.4V – 12V)** | 1 | e.g., 2S Li-ion pack (overcomes L298N ~1.8V drop & feeds 5V regulator) |
| **Jumper Wires & Chassis** | — | Male-to-Female jumper cables + rigid base plate |

---

## 🔌 Wiring & Pinout

### 1. Wemos D1 R1 $\leftrightarrow$ L298N Connections

| Wemos D1 R1 Pin | ESP8266 GPIO | L298N Pin | Function |
| :--- | :---: | :---: | :--- |
| **D5** | GPIO14 | **IN1** | Left Motor Forward |
| **D6** | GPIO12 | **IN2** | Left Motor Backward |
| **D2** | GPIO4 | **IN3** | Right Motor Forward |
| **D7** | GPIO13 | **IN4** | Right Motor Backward |
| **5V** | — | **+5V** | Powers Wemos D1 from L298N onboard regulator |
| **GND** | — | **GND** | **Common Ground** (Required for signal reference) |

> **Note on Jumpers:** Keep the black jumper caps seated on **ENA** and **ENB** for full-speed digital control, and keep the **5V Regulator Jumper** installed when supplying `7.4V–12V` to the L298N.

### 2. Motor & Power Connections

* **Left Drive Motor:** Connected to `OUT1` and `OUT2`
* **Right Drive Motor:** Connected to `OUT3` and `OUT4`
* **Front Center Motor:** Left disconnected (or internal gears removed) to act as a free-rolling passive wheel during turns.
* **Battery Positive (`+`):** Connected to `+12V` screw terminal on the L298N.
* **Battery Negative (`-`):** Connected to `GND` screw terminal on the L298N (shared with Wemos `GND`).

---

## 💻 Firmware Setup (Arduino IDE)

1. Install the **ESP8266 Board Package** in Arduino IDE (`http://arduino.esp8266.com/stable/package_esp8266com_index.json`).
2. Select **Tools $\rightarrow$ Board $\rightarrow$ esp8266 $\rightarrow$ LOLIN(WEMOS) D1 R1**.
3. Set **Upload Speed** to `115200`.
4. **Important:** Disconnect jumper wires from boot-sensitive pins (`D2`, `D3`, `D4`, `D8`) before clicking **Upload** so the ESP8266 enters flash mode cleanly.
5. Flash the sketch below to the board:

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ap_ssid = "TrashBot_Proto";
const char* ap_pass = "12345678";

ESP8266WebServer server(80);

// Motor Control Pins (Wemos D1 R1)
const int IN1 = D5; // Left Forward
const int IN2 = D6; // Left Backward
const int IN3 = D2; // Right Forward
const int IN4 = D7; // Right Backward

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
```

---

## 🎮 Usage & Controls

1. Power on the L298N with your battery pack.
2. On your phone or laptop, connect to the Wi-Fi network:
   * **SSID:** `TrashBot_Proto`
   * **Password:** `12345678`
3. Open a browser and navigate to **`http://192.168.4.1`** *(make sure to use `http://`, not `https://`)*.
4. Control the prototype using:
   * **Touch / Mouse:** Hold `FWD`, `BACK`, `LEFT`, or `RIGHT` (releasing automatically sends `STOP`).
   * **Keyboard (Laptop):** Use **`W` `A` `S` `D`** or **Arrow Keys**, and **`Spacebar`** to stop.

### Python Automation Endpoint

To integrate this prototype directly with an external vision or tracking script while connected to `TrashBot_Proto`:

```python
import requests

BASE_URL = "http://192.168.4.1/cmd"

def send_command(cmd: str):
    # cmd: 'F' (Forward), 'B' (Backward), 'L' (Left), 'R' (Right), 'S' (Stop)
    requests.get(BASE_URL, params={"c": cmd}, timeout=0.5)

send_command("F")
```

---

## 🔧 Troubleshooting

* **Reversed Motor Direction:** If a wheel spins backward when `FWD` is pressed, swap its two motor wires in the L298N blue screw terminal (`OUT1`/`OUT2` or `OUT3`/`OUT4`).
* **Webpage Not Loading:** Ensure Mobile Data is turned off on smartphones so HTTP requests route through the local `TrashBot_Proto` Wi-Fi interface rather than cellular data.
* **Motors Hum Without Spinning:** Check battery voltage—a single `3.7V` cell loses `~1.8V` across the L298N transistors. Use at least `7.4V` (`2S` Li-ion) for reliable torque.