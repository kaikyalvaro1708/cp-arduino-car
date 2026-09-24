// =====================================================
// TINYML - PARTE C: RODANDO O MODELO NA ESP32
// Aula 20 - Project-based Maker Lab
//
// Usa a biblioteca exportada do Edge Impulse
// (projeto "tinyml-ldr-ZOE"), instalada em
// Documentos/Arduino/libraries.
//
// Faz o mesmo pré-processamento da parte B, entrega a
// distância normalizada para o modelo e mostra:
//   - a classe que o MODELO previu
//   - a classe que a REGRA (20 e 60 cm) daria
//   - quanto os dois concordam
//
// Ligação do sensor: igual à parte A (TRIG = GPIO 18,
// ECHO = GPIO 19 com divisor de tensão).
// Monitor Serial: 115200
// =====================================================

#include <tinyml-ldr-ZOE_inferencing.h>

const int TRIG = 18;
const int ECHO = 19;

// Pinos do L298N: mantidos desligados (LOW) para os motores
// não girarem sozinhos enquanto este sketch roda.
const int IN1 = 25;
const int IN2 = 26;
const int IN3 = 27;
const int IN4 = 14;

const unsigned long INTERVALO = 100;

unsigned long ultimaLeitura = 0;


// =====================================================
// PRÉ-PROCESSAMENTO (igual à parte B)
// =====================================================

const float DIST_MIN = 2;
const float DIST_MAX = 200;

const int JANELA = 5;

float leituras[JANELA];

int posicao = 0;

int quantidade = 0;


// =====================================================
// CLASSES
// =====================================================

const float LIMITE_PERTO = 20;
const float LIMITE_LONGE = 60;

const char* NOMES[3] = { "PERTO", "MEDIO", "LONGE" };


// =====================================================
// MODELO
// =====================================================

// O modelo recebe 1 valor: a distância normalizada (0 a 1)
float features[EI_CLASSIFIER_RAW_SAMPLE_COUNT];

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {

  memcpy(out_ptr, features + offset, length * sizeof(float));

  return 0;
}

unsigned long totalPrevisoes = 0;

unsigned long concordancias = 0;


// =====================================================
// SETUP
// =====================================================

void setup() {

  // Motores desligados
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  digitalWrite(TRIG, LOW);

  Serial.begin(115200);

  Serial.println();
  Serial.println("================================");
  Serial.println("   TINYML - MODELO NA ESP32");
  Serial.println("================================");

  Serial.print("Projeto: ");
  Serial.println(EI_CLASSIFIER_PROJECT_NAME);

  Serial.print("Classes: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  if (millis() - ultimaLeitura < INTERVALO) {

    return;
  }

  ultimaLeitura = millis();


  // 1. Leitura crua (-1 = sem eco = nada por perto)
  float bruta = lerDistanciaCm();

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

  if (quantidade < JANELA) {

    return;
  }

  float media = 0;

  for (int i = 0; i < JANELA; i++) {

    media += leituras[i];
  }

  media = media / JANELA;


  // 4. Normalização (0 a 1): é isso que o modelo recebe
  features[0] = (media - DIST_MIN) / (DIST_MAX - DIST_MIN);


  // 5. Roda o modelo
  signal_t sinal;

  sinal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
  sinal.get_data = &raw_feature_get_data;

  ei_impulse_result_t resultado = { 0 };

  EI_IMPULSE_ERROR erro = run_classifier(&sinal, &resultado, false);

  if (erro != EI_IMPULSE_OK) {

    Serial.print("ERRO no modelo: ");
    Serial.println(erro);

    return;
  }


  // 6. Classe com maior probabilidade
  int previsao = 0;

  for (int i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {

    if (resultado.classification[i].value >
        resultado.classification[previsao].value) {

      previsao = i;
    }
  }


  // 7. Classe pela regra dos limites, para comparar
  int regra = classificar(media);

  totalPrevisoes++;

  if (previsao == regra) {

    concordancias++;
  }


  Serial.printf(
    "dist=%6.1f cm | feature=%.4f | modelo=%s (%3.0f%%) | regra=%s | %s | concordancia=%.1f%% | %d ms\n",
    media,
    features[0],
    NOMES[previsao],
    resultado.classification[previsao].value * 100,
    NOMES[regra],
    previsao == regra ? "OK  " : "DIFF",
    100.0 * concordancias / totalPrevisoes,
    resultado.timing.classification
  );
}


// =====================================================
// CLASSIFICAR PELA REGRA
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
