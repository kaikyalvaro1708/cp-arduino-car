// ===============================
// TESTE DO BUZZER
// Só Arduino + buzzer (+ no D2, - no GND).
// Abra o Monitor Serial em 9600.
// ===============================

const int BUZZER = 2;


void setup() {

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  Serial.begin(9600);

  Serial.println("TESTE DO BUZZER");
}


void loop() {

  // Buzzer ATIVO apita só de ligar o pino
  Serial.println("1) Pino ligado (buzzer ATIVO deve apitar)");

  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);

  delay(1000);


  // Buzzer PASSIVO precisa de uma frequência
  Serial.println("2) Tom de 1000 Hz (buzzer PASSIVO deve apitar)");

  tone(BUZZER, 1000);
  delay(1000);
  noTone(BUZZER);

  delay(2000);
}
