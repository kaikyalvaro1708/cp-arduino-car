#include <SoftwareSerial.h>

// ===============================
// TESTE DO BLUETOOTH (HC-05)
// Arduino + ponte H + motores + HC-05.
// Abra o Monitor Serial em 9600.
// ===============================

const int BT_RX = 10;
const int BT_TX = 11;

SoftwareSerial bluetooth(BT_RX, BT_TX);

// Motor A (OUT1/OUT2)
const int IN1 = 8;
const int IN2 = 7;

// Motor B (OUT3/OUT4)
const int IN3 = 6;
const int IN4 = 4;

unsigned long totalRecebidos = 0;


// ===============================
// SETUP
// ===============================

void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  motorA(0);
  motorB(0);

  Serial.begin(9600);

  bluetooth.begin(9600);

  Serial.println("TESTE DO BLUETOOTH");
  Serial.println("Conecte pelo app e aperte os botoes.");
}


// ===============================
// LOOP
// ===============================

void loop() {

  if (bluetooth.available()) {

    char comando = bluetooth.read();

    if (comando == '\r' || comando == '\n') {
      return;
    }

    totalRecebidos++;

    Serial.print("#");
    Serial.print(totalRecebidos);
    Serial.print(" Recebido: ");
    Serial.print(comando);
    Serial.print(" (codigo ");
    Serial.print((byte) comando);
    Serial.print(") -> ");

    executarComando(comando);
  }
}


// ===============================
// COMANDOS
// ===============================

void executarComando(char comando) {

  switch (comando) {

    case 'F':
    case 'f':
      Serial.println("FRENTE");
      motorA(1);
      motorB(1);
      break;

    case 'B':
    case 'b':
      Serial.println("RE");
      motorA(-1);
      motorB(-1);
      break;

    case 'L':
    case 'l':
      Serial.println("ESQUERDA");
      motorA(0);
      motorB(1);
      break;

    case 'R':
    case 'r':
      Serial.println("DIREITA");
      motorA(1);
      motorB(0);
      break;

    case 'S':
    case 's':
      Serial.println("PARAR");
      motorA(0);
      motorB(0);
      break;

    default:
      Serial.println("comando desconhecido");
      break;
  }
}


// ===============================
// MOTORES
// ===============================

// sentido: 1 = frente, -1 = trás, 0 = parado
void motorA(int sentido) {

  digitalWrite(IN1, sentido == 1 ? HIGH : LOW);
  digitalWrite(IN2, sentido == -1 ? HIGH : LOW);
}


void motorB(int sentido) {

  digitalWrite(IN3, sentido == 1 ? HIGH : LOW);
  digitalWrite(IN4, sentido == -1 ? HIGH : LOW);
}
