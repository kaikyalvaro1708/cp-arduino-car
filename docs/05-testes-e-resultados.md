# Testes e resultados

[← Voltar ao README](../README.md)

Resumo dos testes feitos, problemas encontrados e correções. Os códigos de teste estão em [`code/testes/`](../code/testes/).

| Teste | Data | Resultado |
|---|---|---|
| [T0 — Protótipo ESP32 + Wi-Fi](#t0--protótipo-esp32--wi-fi) | 21/08/2026 | ✅ motores e controle pelo navegador funcionando na bancada |
| [T1 — Motores falhando de forma intermitente](#t1--motores-falhando-de-forma-intermitente) | 18/09/2026 | ✅ corrigido (mau contato no GND) |
| [T2 — Ponte H e motores isolados](#t2--ponte-h-e-motores-isolados) | 18/09/2026 | ✅ |
| [T3 — Bluetooth HC-05 isolado](#t3--bluetooth-hc-05-isolado) | 18/09/2026 | ✅ 6 de 6 comandos |
| [T4 — Sensor HC-SR04 isolado](#t4--sensor-hc-sr04-isolado) | 18/09/2026 | ✅ corrigido com filtro de mediana |
| [T5 — Integração completa](#t5--integração-completa) | 18/09/2026 | ✅ |
| [T6 — Funcionamento só com bateria](#t6--funcionamento-só-com-bateria) | 18/09/2026 | ✅ |
| [T7 — Desvio lateral e controle de velocidade](#t7--desvio-lateral-e-controle-de-velocidade) | 18/09/2026 | ⚠️ PWM descartado, pequeno desvio aceito |
| [T8 — Versão com app Dabble](#t8--versão-com-app-dabble) | 18/09/2026 | ✅ criada como alternativa |
| [T9 — Buzzer](#t9--buzzer) | 18/09/2026 | ✅ funcionando |

---

## T0 — Protótipo ESP32 + Wi-Fi

**Objetivo:** validar motores, ponte H e controle remoto antes da montagem no chassi.

**Montagem:** ESP32 + L298N + 2 motores + 2 baterias 18650, controle por uma página web servida pelo próprio ESP32 (botões de direção, velocidade por PWM e distância do sensor). Código: [`code/carrinho_wifi_esp32`](../code/carrinho_wifi_esp32/carrinho_wifi_esp32.ino).

**Resultado:** motores respondendo aos comandos da página. Registros em [`image/`](../image/) e [`video/`](../video/).

<img src="../image/image01.jpg" width="320" alt="Protótipo ESP32 na bancada">

---

## T1 — Motores falhando de forma intermitente

**Sintoma:** na montagem com Arduino Uno, às vezes os dois motores funcionavam e às vezes não. Um dos motores fazia um barulho leve mas não girava; quando girava, era fraco.

**Investigação:**

| Passo | O que foi feito | Conclusão |
|---|---|---|
| 1 | Revisão do código | `frente()` e `re()` sempre acionam os 4 pinos juntos — nenhuma lógica desliga um motor só |
| 2 | Leitura do Monitor Serial com o carrinho parado | Arduino funcionando, sensor estável (132–135 cm) |
| 3 | Motor ligado direto numa bateria 9 V | Girou forte → **o motor está bom** |
| 4 | Arduino alimentado pelo USB em vez da bateria 9 V | Funcionou — a suspeita passou a ser a bateria 9 V, mas a causa real apareceu no passo 5 |
| 5 | **Pressionar a protoboard** | **O motor voltou a funcionar** → mau contato |

**Causa:** a protoboard usada para o 5 V e o GND estava **sem o fundo adesivo**, com as lâminas de metal soltas. O **GND comum** entre Arduino e ponte H passava por ela; com a vibração, o contato ia e voltava. Sem referência de GND, os sinais IN1–IN4 ficavam instáveis.

**Correção:**
- Fio de GND **direto do Arduino até o borne GND da ponte H** (preso junto com o negativo das pilhas), sem passar pela protoboard.
- Fita isolante no fundo da protoboard (as lâminas expostas podiam causar curto no chassi).

**Resultado:** motores funcionando. Ainda houve **1 falha em 5 tentativas**, então a equipe passou a **testar cada parte isolada** (T2 a T5).

---

## T2 — Ponte H e motores isolados

**Código:** [`teste_motores`](../code/testes/teste_motores/teste_motores.ino) — só Arduino + ponte H + motores, sem Bluetooth e sem sensor. Liga motor A, motor B e os dois juntos, para frente e para trás, 3 s cada.

**Resultado:** ✅ "funcionou perfeitamente" — os dois motores giram forte sozinhos e juntos, nos dois sentidos.

---

## T3 — Bluetooth HC-05 isolado

**Código:** [`teste_bluetooth`](../code/testes/teste_bluetooth/teste_bluetooth.ino) — mostra cada comando recebido com número de sequência e código ASCII.

**Resultado:** ✅ todos os comandos chegaram, sem caracteres estranhos:

```
#1 Recebido: F (codigo 70) -> FRENTE
#2 Recebido: B (codigo 66) -> RE
#3 Recebido: L (codigo 76) -> ESQUERDA
#4 Recebido: R (codigo 82) -> DIREITA
#5 Recebido: L (codigo 76) -> ESQUERDA
#6 Recebido: S (codigo 83) -> PARAR
```

**Observação:** o app manda **uma letra por toque** (não repete). Por isso, se um comando se perde, o carrinho ignora aquele toque até o próximo — isso explicava a falha ocasional do T1.

---

## T4 — Sensor HC-SR04 isolado

**Código:** [`teste_sensor`](../code/testes/teste_sensor/teste_sensor.ino) — mostra a leitura bruta a cada 100 ms, com motores controlados pelo app.

**Testes:** (1) motores parados, mão se aproximando; (2) motores girando, nada na frente; (3) vários comandos seguidos.

**Resultados:**
- ✅ Parede a ~135 cm lida de forma estável; a mão é detectada de 2 a 15 cm.
- ✅ Com os motores girando, o ruído **não** gerou leituras "perto" falsas — ele aparece como leituras **longe** (96, 163–174, 440, 999 cm).
- ✅ Comandos do Bluetooth não alteram as leituras.
- ❌ **Leituras isoladas erradas:**
  - um "5 cm" ou "6 cm" sozinho no meio de leituras longe (`170 → 5 → 441`) — no código antigo isso **parava o carrinho sem motivo**;
  - um "57" ou "138" sozinho no meio da mão parada na frente — no código antigo isso **liberava o carrinho com o obstáculo ainda ali**.

**Correção:** usar a **mediana das 3 últimas leituras** (uma leitura errada sozinha nunca vence as outras duas). Também foi trocado `pulseIn()` por `pulseInLong()` e passou a ser feita uma única medição por ciclo.

Simulação do filtro com trechos reais do log:

| Trecho do log | Leituras brutas | Com a mediana |
|---|---|---|
| Nada na frente, motores ligados | `134, 96, 135, 96, 138, 134, 443, 440, 443...` | nenhuma < 15 cm ✅ |
| Logo após tirar a mão | `... 170, 5, 441, 441, 173, 166, 161, 6, 135` | os "5" e "6" isolados somem ✅ |
| Mão parada na frente | `... 4, 9, 5, 57, 7, 5, 5, 3, 4, 4, 6, 138, 5, 6` | continua detectando a mão ✅ |

**Limitação conhecida:** muito colado (< ~4 cm) ou com superfície inclinada, o eco não volta e o sensor retorna 999. Andando em direção a uma parede, o carrinho para nos 15 cm, antes disso.

---

## T5 — Integração completa

**Código:** [`arduino_carrinho_codigo`](../code/arduino_carrinho_codigo/arduino_carrinho_codigo.ino) (versão final), rodas no ar e depois no chão.

**Resultados:** ✅ distância estável (128–131 cm) sem os saltos do T4. Trecho do log do teste de obstáculo:

```
Recebido: F
FRENTE
Distancia: 129 cm
Distancia: 55 cm
Distancia: 51 cm
Distancia: 4 cm
OBSTACULO!
PARAR
Distancia: 15 cm
Distancia: 113 cm
Caminho liberado!
```

Frente bloqueada (mão parada a 3–7 cm, comando F enviado 3 vezes — nas 3 o carrinho recusou):

```
Distancia: 7 cm
Recebido: F
FRENTE BLOQUEADA!
PARAR
```

---

## T6 — Funcionamento só com bateria

Sem o cabo USB: Arduino na bateria 9 V e motores nas 4 pilhas AA.

**Resultado:** ✅ tudo funcionou (movimentos, Bluetooth, sensor e parada por obstáculo).

---

## T7 — Desvio lateral e controle de velocidade

**Sintoma:** a roda esquerda girava um pouco mais rápido e o carrinho puxava levemente para um lado.

**Tentativas:**

| # | O que foi feito | Resultado |
|---|---|---|
| 1 | Jumper do ENA retirado; ENA no D9 com PWM (230, depois 200) | Andou mais reto, mas a roda esquerda **arrancava atrasada** |
| 2 | "Tranco" de 150 ms em força total na partida | Implementado para as rodas saírem juntas |
| 3 | PWM nos dois motores (ENA no D9, ENB no D5), velocidades 140 e 200 | Ainda puxava para a direita; depois, **na ré só uma roda girava** |
| 4 | Jumpers de volta, código sem PWM | Movimentos confiáveis; pequeno desvio aceito |

**Conclusão:** com 4 pilhas AA (6 V) e a queda de tensão do L298N (~2 V), reduzir o PWM deixa os motores sem força para arrancar. A equipe preferiu confiabilidade à correção fina da trajetória. Melhorias futuras: alimentação de maior tensão (2 × 18650 ou 6 × AA) ou um driver com menor queda de tensão (TB6612FNG).

⚠️ **Cuidado registrado:** com o jumper de volta no ENA/ENB, o fio do pino do Arduino **precisa ser retirado** — senão o pino fica ligado direto no 5 V da ponte H.

---

## T8 — Versão com app Dabble

Criada a versão [`carrinho_dabble`](../code/carrinho_dabble/carrinho_dabble.ino) usando o módulo **Gamepad** do app Dabble (segurar a seta = andar, soltar = parar), sem mudar a fiação.

**Problema encontrado:** a biblioteca Dabble já define um nome `BUZZER` (pino da placa evive), o que dava erro de compilação. **Correção:** a constante foi renomeada para `PINO_BUZZER` nessa versão.

**Resultado:** compila e fica como alternativa; a equipe manteve o **app serial** como controle principal.

---

## T9 — Buzzer

**Sintoma:** o buzzer não apitou nos testes de obstáculo.

**Investigação:** criado o [`teste_buzzer`](../code/testes/teste_buzzer/teste_buzzer.ino), que testa separadamente:
1. pino ligado por 1 s (apita se o buzzer for **ativo**);
2. tom de 1000 Hz por 1 s (apita se o buzzer for **passivo**).

| Apita em | Diagnóstico |
|---|---|
| 1 e 2 | buzzer ativo, ligação correta |
| só no 2 | buzzer passivo → usar `tone()` no código |
| nenhum | ligação: + no D2, − no GND, polaridade (perna longa no +) |

**Resultado:** ✅ buzzer funcionando.

---

## Resultado final

| Requisito | Situação |
|---|---|
| Frente, ré e curvas | ✅ |
| Controle sem fio (Bluetooth) | ✅ |
| Parada automática por obstáculo | ✅ |
| Bloqueio de avanço com obstáculo | ✅ |
| Alimentação por bateria | ✅ |
| Estabilidade (sem falhas intermitentes) | ✅ após T1 e T4 |
| Alerta sonoro | ✅ |
| Trajetória reta | ⚠️ pequeno desvio (T7) |
