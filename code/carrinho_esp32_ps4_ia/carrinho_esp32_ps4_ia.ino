// =====================================================
// CARRINHO - ESP32 + L298N + PS4 + TINYML (Edge Impulse)
//
// Igual ao carrinho_esp32_ps4, mais um sensor de distância
// (HC-SR04) e o modelo treinado no Edge Impulse
// ("tinyml-ldr-ZOE"), que classifica o que está na frente:
//   PERTO (menos de ~20 cm), MEDIO (~20 a ~60 cm), LONGE
//
// Placa na IDE: ESP32 + Bluepad32 Arduino > DOIT ESP32 DEVKIT V1
// Biblioteca: tinyml-ldr-ZOE_inferencing (zip do Edge Impulse)
//
// Como parear (primeira vez):
//   Segure SHARE + botão PS até a luz do controle piscar.
//
// Controles (estilo jogo de corrida):
//   R2 = acelerar (frente)
//   L2 = ré
//   R2 + L2 juntos = freio (para)
//   Analógico esquerdo (ou D-pad esquerda / direita) = virar
//   Botão X (✕): parada de emergência
//
// O que o modelo faz:
//   PERTO: o carrinho não deixa ir para a frente (ré pode).
//          Luz vermelha e vibração forte no controle.
//   MEDIO: aviso. Indo para a frente, a luz fica roxa e o
//          controle vibra de leve.
//   LONGE: normal.
//
// Luz do controle: verde = parado, azul = andando,
//   amarelo = ré, roxo = andando com obstáculo médio,
//   vermelho = emergência ou obstáculo perto
//
// Ligação do sensor:
//   VCC -> 5V (VIN), GND -> GND, TRIG -> GPIO 18,
//   ECHO -> GPIO 19 por divisor de tensão (1 kΩ + 2 kΩ)
// =====================================================

#include <Bluepad32.h>
#include <tinyml-ldr-ZOE_inferencing.h>


// =====================================================
// L298N
// =====================================================

// -----------------------------
// Motor A - Esquerda (OUT1/OUT2)
// -----------------------------
const int IN1 = 25;
const int IN2 = 26;

// -----------------------------
// Motor B - Direita (OUT3/OUT4)
// -----------------------------
const int IN3 = 27;
const int IN4 = 14;

// ENA e ENB ficam com jumper na ponte H (velocidade máxima)


// =====================================================
// SENSOR DE DISTÂNCIA (HC-SR04)
// =====================================================

const int TRIG = 18;
const int ECHO = 19;

const unsigned long INTERVALO_SENSOR = 100;

unsigned long ultimaLeituraSensor = 0;


// =====================================================
// PRÉ-PROCESSAMENTO (igual ao usado no treino)
// =====================================================

const float DIST_MIN = 2;      // cm
const float DIST_MAX = 200;    // cm

const int JANELA = 5;          // média móvel

float leituras[JANELA];

int posicao = 0;

int quantidade = 0;


// =====================================================
// MODELO
// =====================================================

const int CLASSE_PERTO = 0;
const int CLASSE_MEDIO = 1;
const int CLASSE_LONGE = 2;

const char* NOMES_CLASSES[3] = { "PERTO", "MEDIO", "LONGE" };

// O modelo recebe 1 valor: a distância normalizada (0 a 1)
float features[EI_CLASSIFIER_RAW_SAMPLE_COUNT];

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {

  memcpy(out_ptr, features + offset, length * sizeof(float));

  return 0;
}

int classeObstaculo = CLASSE_LONGE;

float distanciaMedia = DIST_MAX;

// Perto da fronteira entre duas classes a resposta do modelo
// pode ficar trocando. Por isso a classe só muda depois de
// algumas leituras seguidas iguais (histerese). Entrar em
// PERTO é imediato (segurança); sair de PERTO demora mais.
const int CONFIRMACOES_NORMAL = 3;    // 0,3 s
const int CONFIRMACOES_LIBERAR = 5;   // 0,5 s (saindo de PERTO)

int classeCandidata = CLASSE_LONGE;

int confirmacoes = 0;


// =====================================================
// CONTROLE
// =====================================================

ControllerPtr controle = nullptr;

// O analógico vai de -511 a +512. Abaixo disso, ignora
// (o analógico nunca volta exatamente ao zero).
const int ZONA_MORTA = 200;

// R2 e L2 vão de 0 (solto) a 1023 (apertado até o fim).
// Acima deste valor, o gatilho conta como apertado.
const int GATILHO_MINIMO = 100;

