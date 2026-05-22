#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

const char* SSID     = "Arthur_2.4g";
const char* PASSWORD = "Ralados1010";

const float OFFSET_ACC_X = 0.0;
const float OFFSET_ACC_Y = 0.0;

// --- Configuração dos Servos ---
#define SERVO_X_PIN  25   // Servo que corrige inclinação no eixo X
#define SERVO_Y_PIN  26   // Servo que corrige inclinação no eixo Y

// Ângulo central dos servos (posição de nível)
#define SERVO_CENTER     90
// Limite de correção máxima em graus (±) a partir do centro
#define SERVO_MAX_CORR   45
// Ângulo máximo de inclinação considerado para mapeamento (±30°)
#define ANGLE_MAX        30.0
// Threshold: inclinação abaixo desse valor (graus) = considerado nivelado
#define LEVEL_THRESHOLD  3.0
// Suavização do movimento dos servos (0.0 = sem suavização, 1.0 = máxima)
#define SMOOTH_FACTOR    0.15f

Adafruit_MPU6050 mpu;
WebServer server(80);
WebSocketsServer ws(81);
Servo servoX;
Servo servoY;

float smoothAngX = 0.0f;
float smoothAngY = 0.0f;

// Mapeia ângulo de inclinação para posição do servo (corrigida de forma inversa)
int angleToServo(float angle) {
  float clamped = constrain(angle, -ANGLE_MAX, ANGLE_MAX);
  // Inversão: inclinação positiva move servo negativamente para compensar
  float corrected = SERVO_CENTER - (clamped / ANGLE_MAX) * SERVO_MAX_CORR;
  return (int)constrain(corrected, SERVO_CENTER - SERVO_MAX_CORR, SERVO_CENTER + SERVO_MAX_CORR);
}

