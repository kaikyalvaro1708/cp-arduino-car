// =====================================================
// TINYML - PARTE A: LEITURA BRUTA DO SENSOR
// Aula 19 - Project-based Maker Lab
//
// Sensor: HC-SR04 (distância, ultrassônico) + ESP32
// Só lê e mostra o valor CRU, sem nenhum pré-processamento.
// Monitor Serial: 115200
//
// Ligação:
//   VCC  -> pino 5V (VIN) da ESP32
//   GND  -> GND da ESP32
//   TRIG -> GPIO 18
//   ECHO -> GPIO 19, POR UM DIVISOR DE TENSÃO (o ECHO manda
//           5V e a ESP32 só aguenta 3,3V):
//             ECHO --[1 kΩ]--+--> GPIO 19
//                            |
//                          [2 kΩ]
//                            |
//                           GND
// =====================================================

const int TRIG = 18;
const int ECHO = 19;

const unsigned long INTERVALO = 100;   // uma leitura a cada 100 ms

unsigned long ultimaLeitura = 0;


// =====================================================
// SETUP
// =====================================================

void setup() {

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  digitalWrite(TRIG, LOW);

  Serial.begin(115200);

  Serial.println();
  Serial.println("distancia_cm_bruta");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  if (millis() - ultimaLeitura >= INTERVALO) {

    ultimaLeitura = millis();

    Serial.println(lerDistanciaCm());
  }
}


// =====================================================
// MEDIR DISTÂNCIA
// =====================================================

// Devolve a distância em cm, exatamente como o sensor mediu.
// -1 significa "sem eco" (nada dentro do alcance do sensor).
float lerDistanciaCm() {

  digitalWrite(TRIG, LOW);

  delayMicroseconds(2);


  digitalWrite(TRIG, HIGH);

  delayMicroseconds(10);


  digitalWrite(TRIG, LOW);


  long duracao = pulseIn(ECHO, HIGH, 30000);


  if (duracao == 0) {

    return -1;
  }


  return duracao * 0.0343 / 2;
}
