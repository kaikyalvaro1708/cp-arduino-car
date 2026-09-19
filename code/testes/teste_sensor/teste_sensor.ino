#include <SoftwareSerial.h>

// ===============================
// TESTE DO SENSOR (HC-SR04)
// Arduino + ponte H + motores + HC-05 + HC-SR04.
// Só mostra as leituras: NÃO para em obstáculo.
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

// HC-SR04
const int TRIG = 12;
const int ECHO = 13;

const int DISTANCIA_SEGURA = 15;

unsigned long ultimaLeitura = 0;
const unsigned long INTERVALO_SENSOR = 100;


// ===============================
// SETUP
// ===============================

void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  motorA(0);
  motorB(0);

  Serial.begin(9600);

  bluetooth.begin(9600);

  Serial.println("TESTE DO SENSOR");
  Serial.println("Deixe as rodas no ar.");
}


// ===============================
// LOOP
// ===============================

void loop() {

  if (bluetooth.available()) {

    char comando = bluetooth.read();

    if (comando != '\r' && comando != '\n') {

      Serial.print(">>> Recebido: ");
      Serial.print(comando);
      Serial.print(" -> ");

      executarComando(comando);
    }
  }


  if (millis() - ultimaLeitura >= INTERVALO_SENSOR) {

    ultimaLeitura = millis();

    long distancia = medirDistancia();

    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.print(" cm");

    if (distancia == 999) {
      Serial.println("  (sem eco)");
    } else if (distancia < DISTANCIA_SEGURA) {
      Serial.println("  <-- PERTO");
    } else {
      Serial.println();
    }
  }
}


// ===============================
// MEDIR DISTÂNCIA
// ===============================

long medirDistancia() {

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duracao = pulseInLong(ECHO, HIGH, 30000);

  if (duracao == 0) {
    return 999;
  }

  return duracao * 0.0343 / 2;
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