const char PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Nível ESP32</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:monospace;background:#0f0f0f;color:#eee;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;gap:1.5rem}
  h1{font-size:12px;letter-spacing:.15em;color:#555;text-transform:uppercase}
  .circle{position:relative;width:260px;height:260px;border-radius:50%;border:1.5px solid #2a2a2a;background:#161616;overflow:hidden}
  .ch{position:absolute;width:100%;height:.5px;background:#222;top:50%}
  .cv{position:absolute;height:100%;width:.5px;background:#222;left:50%}
  .cr{position:absolute;width:40px;height:40px;border-radius:50%;border:1px solid #2a2a2a;top:50%;left:50%;transform:translate(-50%,-50%)}
  .bubble{position:absolute;width:36px;height:36px;border-radius:50%;transform:translate(-50%,-50%);transition:left .12s ease,top .12s ease,background .2s}
  .readouts{display:grid;grid-template-columns:1fr 1fr;gap:12px;width:260px}
  .card{background:#161616;border:1px solid #222;border-radius:8px;padding:.75rem 1rem}
  .card-label{font-size:11px;color:#555;margin-bottom:4px}
  .card-val{font-size:20px;font-weight:500}
  .servo-readouts{display:grid;grid-template-columns:1fr 1fr;gap:12px;width:260px}
  .servo-card{background:#111;border:1px solid #1a2a1a;border-radius:8px;padding:.75rem 1rem}
  .servo-label{font-size:11px;color:#3a6a3a;margin-bottom:4px}
  .servo-val{font-size:20px;font-weight:500;color:#5DCAA5}
  .servo-bar-wrap{height:4px;background:#1a1a1a;border-radius:2px;margin-top:6px;overflow:hidden}
  .servo-bar{height:100%;border-radius:2px;background:#1D9E75;transition:width .12s ease}
  .status{font-size:12px;padding:4px 16px;border-radius:99px}
  .ok{background:#04342C;color:#5DCAA5}
  .ng{background:#4A1B0C;color:#F0997B}
  .servo-section-label{font-size:10px;letter-spacing:.12em;color:#333;text-transform:uppercase;width:260px;padding-left:4px}
</style>
</head><body>
<h1>nível — esp32 + mpu6050 + servos</h1>
<div class="circle">
  <div class="ch"></div><div class="cv"></div><div class="cr"></div>
  <div class="bubble" id="b" style="left:50%;top:50%;background:#1D9E75"></div>
</div>
<div class="readouts">
  <div class="card"><div class="card-label">ângulo X</div><div class="card-val" id="ax">—</div></div>
  <div class="card"><div class="card-label">ângulo Y</div><div class="card-val" id="ay">—</div></div>
</div>
<span class="status ok" id="st">aguardando...</span>

<div class="servo-section-label">correção servos</div>
<div class="servo-readouts">
  <div class="servo-card">
    <div class="servo-label">servo X (D35)</div>
    <div class="servo-val" id="sx">—</div>
    <div class="servo-bar-wrap"><div class="servo-bar" id="sbx" style="width:50%"></div></div>
  </div>
  <div class="servo-card">
    <div class="servo-label">servo Y (D34)</div>
    <div class="servo-val" id="sy">—</div>
    <div class="servo-bar-wrap"><div class="servo-bar" id="sby" style="width:50%"></div></div>
  </div>
</div>

<script>
  const ws = new WebSocket('ws://' + location.hostname + ':81');
  ws.onmessage = e => {
    const d = JSON.parse(e.data);
    const px = 50 + (d.y / 30) * 35;
    const py = 50 + (d.x / 30) * 35;
    const b = document.getElementById('b');
    b.style.left = Math.max(10, Math.min(90, px)) + '%';
    b.style.top  = Math.max(10, Math.min(90, py)) + '%';
    const level = Math.sqrt(d.x*d.x + d.y*d.y) < 3;
    b.style.background = level ? '#1D9E75' : '#D85A30';
    document.getElementById('ax').textContent = d.x.toFixed(1) + '°';
    document.getElementById('ay').textContent = d.y.toFixed(1) + '°';
    const st = document.getElementById('st');
    st.textContent = level ? 'nivelado' : 'inclinado';
    st.className = 'status ' + (level ? 'ok' : 'ng');

    // Exibe posição dos servos
    document.getElementById('sx').textContent = d.sx + '°';
    document.getElementById('sy').textContent = d.sy + '°';
    // Barra de 0-180 graus
    document.getElementById('sbx').style.width = ((d.sx / 180) * 100) + '%';
    document.getElementById('sby').style.width = ((d.sy / 180) * 100) + '%';
  };
</script>
</body></html>
)rawhtml";

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("MPU6050 não encontrado!");
    while (1) delay(10);
  }

  // Inicializa servos
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoX.setPeriodHertz(50);
  servoY.setPeriodHertz(50);
  servoX.attach(SERVO_X_PIN, 500, 2400);
  servoY.attach(SERVO_Y_PIN, 500, 2400);

  // Centraliza servos na inicialização
  servoX.write(SERVO_CENTER);
  servoY.write(SERVO_CENTER);
  Serial.println("Servos inicializados no centro (90°)");

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  server.on("/", []() {
    server.send_P(200, "text/html", PAGE);
  });
  server.begin();
  ws.begin();
}

void loop() {
  server.handleClient();
  ws.loop();

  static unsigned long last = 0;
  if (millis() - last > 50) {
    last = millis();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x - OFFSET_ACC_X;
    float ay = a.acceleration.y - OFFSET_ACC_Y;
    float az = a.acceleration.z;

    float angX = atan2(ay, az) * 180.0 / PI;
    float angY = atan2(ax, az) * 180.0 / PI;

    // Suavização exponencial para evitar tremidos nos servos
    smoothAngX = smoothAngX + SMOOTH_FACTOR * (angX - smoothAngX);
    smoothAngY = smoothAngY + SMOOTH_FACTOR * (angY - smoothAngY);

    // Calcula posições dos servos
    int posX = SERVO_CENTER;
    int posY = SERVO_CENTER;

    bool nivelado = (sqrt(smoothAngX * smoothAngX + smoothAngY * smoothAngY) < LEVEL_THRESHOLD);

    if (!nivelado) {
      posX = angleToServo(smoothAngX);
      posY = angleToServo(smoothAngY);
    }

    servoX.write(posX);
    servoY.write(posY);

    // Log serial para debug
    Serial.printf("AngX: %.2f°  AngY: %.2f°  ServoX: %d°  ServoY: %d°  %s\n",
                  smoothAngX, smoothAngY, posX, posY,
                  nivelado ? "[NIVELADO]" : "[CORRIGINDO]");

    // JSON com dados dos ângulos e posição dos servos
    String json = "{\"x\":" + String(angX, 2) +
                  ",\"y\":" + String(angY, 2) +
                  ",\"sx\":" + String(posX) +
                  ",\"sy\":" + String(posY) + "}";
    ws.broadcastTXT(json);
  }
}