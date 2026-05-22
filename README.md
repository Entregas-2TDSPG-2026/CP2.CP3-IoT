# CP2 — Estabilizadora com Plataforma Delta

## Descrição

Este projeto implementa uma **plataforma estabilizadora de três eixos** utilizando uma geometria delta — três servos posicionados a 120° entre si — controlados por um ESP32 com base nas leituras de um acelerômetro/giroscópio MPU6050. O objetivo é manter uma superfície nivelada horizontalmente, compensando automaticamente qualquer inclinação detectada em tempo real.

O projeto foi desenvolvido e simulado no **Wokwi**, plataforma de simulação de eletrônica embarcada.

🔗 [Simular no Wokwi](https://wokwi.com/projects/463452907877259265)

---

## Como funciona

O sensor MPU6050 lê continuamente a aceleração nos eixos X e Y. A partir dessas leituras, o firmware calcula o ângulo de inclinação da plataforma e aciona os três servos de forma coordenada para corrigir o desvio.

O controle é feito por um algoritmo **PID (Proporcional-Integral-Derivativo)**, que garante resposta suave e precisa, evitando oscilações:

- **Proporcional (Kp = 1.5)** — reage proporcionalmente ao erro de inclinação atual
- **Integral (Ki = 0.0)** — acumula erros ao longo do tempo (desativado nesta versão)
- **Derivativo (Kd = 0.8)** — amorte variações bruscas, suavizando o movimento

A saída do PID é projetada sobre cada servo usando sua posição angular na geometria delta (A = 90°, B = 210°, C = 330°), calculando a correção individualmente para cada motor através de funções trigonométricas. O loop de controle roda a aproximadamente **50 Hz** (a cada 20 ms).

---

## Hardware

| Componente        | Quantidade | Observação                        |
|-------------------|------------|-----------------------------------|
| ESP32 DevKit V1   | 1          | Microcontrolador principal        |
| MPU6050           | 1          | Acelerômetro e giroscópio I2C     |
| Servo motor (SG90 ou similar) | 3 | Posicionados em geometria delta |

### Conexões

**MPU6050 → ESP32**
| MPU6050 | ESP32   |
|---------|---------|
| VCC     | 3.3V    |
| GND     | GND     |
| SDA     | GPIO 21 |
| SCL     | GPIO 22 |

**Servos → ESP32**
| Servo   | PWM    | Alimentação |
|---------|--------|-------------|
| Servo A | GPIO 13 | VIN / GND  |
| Servo B | GPIO 12 | VIN / GND  |
| Servo C | GPIO 14 | VIN / GND  |

> **Nota:** Os servos são alimentados pelo pino VIN do ESP32. Para uso físico (fora de simulação), recomenda-se alimentação externa para os servos, pois o consumo conjunto pode exceder a capacidade da porta USB.

---

## Software

### Bibliotecas necessárias

- `Wire.h` — comunicação I2C (built-in)
- `MPU6050` — leitura do sensor
- `ESP32Servo` — controle de servos no ESP32

Listadas em `libraries.txt`:
```
MPU6050
ESP32Servo
```

### Estrutura do código (`sketch.ino`)

| Função / Bloco     | Descrição                                                  |
|--------------------|------------------------------------------------------------|
| `setup()`          | Inicializa Serial, I2C, MPU6050 e posiciona servos ao centro (90°) |
| `loop()`           | Lê o sensor, calcula ângulos, executa PID e aciona servos  |
| `setDeltaServos()` | Projeta a correção PID nos três servos usando geometria trigonométrica |

### Parâmetros configuráveis

```cpp
#define SERVO_MID    90    // Posição neutra dos servos (graus)
#define SERVO_RANGE  30    // Amplitude máxima de correção (±30°)

float Kp = 1.5f;           // Ganho proporcional
float Ki = 0.0f;           // Ganho integral
float Kd = 0.8f;           // Ganho derivativo
```

---

## Saída Serial

Durante a execução, o monitor serial (115200 baud) exibe:

```
angleX: -2.34 | angleY: 1.10 | outX: -3.51 | outY: 1.65
```

Útil para depuração e ajuste fino dos ganhos PID.

---

## Autor

**Arthur Brito da Silva - RM562085**
**Luiz Felipe Flosi dos Santos - RM563197**
**Pedro Henrique Brum Lopes - RM561780**
  
Projeto desenvolvido e simulado no Wokwi.