// Coloque true se o controle não parear (apaga os pareamentos
// guardados na ESP32). Depois volte para false.
const bool ESQUECER_PAREAMENTOS = false;


// =====================================================
// ESTADO
// =====================================================

char movimentoAtual = 'S';

bool emergenciaAtiva = false;

// Frente bloqueada pelo modelo (obstáculo PERTO)
bool bloqueadoPorObstaculo = false;

// A saudação (luz + vibração) só pode ser enviada depois que o
// controle mandou o primeiro dado, por isso fica pendente.
bool saudacaoPendente = false;

// Última cor enviada à luz do controle (só envia se mudar)
long ultimaCor = -1;


// =====================================================
// BATERIA DO CONTROLE
// =====================================================

// Nível vai de 1 (vazia) a 255 (cheia). 0 = desconhecido.
const int BATERIA_BAIXA = 50;

const unsigned long INTERVALO_BATERIA = 30000;

unsigned long ultimaChecagemBateria = 0;


// =====================================================
// SETUP
// =====================================================

void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Começa parado
  motorA(0);
  motorB(0);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  digitalWrite(TRIG, LOW);

  Serial.begin(115200);

  BP32.setup(&aoConectarControle, &aoDesconectarControle);

  if (ESQUECER_PAREAMENTOS) {

    BP32.forgetBluetoothKeys();
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("  CARRINHO ESP32 + TINYML");
  Serial.println("================================");

  Serial.print("Modelo: ");
  Serial.println(EI_CLASSIFIER_PROJECT_NAME);

  Serial.println("Segure SHARE + PS no controle para parear.");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  BP32.update();

  atualizarSensor();

  if (controle && controle->isConnected() && controle->isGamepad()) {

    if (saudacaoPendente && controle->hasData()) {

      saudacaoPendente = false;

      saudarControle(controle);
    }

    lerControle(controle);

    checarBateria(controle);
  }

  // Sem controle conectado: garante que está parado
  if (!controle) {

    mover('S');
  }

  delay(20);
}


// =====================================================
// CONEXÃO DO CONTROLE
// =====================================================

void aoConectarControle(ControllerPtr ctl) {

  if (controle == nullptr) {

    Serial.println("Controle conectado!");

    controle = ctl;

    saudacaoPendente = true;

    ultimaCor = -1;

    ultimaChecagemBateria = millis();

  } else {

    Serial.println("Ja existe um controle conectado.");
  }
}


void aoDesconectarControle(ControllerPtr ctl) {

  if (controle == ctl) {

    Serial.println("Controle desconectou! Parando.");

    controle = nullptr;

    saudacaoPendente = false;

    emergenciaAtiva = false;

    bloqueadoPorObstaculo = false;

    ultimaCor = -1;

    mover('S');
  }
}


// =====================================================
// SENSOR + MODELO
// =====================================================

// A cada 100 ms: mede, trata o sinal (igual ao treino) e
// pergunta ao modelo qual é a classe.
void atualizarSensor() {

  if (millis() - ultimaLeituraSensor < INTERVALO_SENSOR) {

    return;
  }

  ultimaLeituraSensor = millis();


  // Leitura crua (-1 = sem eco = nada por perto)
  float bruta = lerDistanciaCm();

  if (bruta < 0) {

    bruta = DIST_MAX;
  }


  // Limitação de faixa
  float limitada = constrain(bruta, DIST_MIN, DIST_MAX);


  // Média móvel
  leituras[posicao] = limitada;

  posicao = (posicao + 1) % JANELA;

  if (quantidade < JANELA) {

    quantidade++;
  }

  // Espera a janela encher
  if (quantidade < JANELA) {

    return;
  }

  float media = 0;

  for (int i = 0; i < JANELA; i++) {

    media += leituras[i];
  }

  distanciaMedia = media / JANELA;


  // Normalização (0 a 1): é isso que o modelo recebe
  features[0] = (distanciaMedia - DIST_MIN) / (DIST_MAX - DIST_MIN);


  int nova = preverClasse();

  // Mesma classe de antes: nada a confirmar
  if (nova == classeObstaculo) {

    classeCandidata = nova;

    confirmacoes = 0;

    return;
  }

  // Classe diferente: conta quantas leituras seguidas concordam
  if (nova != classeCandidata) {

    classeCandidata = nova;

    confirmacoes = 0;
  }

  confirmacoes++;

  int necessarias = CONFIRMACOES_NORMAL;

  if (nova == CLASSE_PERTO) {

    necessarias = 1;

  } else if (classeObstaculo == CLASSE_PERTO) {

    necessarias = CONFIRMACOES_LIBERAR;
  }

  if (confirmacoes >= necessarias) {

    classeObstaculo = nova;

    confirmacoes = 0;

    aoMudarClasse();
  }
}


