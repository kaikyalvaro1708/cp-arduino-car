// =====================================================
// CARRINHO - ESP32 + L298N + controle de PS4
//
// Usa a biblioteca Bluepad32. Na IDE, escolha a placa:
//   ESP32 + Bluepad32 Arduino > DOIT ESP32 DEVKIT V1
//
// Como parear (primeira vez):
//   Segure SHARE + botão PS até a luz do controle piscar.
//   Depois disso basta apertar o botão PS para reconectar.
//
// Controles (estilo jogo de corrida):
//   R2 = acelerar (frente)
//   L2 = ré
//   R2 + L2 juntos = freio (para)
//   Analógico esquerdo (ou D-pad esquerda / direita) = virar
//   Botão X (✕): parada de emergência
//
// Sem acelerar nem dar ré, o carrinho fica parado.
// Como o L298N está com os jumpers ENA/ENB, o gatilho liga ou
// desliga: não existe velocidade proporcional ao quanto aperta.
//
// O carrinho também "responde" no controle:
//   Luz: verde = parado, azul = andando, amarelo = ré,
//        vermelho = parada de emergência
//   Vibra ao conectar, na parada de emergência e quando
//   a bateria do controle está baixa
// =====================================================

#include <Bluepad32.h>


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

// A saudação (luz + vibração) só pode ser enviada depois que o
// controle mandou o primeiro dado, por isso fica pendente.
bool saudacaoPendente = false;


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

  Serial.begin(115200);

  BP32.setup(&aoConectarControle, &aoDesconectarControle);

  if (ESQUECER_PAREAMENTOS) {

    BP32.forgetBluetoothKeys();
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("     CARRINHO ESP32 INICIADO");
  Serial.println("================================");
  Serial.println("Segure SHARE + PS no controle para parear.");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  BP32.update();

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

    mover('S');
  }
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

  if (acelerando && dandoRe) {

    // Os dois juntos: freio
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

// Cor da barra de luz conforme o estado do carrinho
void atualizarLuz() {

  if (!controle) {

    return;
  }

  if (emergenciaAtiva) {

    controle->setColorLED(255, 0, 0);       // vermelho

  } else if (movimentoAtual == 'S') {

    controle->setColorLED(0, 255, 0);       // verde: parado

  } else if (
    movimentoAtual == 'B' ||
    movimentoAtual == 'E' ||
    movimentoAtual == 'D'
  ) {

    controle->setColorLED(255, 160, 0);     // amarelo: ré

  } else {

    controle->setColorLED(0, 0, 255);       // azul: andando
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
