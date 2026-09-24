// =====================================================
// TESTE DA PONTE H (L298N) - ESP32
// ESP32 + L298N + 4 motores
//
// ENA e ENB: jumpers instalados na L298N
// Monitor Serial: 115200
// =====================================================


// =====================================================
// PINOS
// =====================================================

// Motor A - lado esquerdo
// OUT1 / OUT2
const int IN1 = 25;
const int IN2 = 26;

// Motor B - lado direito
// OUT3 / OUT4
const int IN3 = 27;
const int IN4 = 14;


// =====================================================
// TEMPOS
// =====================================================

const unsigned long TEMPO_LIGADO = 3000;  // 3 segundos
const unsigned long TEMPO_PARADO = 1000; // 1 segundo


// =====================================================
// SETUP
// =====================================================

void setup() {

  // Configura os pinos da L298N
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Garante que os motores comecem desligados
  pararTodos();

  // Inicializa comunicação serial
  Serial.begin(115200);

  Serial.println();
  Serial.println("====================================");
  Serial.println(" TESTE DA PONTE H - ESP32 + L298N");
  Serial.println("====================================");

  Serial.println("Motores iniciando em 5 segundos...");
  Serial.println("Mantenha as rodas suspensas!");

  delay(5000);
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // 1. Motor A para frente
  etapa(
    "1) Motor A (ESQUERDA) -> FRENTE",
    1,
    0
  );

  // 2. Motor A para trás
  etapa(
    "2) Motor A (ESQUERDA) -> TRAS",
    -1,
    0
  );

  // 3. Motor B para frente
  etapa(
    "3) Motor B (DIREITA) -> FRENTE",
    0,
    1
  );

  // 4. Motor B para trás
  etapa(
    "4) Motor B (DIREITA) -> TRAS",
    0,
    -1
  );

  // 5. Ambos para frente
  etapa(
    "5) OS DOIS LADOS -> FRENTE",
    1,
    1
  );

  // 6. Ambos para trás
  etapa(
    "6) OS DOIS LADOS -> TRAS",
    -1,
    -1
  );

  // Final do ciclo
  pararTodos();

  Serial.println();
  Serial.println("------------------------------------");
  Serial.println("Fim do ciclo. Repetindo...");
  Serial.println("------------------------------------");

  delay(2000);
}


// =====================================================
// ETAPA
// =====================================================
//
// sentido:
//   1  = frente
//  -1  = trás
//   0  = parado
//

void etapa(const char* mensagem, int sentidoA, int sentidoB) {

  Serial.println();
  Serial.println(mensagem);

  // Liga os motores
  motorA(sentidoA);
  motorB(sentidoB);

  // Mantém ligados
  delay(TEMPO_LIGADO);

  // Para
  pararTodos();

  // Aguarda antes da próxima etapa
  delay(TEMPO_PARADO);
}


// =====================================================
// MOTOR A - ESQUERDA
// =====================================================

void motorA(int sentido) {

  if (sentido == 1) {

    // Frente
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

  } else if (sentido == -1) {

    // Trás
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

  } else {

    // Parado
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}


// =====================================================
// MOTOR B - DIREITA
// =====================================================

void motorB(int sentido) {

  if (sentido == 1) {

    // Frente
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

  } else if (sentido == -1) {

    // Trás
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

  } else {

    // Parado
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}


// =====================================================
// PARAR TODOS OS MOTORES
// =====================================================

void pararTodos() {

  motorA(0);
  motorB(0);
}