// Roda o modelo e devolve a classe com maior probabilidade
int preverClasse() {

  signal_t sinal;

  sinal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
  sinal.get_data = &raw_feature_get_data;

  ei_impulse_result_t resultado = { 0 };

  EI_IMPULSE_ERROR erro = run_classifier(&sinal, &resultado, false);

  if (erro != EI_IMPULSE_OK) {

    Serial.print("ERRO no modelo: ");
    Serial.println(erro);

    return classeObstaculo;
  }

  int melhor = 0;

  for (int i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {

    if (resultado.classification[i].value >
        resultado.classification[melhor].value) {

      melhor = i;
    }
  }

  return melhor;
}


void aoMudarClasse() {

  Serial.print("Obstaculo: ");
  Serial.print(NOMES_CLASSES[classeObstaculo]);
  Serial.print(" (");
  Serial.print(distanciaMedia, 1);
  Serial.println(" cm)");

  bool indoParaFrente =
    movimentoAtual == 'F' ||
    movimentoAtual == 'L' ||
    movimentoAtual == 'R';

  // Aviso suave: chegou na faixa MEDIO andando para a frente
  // (a vibração forte do PERTO vem do bloqueio da frente)
  if (controle && indoParaFrente && classeObstaculo == CLASSE_MEDIO) {

    controle->playDualRumble(0, 120, 0x60, 0x00);
  }

  atualizarLuz();
}


// =====================================================
// LER O CONTROLE
// =====================================================

void lerControle(ControllerPtr c) {

  // Parada de emergência: botão X (✕)
  if (c->a()) {

    // Só na hora que apertou: luz vermelha e vibração
    if (!emergenciaAtiva) {

      emergenciaAtiva = true;

      mover('S');

      atualizarLuz();

      vibrar(c, 250);
    }

    mover('S');

    return;
  }

  // Soltou o botão X: volta a luz normal
  if (emergenciaAtiva) {

    emergenciaAtiva = false;

    atualizarLuz();
  }

  int x = c->axisX();

  uint8_t dpad = c->dpad();

  bool acelerando = c->throttle() > GATILHO_MINIMO;   // R2
  bool dandoRe = c->brake() > GATILHO_MINIMO;         // L2

  bool esq = x < -ZONA_MORTA || (dpad & DPAD_LEFT);
  bool dir = x > ZONA_MORTA || (dpad & DPAD_RIGHT);


  // O modelo diz PERTO e o motorista quer ir para a frente:
  // bloqueia (só na hora que bloqueia: luz vermelha e vibração)
  bool frenteBloqueada =
    acelerando && !dandoRe && classeObstaculo == CLASSE_PERTO;

  if (frenteBloqueada != bloqueadoPorObstaculo) {

    bloqueadoPorObstaculo = frenteBloqueada;

    if (frenteBloqueada) {

      Serial.println("FRENTE BLOQUEADA (obstaculo perto)");

      vibrar(c, 300);
    }

    atualizarLuz();
  }


  if (acelerando && dandoRe) {

    // Os dois juntos: freio
    mover('S');

  } else if (frenteBloqueada) {

    mover('S');

  } else if (acelerando) {

    if (esq) {

      mover('L');

    } else if (dir) {

      mover('R');

    } else {

      mover('F');
    }

  } else if (dandoRe) {

    // Em ré, virar para a esquerda leva a traseira para a
    // esquerda (como num carro de verdade).
    if (esq) {

      mover('E');

    } else if (dir) {

      mover('D');

    } else {

      mover('B');
    }

  } else {

    // Sem acelerar: parado, mesmo mexendo no analógico
    mover('S');
  }
}


// =====================================================
// MOVIMENTOS
// =====================================================

// Só muda quando o comando é diferente do atual
// (senão o Monitor Serial enche de mensagens repetidas).
void mover(char comando) {

  if (comando == movimentoAtual) {

    return;
  }

  switch (comando) {

    case 'F':

      frente();

      break;


    case 'B':

      re();

      break;


    case 'L':

      esquerda();

      break;


    case 'R':

      direita();

      break;


    case 'E':

      reEsquerda();

      break;


    case 'D':

      reDireita();

      break;


    case 'S':

      parar();

      break;
  }
}


void frente() {

  Serial.println("FRENTE");

  movimentoAtual = 'F';

  motorA(1);
  motorB(1);

  atualizarLuz();
}


void re() {

  Serial.println("RE");

  movimentoAtual = 'B';

  motorA(-1);
  motorB(-1);

  atualizarLuz();
}


void esquerda() {

  Serial.println("ESQUERDA");

  movimentoAtual = 'L';

  // Motor A parado, motor B para frente
  motorA(0);
  motorB(1);

  atualizarLuz();
}


void direita() {

  Serial.println("DIREITA");

  movimentoAtual = 'R';

  // Motor A para frente, motor B parado
  motorA(1);
  motorB(0);

  atualizarLuz();
}


void reEsquerda() {

  Serial.println("RE ESQUERDA");

  movimentoAtual = 'E';

  // Motor A parado, motor B para trás: a traseira vai para a esquerda
  motorA(0);
  motorB(-1);

  atualizarLuz();
}


void reDireita() {

  Serial.println("RE DIREITA");

  movimentoAtual = 'D';

  // Motor A para trás, motor B parado: a traseira vai para a direita
  motorA(-1);
  motorB(0);

  atualizarLuz();
}


void parar() {

  Serial.println("PARAR");

  movimentoAtual = 'S';

  motorA(0);
  motorB(0);

  atualizarLuz();
}


// =====================================================
// RESPOSTA NO CONTROLE (luz e vibração)
// =====================================================

// Manda a cor para a barra de luz, só se for diferente da última
void definirCor(uint8_t r, uint8_t g, uint8_t b) {

  if (!controle) {

    return;
  }

  long codigo = ((long)r << 16) | ((long)g << 8) | b;

  if (codigo == ultimaCor) {

    return;
  }

  ultimaCor = codigo;

  controle->setColorLED(r, g, b);
}


// Cor da barra de luz conforme o estado do carrinho
void atualizarLuz() {

  bool indoParaFrente =
    movimentoAtual == 'F' ||
    movimentoAtual == 'L' ||
    movimentoAtual == 'R';

  if (emergenciaAtiva || bloqueadoPorObstaculo) {

    definirCor(255, 0, 0);       // vermelho

  } else if (movimentoAtual == 'S') {

    definirCor(0, 255, 0);       // verde: parado

  } else if (!indoParaFrente) {

    definirCor(255, 160, 0);     // amarelo: ré

  } else if (classeObstaculo == CLASSE_MEDIO) {

    definirCor(160, 0, 255);     // roxo: obstáculo médio

  } else {

    definirCor(0, 0, 255);       // azul: andando
  }
}


// Vibra os dois motores do controle
void vibrar(ControllerPtr c, int duracaoMs) {

  c->playDualRumble(0, duracaoMs, 0xC0, 0xC0);
}


// Luz verde, primeiro LED de jogador e uma vibração curta
void saudarControle(ControllerPtr c) {

  atualizarLuz();

  c->setPlayerLEDs(1);

  vibrar(c, 200);

  int nivel = c->battery();

  Serial.print("Bateria do controle: ");

  if (nivel == 0) {

    Serial.println("desconhecida");

  } else {

    Serial.print(nivel * 100 / 255);
    Serial.println("%");
  }
}


// Bateria baixa: duas vibrações curtas, no máximo a cada 30 s
void checarBateria(ControllerPtr c) {

  if (millis() - ultimaChecagemBateria < INTERVALO_BATERIA) {

    return;
  }

  ultimaChecagemBateria = millis();

  int nivel = c->battery();

  if (nivel > 0 && nivel < BATERIA_BAIXA) {

    Serial.println("Bateria do controle baixa!");

    c->playDualRumble(0, 150, 0xC0, 0xC0);
    c->playDualRumble(300, 150, 0xC0, 0xC0);
  }
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


// =====================================================
// MOTORES
// =====================================================

// sentido: 1 = frente, -1 = trás, 0 = parado
void motorA(int sentido) {

  digitalWrite(IN1, sentido == 1 ? HIGH : LOW);
  digitalWrite(IN2, sentido == -1 ? HIGH : LOW);
}


void motorB(int sentido) {

  digitalWrite(IN3, sentido == 1 ? HIGH : LOW);
  digitalWrite(IN4, sentido == -1 ? HIGH : LOW);
}
