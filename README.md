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

# CP2 — Mesa Estabilizadora com ESP32 + MPU6050

## Descrição

Este projeto implementa uma **plataforma estabilizadora automática** controlada por um ESP32. O sensor MPU6050 detecta a inclinação da superfície em tempo real e dois servos corrigem o ângulo nos eixos X e Y, mantendo a plataforma sempre nivelada.

O sistema conta ainda com uma **interface web embarcada**, acessada via Wi-Fi, que exibe uma bolha de nível animada, os ângulos medidos e as posições dos servos — tudo em tempo real via WebSocket, sem necessidade de nenhum aplicativo externo.

O projeto possui duas versões:

- **Simulação (Wokwi)** — geometria delta com 3 servos, algoritmo PID completo  
- **Físico (Arduino IDE)** — 2 servos (eixo X e Y), suavização exponencial, dashboard web via Wi-Fi

🔗 [Simular no Wokwi](https://wokwi.com/projects/463452907877259265)

---

## Como funciona

O sensor MPU6050 lê a aceleração nos três eixos via I2C. Com esses valores, o firmware calcula os ângulos de inclinação usando `atan2`. Uma **suavização exponencial** filtra leituras instáveis antes de acionar os servos, evitando tremidos.

Se a inclinação total estiver abaixo do threshold (`LEVEL_THRESHOLD = 3°`), os servos permanecem na posição central — evitando micro-correções desnecessárias. Caso contrário, cada servo recebe uma posição calculada de forma inversa ao ângulo medido, compensando o desvio.

Os dados são transmitidos via **WebSocket (porta 81)** a 50 Hz para a página web embarcada no próprio ESP32.

---

## Hardware

| Componente              | Quantidade | Observação                                   |
|-------------------------|------------|----------------------------------------------|
| ESP32 DevKit V1         | 1          | Microcontrolador principal                   |
| MPU6050                 | 1          | Acelerômetro/giroscópio I2C (endereço 0x68)  |
| Servo motor SG90        | 2          | Um para eixo X, um para eixo Y               |

### Conexões

**MPU6050 → ESP32**

| MPU6050 | ESP32    |
|---------|----------|
| VCC     | 3.3V     |
| GND     | GND      |
| SDA     | GPIO 21  |
| SCL     | GPIO 22  |

**Servos → ESP32**

| Servo          | Pino PWM | Alimentação  |
|----------------|----------|--------------|
| Servo X (eixo X) | GPIO 25 | 5V externo / GND |
| Servo Y (eixo Y) | GPIO 26 | 5V externo / GND |

---

## Software

### Bibliotecas necessárias

Instale pela Arduino IDE em **Sketch → Include Library → Manage Libraries**:

| Biblioteca          | Uso                                      |
|---------------------|------------------------------------------|
| `Adafruit MPU6050`  | Leitura do sensor via driver Adafruit    |
| `Adafruit Unified Sensor` | Dependência do driver Adafruit     |
| `ESP32Servo`        | Controle de servos no ESP32              |
| `WebSockets` (Markus Sattler) | Servidor WebSocket para o dashboard |
| `Wire` (built-in)   | Comunicação I2C                          |
| `WiFi` (built-in)   | Conexão à rede Wi-Fi                     |
| `WebServer` (built-in) | Servidor HTTP para a página web       |

### Configuração Wi-Fi

Antes de gravar, edite as credenciais no início do arquivo:

```cpp
const char* SSID     = "SUA_REDE";
const char* PASSWORD = "SUA_SENHA";
```

Após a inicialização, o IP do ESP32 é exibido no Monitor Serial. Acesse esse IP no navegador para abrir o dashboard.

### Parâmetros configuráveis

```cpp
#define SERVO_X_PIN     25      // Pino PWM do servo do eixo X
#define SERVO_Y_PIN     26      // Pino PWM do servo do eixo Y

#define SERVO_CENTER    90      // Posição neutra (graus)
#define SERVO_MAX_CORR  45      // Amplitude máxima de correção (±45°)
#define ANGLE_MAX       30.0    // Ângulo máximo mapeado (±30°)
#define LEVEL_THRESHOLD  3.0   // Abaixo disso, considera nivelado
#define SMOOTH_FACTOR   0.15f  // Suavização exponencial (0 = nenhuma, 1 = máxima)

const float OFFSET_ACC_X = 0.0; // Calibração do eixo X (ajustar se necessário)
const float OFFSET_ACC_Y = 0.0; // Calibração do eixo Y (ajustar se necessário)
```

### Estrutura do código (`MesaEstabilizadora.ino`)

| Função / Bloco      | Descrição                                                                 |
|---------------------|---------------------------------------------------------------------------|
| `angleToServo()`    | Converte ângulo de inclinação em posição de servo com inversão de sentido |
| `setup()`           | Inicializa Serial, I2C, MPU6050, servos, Wi-Fi, HTTP server e WebSocket   |
| `loop()`            | Lê sensor, suaviza, calcula posições, aciona servos e transmite JSON via WS |
| `PAGE` (PROGMEM)    | Página HTML/CSS/JS completa da interface web, armazenada em flash          |

---

## Interface Web

Acesse pelo navegador o IP exibido no Serial Monitor após a conexão Wi-Fi.

A interface exibe:

- **Bolha de nível** animada — indica visualmente a direção da inclinação
- **Ângulo X e Y** em graus, atualizados em tempo real
- **Status** — `nivelado` (verde) ou `inclinado` (laranja)
- **Posição dos servos** X e Y em graus, com barra de progresso

A comunicação entre o ESP32 e o browser é feita via **WebSocket na porta 81**, com envio de JSON a cada 50 ms:

```json
{"x": -2.34, "y": 1.10, "sx": 93, "sy": 87}
```

---

## Saída Serial

O Monitor Serial (115200 baud) exibe o estado a cada ciclo:

```
AngX: -2.34°  AngY: 1.10°  ServoX: 93°  ServoY: 87°  [CORRIGINDO]
AngX: -0.41°  AngY: 0.22°  ServoX: 90°  ServoY: 90°  [NIVELADO]
```

Útil para calibrar os offsets e ajustar o `SMOOTH_FACTOR`.

---

## Versões do Projeto

| Versão      | Arquivo                    | Servos | Algoritmo           | Interface     |
|-------------|----------------------------|--------|---------------------|---------------|
| Simulação   | `sketch.ino` (Wokwi)       | 3 (delta) | PID (Kp/Ki/Kd)  | Monitor Serial |
| Físico      | `MesaEstabilizadora.ino`   | 2 (X/Y) | Suavização exponencial | Dashboard Web via Wi-Fi |

---

## Autor

**Arthur Brito da Silva - RM562085**
**Luiz Felipe Flosi dos Santos - RM563197**
**Pedro Henrique Brum Lopes - RM561780**  

  Projeto desenvolvido e simulado no Wokwi.