// =====================================================
// DABBLE (app no celular -> Gamepad)
// =====================================================

// Carrega só o módulo Gamepad (programa fica menor)
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <Dabble.h>

// HC-05: TX do módulo no D10, RX do módulo no D11
const int BT_RX = 10;
const int BT_TX = 11;


// =====================================================
// L298N
// =====================================================

// -----------------------------
// Motor A - Esquerda
// -----------------------------
const int IN1 = 8;
const int IN2 = 7;

// -----------------------------
// Motor B - Direita
// -----------------------------
const int IN3 = 6;
const int IN4 = 4;

// ENA e ENB ficam com jumper na ponte H (velocidade máxima)


// =====================================================
// HC-SR04
// =====================================================

const int TRIG = 12;
const int ECHO = 13;

const int DISTANCIA_SEGURA = 15;
const int DISTANCIA_LIBERAR = 20;


// =====================================================
// BUZZER
// =====================================================

const int PINO_BUZZER = 2;


// =====================================================
// ESTADO
// =====================================================

char movimentoAtual = 'S';

bool bloqueadoPorObstaculo = false;

// Último botão lido do Gamepad
// (só executa quando o botão muda)
char ultimoComando = 'S';


// =====================================================
// FILTRO DO SENSOR
// =====================================================

long leituras[3] = {
  999,
  999,
  999
};

int proximaLeitura = 0;

long distanciaAtual = 999;


// =====================================================
// TEMPOS
// =====================================================

unsigned long ultimaChecagemSensor = 0;

const unsigned long INTERVALO_SENSOR = 100;


// =====================================================
// SETUP
// =====================================================

void setup() {

  // -----------------------------
  // Motor A
  // -----------------------------

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);


  // -----------------------------
  // Motor B
  // -----------------------------

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);


  // -----------------------------
  // Sensor
  // -----------------------------

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);


  // -----------------------------
  // Buzzer
  // -----------------------------

  pinMode(PINO_BUZZER, OUTPUT);

  digitalWrite(PINO_BUZZER, LOW);


  // -----------------------------
  // Começa parado
  // -----------------------------

  parar();


  // -----------------------------
  // Serial USB
  // -----------------------------

  Serial.begin(9600);


  // -----------------------------
  // Dabble (Bluetooth)
  // -----------------------------

  Dabble.begin(9600, BT_RX, BT_TX);


  Serial.println("================================");
  Serial.println("     CARRINHO INICIADO");
  Serial.println("================================");

  Serial.println("Aguardando o app Dabble...");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // DABBLE
  // ===================================================

  // Obrigatório: atualiza os botões vindos do celular
  Dabble.processInput();


  char comando = lerGamePad();


  // Só age quando o botão muda
  // (segurar o botão não repete o comando)

  if (comando != ultimoComando) {

    ultimoComando = comando;

    Serial.print("Gamepad: ");
    Serial.println(comando);

    executarComando(comando);
  }


  // ===================================================
  // SENSOR
  // ===================================================

  if (millis() - ultimaChecagemSensor >= INTERVALO_SENSOR) {

    ultimaChecagemSensor = millis();

    checarObstaculo();
  }
}


// =====================================================
// LER GAMEPAD
// =====================================================

// Transforma o botão apertado no Dabble
// na mesma letra de comando de antes

char lerGamePad() {

  if (GamePad.isUpPressed()) {

    return 'F';
  }

  if (GamePad.isDownPressed()) {

    return 'B';
  }

  if (GamePad.isLeftPressed()) {

    return 'L';
  }

  if (GamePad.isRightPressed()) {

    return 'R';
  }


  // Nenhum botão apertado

  return 'S';
}


// =====================================================
// MEDIR DISTÂNCIA
// =====================================================

long medirDistancia() {

  digitalWrite(TRIG, LOW);

  delayMicroseconds(2);


  digitalWrite(TRIG, HIGH);

  delayMicroseconds(10);


  digitalWrite(TRIG, LOW);


  long duracao = pulseInLong(
    ECHO,
    HIGH,
    30000
  );


  // Nenhum retorno
  if (duracao == 0) {

    return 999;
  }


  long distancia = duracao * 0.0343 / 2;


  return distancia;
}


// =====================================================
// VERIFICAR OBSTÁCULO
// =====================================================

