// =====================================================
// CARRINHO - ESP32 + L298N + Wi-Fi (página web)
//
// A ESP32 cria a própria rede Wi-Fi e mostra uma página
// com botões. Funciona em iPhone e Android, sem app.
//
// Como usar:
//   1. No celular, entre na rede Wi-Fi "Carrinho-ESP32"
//   2. Abra o navegador (Safari) em http://192.168.4.1
//   3. Segure os botões para andar. Soltou, parou.
// =====================================================

#include <WiFi.h>
#include <WebServer.h>


// =====================================================
// WI-FI
// =====================================================

const char* NOME_WIFI = "Carrinho-ESP32";
const char* SENHA_WIFI = "carrinho123";   // mínimo 8 caracteres

WebServer servidor(80);


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

// A página repete o comando enquanto o botão está apertado.
// Se parar de chegar comando (dedo soltou, Wi-Fi caiu, celular
// bloqueou), o carrinho para sozinho.
unsigned long ultimoComando = 0;

const unsigned long TEMPO_LIMITE = 500;


// =====================================================
// PÁGINA WEB
// =====================================================

const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>Carrinho</title>
<style>
  * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
  html, body { margin: 0; height: 100%; background: #111; color: #eee;
               font-family: -apple-system, system-ui, sans-serif;
               overscroll-behavior: none; }
  body { display: flex; flex-direction: column; align-items: center;
         justify-content: center; gap: 16px; }
  h1 { margin: 0; font-size: 22px; }
  p { margin: 0; font-size: 13px; color: #999; }
  .grade { display: grid; grid-template-columns: repeat(3, 1fr);
           gap: 12px; width: min(90vw, 420px); }
  button { aspect-ratio: 1; border: 0; border-radius: 20px;
           background: #2a6df4; color: #fff; font-size: 18px; font-weight: 600;
           touch-action: none; user-select: none; -webkit-user-select: none;
           -webkit-touch-callout: none; }
  button.ativo { background: #8fb0ff; }
  button.parar { background: #d33; }
  .vazio { visibility: hidden; }
</style>
</head>
<body>
  <h1>Carrinho</h1>
  <div class="grade">
    <button class="vazio"></button>
    <button data-c="F">FRENTE</button>
    <button class="vazio"></button>

    <button data-c="L">ESQ.</button>
    <button data-c="S" class="parar">PARAR</button>
    <button data-c="R">DIR.</button>

    <button class="vazio"></button>
    <button data-c="B">RÉ</button>
    <button class="vazio"></button>
  </div>
  <p>Segure o botão para andar. Soltou, parou.</p>

<script>
  var intervalo = null;

  function enviar(c) {
    fetch('/cmd?c=' + c, { cache: 'no-store' }).catch(function () {});
  }

  function segurar(c) {
    soltar(false);
    enviar(c);
    intervalo = setInterval(function () { enviar(c); }, 150);
  }

  function soltar(mandarParar) {
    if (intervalo) { clearInterval(intervalo); intervalo = null; }
    if (mandarParar) { enviar('S'); }
  }

  document.querySelectorAll('button[data-c]').forEach(function (b) {
    var c = b.dataset.c;

    if (c === 'S') {
      b.addEventListener('pointerdown', function () { soltar(false); enviar('S'); });
      return;
    }

    function fim() { b.classList.remove('ativo'); soltar(true); }

    b.addEventListener('pointerdown', function (e) {
      b.setPointerCapture(e.pointerId);
      b.classList.add('ativo');
      segurar(c);
    });
    b.addEventListener('pointerup', fim);
    b.addEventListener('pointercancel', fim);
  });

  document.addEventListener('contextmenu', function (e) { e.preventDefault(); });
  document.addEventListener('visibilitychange', function () {
    if (document.hidden) { soltar(true); }
  });
</script>
</body>
</html>
)rawliteral";


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

  WiFi.mode(WIFI_AP);
  WiFi.softAP(NOME_WIFI, SENHA_WIFI);

  servidor.on("/", paginaInicial);
  servidor.on("/cmd", receberComando);
  servidor.begin();

  Serial.println();
  Serial.println("================================");
  Serial.println("     CARRINHO ESP32 INICIADO");
  Serial.println("================================");

  Serial.print("Wi-Fi: ");
  Serial.println(NOME_WIFI);

  Serial.print("Senha: ");
  Serial.println(SENHA_WIFI);

  Serial.print("Abra no navegador: http://");
  Serial.println(WiFi.softAPIP());
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  servidor.handleClient();

  // Parada automática: sem comando há muito tempo
  if (
    movimentoAtual != 'S' &&
    millis() - ultimoComando > TEMPO_LIMITE
  ) {

    Serial.println("Sem comando! Parando.");

    parar();
  }
}


// =====================================================
// SERVIDOR
// =====================================================

void paginaInicial() {

  servidor.send_P(200, "text/html", PAGINA);
}


void receberComando() {

  if (servidor.hasArg("c") && servidor.arg("c").length() > 0) {

    executarComando(servidor.arg("c")[0]);
  }

  servidor.send(204, "text/plain", "");
}


// =====================================================
// COMANDOS
// =====================================================

void executarComando(char comando) {

  comando = toupper(comando);

  ultimoComando = millis();

  // A página repete o mesmo comando: só renova o tempo
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


    case 'S':

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
