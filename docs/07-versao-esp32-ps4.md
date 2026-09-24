# Versão 2: ESP32 + controle de PS4

[← Voltar ao README](../README.md)

A versão 2 do carrinho troca o Arduino Uno e o Bluetooth HC-05 por uma **ESP32**, controlada por um **controle de PS4**, e acrescenta um modelo de **TinyML** que classifica a distância do sensor ([veja o relatório](08-tinyml.md)).

## 1. Hardware

| Componente | Detalhe |
|---|---|
| Controlador | **ESP32-WROOM-32 DevKit V1** (30 pinos, chip USB CP2102) |
| Ponte H | L298N, com **jumpers ENA e ENB** (velocidade máxima, sem PWM) |
| Motores | 4 motores DC, dois de cada lado ligados em paralelo |
| Sensor | HC-SR04 (distância) |
| Controle | DualShock 4 (PS4), por Bluetooth, com a biblioteca Bluepad32 |
| Alimentação | pilhas só na ponte H (12V/GND); ESP32 pelo USB (um power bank está previsto) |

### Ligações

| Função | Pino da ESP32 | Liga em |
|---|---|---|
| Motores da esquerda | GPIO 25 | IN1 |
| | GPIO 26 | IN2 |
| Motores da direita | GPIO 27 | IN3 |
| | GPIO 14 | IN4 |
| Sensor, disparo | GPIO 18 | TRIG |
| Sensor, eco | GPIO 19 | ECHO, **por divisor de tensão** (1 kΩ + 2 kΩ) |
| Sensor, alimentação | 5V (VIN) e GND | VCC e GND |
| GND comum | GND | GND da ponte H, direto no borne |

Cada motor usa **dois bornes diferentes** do par: os motores da esquerda em OUT1 e OUT2, os da direita em OUT3 e OUT4, com um fio de cada motor em cada borne. Os 4 fios do lado esquerdo vão para os 2 bornes, dois em cada.

O GPIO 14 dá um pulso curto ao ligar ou resetar a ESP32, então os motores podem dar um tranco rápido nesse momento.

## 2. Controles do PS4 (`carrinho_esp32_ps4` e `carrinho_esp32_ps4_ia`)

Controles no estilo de jogo de corrida:

| Controle | Ação |
|---|---|
| **R2** | acelerar (frente) |
| **L2** | ré |
| **R2 + L2** juntos | freio |
| **Analógico esquerdo** (ou D-pad esquerda e direita) | virar |
| **✕** | parada de emergência |

Sem apertar nenhum gatilho, o carrinho fica parado. Como os jumpers ENA e ENB estão ligados, o gatilho **liga ou desliga**: não há velocidade proporcional ao quanto se aperta. Em ré, virar para um lado leva a traseira para esse lado, como num carro.

O carrinho **responde no controle** (a ESP32 manda dados para o controle):

| Situação | Luz do controle | Vibração |
|---|---|---|
| Conectou | verde | curta |
| Parado | verde | não |
| Andando | azul | não |
| Ré | amarela | não |
| Andando com obstáculo médio (versão `_ia`) | roxa | leve |
| Emergência (✕) ou obstáculo perto (versão `_ia`) | vermelha | forte |
| Bateria do controle baixa | não muda | duas vibrações |

Se o controle desconectar, o carrinho para sozinho.

## 3. Códigos (`code/`)

| Pasta (em `code/`) | Para que serve |
|---|---|
| `testes/teste_motores_esp32` | teste isolado da ponte H e dos motores |
| `carrinho_esp32_ps4` | carrinho controlado pelo PS4 |
| `carrinho_esp32_ps4_ia` | PS4 + sensor + modelo TinyML (versão final) |
| `carrinho_esp32_wifi` | alternativa: a ESP32 cria a rede `Carrinho-ESP32` e mostra uma página com botões em `http://192.168.4.1` (funciona no iPhone e no Android) |
| `carrinho_esp32` | alternativa: Bluetooth clássico com o app serial (só Android; o iPhone não enxerga) |
| `tinyml/tinyml_a_leitura_bruta`, `tinyml/tinyml_b_preprocessamento`, `tinyml/tinyml_c_modelo` | etapas do trabalho de TinyML |

### Como compilar

| Sketches | Placa na Arduino IDE | Pré-requisito |
|---|---|---|
| `teste_motores_esp32`, `carrinho_esp32`, `carrinho_esp32_wifi`, `tinyml_a_*`, `tinyml_b_*`, `tinyml_c_*` | **DOIT ESP32 DEVKIT V1** (pacote ESP32, versão 2.0.11) | `tinyml_c_*` precisa da biblioteca do modelo (zip em `dados/tinyml/`) |
| `carrinho_esp32_ps4`, `carrinho_esp32_ps4_ia` | **ESP32 + Bluepad32 Arduino > DOIT ESP32 DEVKIT V1** (pacote 4.1.0) | adicionar em *Preferências* a URL abaixo; a versão `_ia` também precisa da biblioteca do modelo |

```text
https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
```

No Windows, a ESP32 com chip CP2102 precisa do driver **CP210x** da Silicon Labs para aparecer como porta COM.

## 4. Testes, problemas e soluções

| Problema | Causa | Solução |
|---|---|---|
| Nenhuma porta COM da ESP32 (só COM4 a COM7, que eram Bluetooth virtuais) | faltava o driver do chip CP2102 (erro 28 no Gerenciador de Dispositivos) | instalar o driver CP210x; a placa apareceu como COM8 |
| Nenhum motor girava no teste | os **dois fios do mesmo motor** estavam no mesmo borne OUT, o que deixa o motor em curto | ligar cada motor em dois bornes diferentes do par |
| Só o lado esquerdo não girava | contato ruim no jumper ENA, nos fios IN1/IN2 ou nos bornes | reencaixar o jumper e os fios; funcionou |
| A ESP32 aparecia no PC mas não no iPhone | o iPhone não enxerga Bluetooth clássico | versão por Wi-Fi e, depois, o controle de PS4 |
| Gravar ou ler a serial dava "acesso negado" na COM8 | o Monitor Serial da IDE estava aberto | fechar o Monitor Serial |
| Leituras erradas do sensor com o objeto se movendo | eco do fundo, superfícies moles e sem eco | média móvel e, no carrinho, confirmação por leituras seguidas |
| No log do teste, a classe do modelo trocava entre PERTO e MEDIO perto de 22 cm | a fronteira do modelo fica nessa distância e o sensor varia uns 0,4 cm; parado ali, o motor ligaria e desligaria sem parar | histerese (sair de PERTO exige 5 leituras seguidas) |
| Gravação falhou duas vezes com "porta não existe" | conexão USB oscilou | tentar de novo; se repetir, trocar ou reencaixar o cabo |

Testes que passaram: motores (`teste_motores_esp32`), controle pelo Wi-Fi no iPhone, controle de PS4 (frente, ré, curvas, ✕), luz e vibração no controle, controles de corrida com R2 e L2, e o carrinho com o modelo (bloqueio da frente, ré livre, luzes e vibração).

## 5. Pendências

- Alimentar a ESP32 sem o cabo USB (power bank previsto).
- Carenagem, fotos e vídeo finais.
- Medir a distância de parada do carrinho no chão, já que o bloqueio começa a uns 22 cm.
- Velocidade proporcional ao gatilho (exige tirar os jumpers ENA e ENB e ligá-los a pinos com PWM).
