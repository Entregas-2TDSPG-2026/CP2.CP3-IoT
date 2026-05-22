#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>

// --- Pinos ---
#define SERVO_A 13
#define SERVO_B 12
#define SERVO_C 14

// --- Geometria delta ---
// Ângulos de posição de cada servo na mesa (120° entre si)
#define ANGLE_A  90.0f   // graus
#define ANGLE_B  210.0f
#define ANGLE_C  330.0f

// --- PID ---
float Kp = 1.5f, Ki = 0.0f, Kd = 0.8f;

float errX_prev = 0, errY_prev = 0;
float integX = 0, integY = 0;

// --- Servo ---
Servo servoA, servoB, servoC;
#define SERVO_MID   90   // posição neutra (graus)
#define SERVO_RANGE 30   // amplitude máxima de correção (graus)

// --- MPU ---
MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;

unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 não encontrado!");
    while (1);
  }

  servoA.attach(SERVO_A);
  servoB.attach(SERVO_B);
  servoC.attach(SERVO_C);

  servoA.write(SERVO_MID);
  servoB.write(SERVO_MID);
  servoC.write(SERVO_MID);

  lastTime = millis();
  Serial.println("Sistema iniciado.");
}

void setDeltaServos(float tiltX, float tiltY) {
  // Projeta a correção em cada servo usando sua posição angular na mesa
  float angles[3] = { ANGLE_A, ANGLE_B, ANGLE_C };
  Servo* servos[3] = { &servoA, &servoB, &servoC };

  for (int i = 0; i < 3; i++) {
    float rad = angles[i] * PI / 180.0f;
    float correction = tiltX * cos(rad) + tiltY * sin(rad);
    int pos = SERVO_MID + (int)constrain(correction, -SERVO_RANGE, SERVO_RANGE);
    servos[i]->write(pos);
  }
}

void loop() {
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0f;
  if (dt < 0.02f) return; // ~50 Hz
  lastTime = now;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Converte aceleração em ângulo (rad → graus, escala 16384 = 1g)
  float angleX = atan2((float)ay, (float)az) * 180.0f / PI;
  float angleY = atan2((float)ax, (float)az) * 180.0f / PI;

  // Setpoint: mesa nivelada (0°)
  float errX = 0 - angleX;
  float errY = 0 - angleY;

  integX += errX * dt;
  integY += errY * dt;

  float dX = (errX - errX_prev) / dt;
  float dY = (errY - errY_prev) / dt;

  float outX = Kp * errX + Ki * integX + Kd * dX;
  float outY = Kp * errY + Ki * integY + Kd * dY;

  errX_prev = errX;
  errY_prev = errY;

  setDeltaServos(outX, outY);

  Serial.printf("angleX: %.2f | angleY: %.2f | outX: %.2f | outY: %.2f\n",
                angleX, angleY, outX, outY);
}