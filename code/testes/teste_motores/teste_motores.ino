// ===============================
// TESTE DA PONTE H (L298N)
// Só Arduino + ponte H + motores.
// Abra o Monitor Serial em 9600.
// ===============================

// Motor A (OUT1/OUT2)
const int IN1 = 8;
const int IN2 = 7;

// Motor B (OUT3/OUT4)
const int IN3 = 6;
const int IN4 = 4;

const unsigned long TEMPO_LIGADO = 3000;
const unsigned long TEMPO_PARADO = 1000;


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

  Serial.println("TESTE DA PONTE H");
  Serial.println("Levante as rodas! Comecando em 5 segundos...");

  delay(5000);
}


// ===============================
// LOOP
// ===============================

void loop() {

  etapa("1) Motor A para FRENTE", 1, 0);
  etapa("2) Motor A para TRAS", -1, 0);

  etapa("3) Motor B para FRENTE", 0, 1);
  etapa("4) Motor B para TRAS", 0, -1);

  etapa("5) Os DOIS para FRENTE", 1, 1);
  etapa("6) Os DOIS para TRAS", -1, -1);

  Serial.println("--- fim do ciclo, repetindo ---");
  Serial.println();

  delay(2000);
}


// ===============================
// ETAPA
// ===============================

// sentido: 1 = frente, -1 = trás, 0 = parado
void etapa(const char* texto, int sentidoA, int sentidoB) {

  Serial.println(texto);

  motorA(sentidoA);
  motorB(sentidoB);

  delay(TEMPO_LIGADO);

  motorA(0);
  motorB(0);

  delay(TEMPO_PARADO);
}


// ===============================
// MOTORES
// ===============================

void motorA(int sentido) {

  digitalWrite(IN1, sentido == 1 ? HIGH : LOW);
  digitalWrite(IN2, sentido == -1 ? HIGH : LOW);
}


void motorB(int sentido) {

  digitalWrite(IN3, sentido == 1 ? HIGH : LOW);
  digitalWrite(IN4, sentido == -1 ? HIGH : LOW);
}
