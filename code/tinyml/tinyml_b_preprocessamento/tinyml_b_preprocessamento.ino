// =====================================================
// TINYML - PARTE B: PRÉ-PROCESSAMENTO + CLASSES + CSV
// Aula 19 - Project-based Maker Lab
//
// Sensor: HC-SR04 (distância) + ESP32
// Mesma ligação da parte A (TRIG = GPIO 18, ECHO = GPIO 19
// com divisor de tensão).
// Monitor Serial: 115200
//
// Pré-processamento (3 técnicas):
//   1. Limitação de faixa (2 a 200 cm)
//   2. Média móvel (5 leituras)
//   3. Normalização (0 a 1)
//
// Classes (pela distância média):
//   0 = PERTO   (menos de 20 cm)
//   1 = MEDIO   (de 20 a 60 cm)
//   2 = LONGE   (60 cm ou mais)
//
// Cada linha impressa já está no formato do CSV:
//   feature,label
// =====================================================

const int TRIG = 18;
const int ECHO = 19;

const unsigned long INTERVALO = 100;   // uma leitura a cada 100 ms

unsigned long ultimaLeitura = 0;


// =====================================================
// PRÉ-PROCESSAMENTO
// =====================================================

const float DIST_MIN = 2;      // cm (menor valor aceito)
const float DIST_MAX = 200;    // cm (maior valor aceito)

const int JANELA = 5;          // tamanho da média móvel

float leituras[JANELA];

int posicao = 0;

int quantidade = 0;


// =====================================================
// CLASSES
// =====================================================

const float LIMITE_PERTO = 20;   // cm: abaixo disso = classe 0
const float LIMITE_LONGE = 60;   // cm: a partir daqui = classe 2


// =====================================================
// SETUP
// =====================================================

void setup() {

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  digitalWrite(TRIG, LOW);

  Serial.begin(115200);

  // Cabeçalho do CSV
  Serial.println();
  Serial.println("feature,label");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  if (millis() - ultimaLeitura < INTERVALO) {

    return;
  }

  ultimaLeitura = millis();


  // 1. Leitura crua
  float bruta = lerDistanciaCm();

  // Sem eco = nada por perto: conta como distância máxima
  if (bruta < 0) {

    bruta = DIST_MAX;
  }


  // 2. Limitação de faixa
  float limitada = constrain(bruta, DIST_MIN, DIST_MAX);


  // 3. Média móvel
  leituras[posicao] = limitada;

  posicao = (posicao + 1) % JANELA;

  if (quantidade < JANELA) {

    quantidade++;
  }

  // Espera a janela encher antes de gerar dados
  if (quantidade < JANELA) {

    return;
  }

  float media = 0;

  for (int i = 0; i < JANELA; i++) {

    media += leituras[i];
  }

  media = media / JANELA;


  // 4. Normalização (0 a 1)
  float feature = (media - DIST_MIN) / (DIST_MAX - DIST_MIN);


  // 5. Classe
  int label = classificar(media);


  // Linha do CSV
  Serial.print(feature, 4);
  Serial.print(",");
  Serial.println(label);
}


// =====================================================
// CLASSIFICAR
// =====================================================

int classificar(float distanciaCm) {

  if (distanciaCm < LIMITE_PERTO) {

    return 0;
  }

  if (distanciaCm < LIMITE_LONGE) {

    return 1;
  }

  return 2;
}


// =====================================================
// MEDIR DISTÂNCIA
// =====================================================

// Devolve a distância em cm. -1 = sem eco.
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
