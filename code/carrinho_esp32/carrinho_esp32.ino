// =====================================================
// CARRINHO - ESP32 + L298N + Bluetooth clássico
//
// Usa o Bluetooth da própria ESP32 (sem HC-05).
// No celular (Android): pareie com "Carrinho-ESP32" e
// use o mesmo app serial de antes (F/B/L/R/S).
// Só funciona em ESP32 normal (ESP32-WROOM-32).
// =====================================================

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error "Esta placa nao tem Bluetooth classico. Escolha 'DOIT ESP32 DEVKIT V1' em Ferramentas > Placa."
#endif

BluetoothSerial bluetooth;

const char* NOME_BLUETOOTH = "Carrinho-ESP32";


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
// ESTADO
// =====================================================

char movimentoAtual = 'S';

bool estavaConectado = false;


// =====================================================
// SETUP
// =====================================================

void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Começa parado
  parar();

  Serial.begin(115200);

  bluetooth.begin(NOME_BLUETOOTH);

  Serial.println();
  Serial.println("================================");
  Serial.println("     CARRINHO ESP32 INICIADO");
  Serial.println("================================");

  Serial.print("Bluetooth: ");
  Serial.println(NOME_BLUETOOTH);
  Serial.println("Aguardando conexao...");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // CONEXÃO
  // Se o celular desconectar, o carrinho para sozinho.
  // (Não uso "tempo sem comando" porque o app manda uma
  // letra por toque, e isso pararia o carrinho no meio.)
  // ===================================================

  bool conectado = bluetooth.hasClient();

  if (conectado && !estavaConectado) {

    Serial.println("Celular conectado!");
  }

  if (!conectado && estavaConectado) {

    Serial.println("Celular desconectou! Parando.");

    parar();
  }

  estavaConectado = conectado;


  // ===================================================
  // BLUETOOTH
  // ===================================================

  if (bluetooth.available()) {

    char comando = bluetooth.read();

    // Ignora o fim de linha (\r e \n) que o app manda
    if (comando != '\r' && comando != '\n') {

      Serial.print("Recebido: ");
      Serial.println(comando);

      executarComando(comando);
    }
  }
}


// =====================================================
// COMANDOS BLUETOOTH
// =====================================================

void executarComando(char comando) {

  switch (comando) {

    case 'F':
    case 'f':

      frente();

      break;


    case 'B':
    case 'b':

      re();

      break;


    case 'L':
    case 'l':

      esquerda();

      break;


    case 'R':
    case 'r':

      direita();

      break;


    case 'S':
    case 's':

      parar();

      break;
  }
}


// =====================================================
// MOVIMENTOS
// =====================================================

void frente() {

  Serial.println("FRENTE");

  movimentoAtual = 'F';

  motorA(1);
  motorB(1);
}


void re() {

  Serial.println("RE");

  movimentoAtual = 'B';

  motorA(-1);
  motorB(-1);
}


void esquerda() {

  Serial.println("ESQUERDA");

  movimentoAtual = 'L';

  // Motor A parado, motor B para frente
  motorA(0);
  motorB(1);
}


void direita() {

  Serial.println("DIREITA");

  movimentoAtual = 'R';

  // Motor A para frente, motor B parado
  motorA(1);
  motorB(0);
}


void parar() {

  Serial.println("PARAR");

  movimentoAtual = 'S';

  motorA(0);
  motorB(0);
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