void checarObstaculo() {

  // -----------------------------
  // Nova leitura
  // -----------------------------

  leituras[proximaLeitura] = medirDistancia();


  // Próxima posição do array

  proximaLeitura =
    (proximaLeitura + 1) % 3;


  // -----------------------------
  // Calcula mediana
  // -----------------------------

  distanciaAtual = mediana(
    leituras[0],
    leituras[1],
    leituras[2]
  );


  // -----------------------------
  // Serial
  // -----------------------------

  Serial.print("Distancia: ");

  Serial.print(distanciaAtual);

  Serial.println(" cm");


  // ===================================================
  // OBSTÁCULO
  // ===================================================

  if (movimentoAtual == 'F') {

    if (distanciaAtual < DISTANCIA_SEGURA) {

      Serial.println("!!! OBSTACULO !!!");


      // Para o carrinho

      parar();


      // Bloqueia avanço

      bloqueadoPorObstaculo = true;


      // Buzzer

      alertaObstaculo();
    }
  }


  // ===================================================
  // LIBERAR CAMINHO
  // ===================================================

  if (bloqueadoPorObstaculo) {

    if (distanciaAtual >= DISTANCIA_LIBERAR) {

      bloqueadoPorObstaculo = false;


      Serial.println("Caminho liberado!");
    }
  }
}


// =====================================================
// MEDIANA
// =====================================================

long mediana(long a, long b, long c) {

  return max(
    min(a, b),
    min(max(a, b), c)
  );
}


// =====================================================
// BUZZER
// =====================================================

void alertaObstaculo() {

  digitalWrite(
    PINO_BUZZER,
    HIGH
  );


  delay(200);


  digitalWrite(
    PINO_BUZZER,
    LOW
  );
}


// =====================================================
// COMANDOS
// =====================================================

void executarComando(char comando) {

  switch (comando) {

    // =================================================
    // FRENTE
    // =================================================

    case 'F':
    case 'f':

      if (
        bloqueadoPorObstaculo ||
        distanciaAtual < DISTANCIA_SEGURA
      ) {

        Serial.println(
          "FRENTE BLOQUEADA!"
        );


        parar();


        alertaObstaculo();

      } else {

        frente();
      }

      break;


    // =================================================
    // RÉ
    // =================================================

    case 'B':
    case 'b':

      re();

      break;


    // =================================================
    // ESQUERDA
    // =================================================

    case 'L':
    case 'l':

      esquerda();

      break;


    // =================================================
    // DIREITA
    // =================================================

    case 'R':
    case 'r':

      direita();

      break;


    // =================================================
    // PARAR
    // =================================================

    case 'S':
    case 's':

      parar();

      break;
  }
}


// =====================================================
// FRENTE
// =====================================================

void frente() {

  Serial.println("FRENTE");


  movimentoAtual = 'F';


  // -----------------------------
  // Motor A
  // -----------------------------

  digitalWrite(
    IN1,
    HIGH
  );

  digitalWrite(
    IN2,
    LOW
  );


  // -----------------------------
  // Motor B
  // -----------------------------

  digitalWrite(
    IN3,
    HIGH
  );

  digitalWrite(
    IN4,
    LOW
  );
}


// =====================================================
// RÉ
// =====================================================

void re() {

  Serial.println("RE");


  movimentoAtual = 'B';


  // -----------------------------
  // Motor A
  // -----------------------------

  digitalWrite(
    IN1,
    LOW
  );

  digitalWrite(
    IN2,
    HIGH
  );


  // -----------------------------
  // Motor B
  // -----------------------------

  digitalWrite(
    IN3,
    LOW
  );

  digitalWrite(
    IN4,
    HIGH
  );
}


// =====================================================
// ESQUERDA
// =====================================================

void esquerda() {

  Serial.println("ESQUERDA");


  movimentoAtual = 'L';


  // -----------------------------
  // Motor A parado
  // -----------------------------

  digitalWrite(
    IN1,
    LOW
  );

  digitalWrite(
    IN2,
    LOW
  );


  // -----------------------------
  // Motor B para frente
  // -----------------------------

  digitalWrite(
    IN3,
    HIGH
  );

  digitalWrite(
    IN4,
    LOW
  );
}


// =====================================================
// DIREITA
// =====================================================

void direita() {

  Serial.println("DIREITA");


  movimentoAtual = 'R';


  // -----------------------------
  // Motor A para frente
  // -----------------------------

  digitalWrite(
    IN1,
    HIGH
  );

  digitalWrite(
    IN2,
    LOW
  );


  // -----------------------------
  // Motor B parado
  // -----------------------------

  digitalWrite(
    IN3,
    LOW
  );

  digitalWrite(
    IN4,
    LOW
  );
}


// =====================================================
// PARAR
// =====================================================

void parar() {

  Serial.println("PARAR");


  movimentoAtual = 'S';


  // -----------------------------
  // Motor A
  // -----------------------------

  digitalWrite(
    IN1,
    LOW
  );

  digitalWrite(
    IN2,
    LOW
  );


  // -----------------------------
  // Motor B
  // -----------------------------

  digitalWrite(
    IN3,
    LOW
  );

  digitalWrite(
    IN4,
    LOW
  );


  // -----------------------------
  // Buzzer
  // -----------------------------

  digitalWrite(
    PINO_BUZZER,
    LOW
  );
}