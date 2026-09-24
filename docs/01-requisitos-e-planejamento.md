# Requisitos, planejamento e evolução

[← Voltar ao README](../README.md)

## 1. Requisitos

### Requisitos funcionais

| ID | Requisito | Status |
|---|---|---|
| RF01 | Andar para frente e para trás | ✅ |
| RF02 | Fazer curvas para a esquerda e para a direita | ✅ |
| RF03 | Ser controlado sem fio pelo celular | ✅ Bluetooth (HC-05) |
| RF04 | Detectar obstáculos à frente com um sensor | ✅ HC-SR04 |
| RF05 | Parar sozinho antes de colidir (limite de 15 cm) | ✅ |
| RF06 | Recusar o comando "frente" enquanto houver obstáculo | ✅ |
| RF07 | Emitir alerta sonoro ao detectar obstáculo | ✅ ([T9](05-testes-e-resultados.md#t9--buzzer)) |
| RF08 | Enviar a distância medida para o celular | ✅ |

### Requisitos não funcionais

| ID | Requisito | Status |
|---|---|---|
| RNF01 | Funcionar só com bateria, sem cabo USB | ✅ |
| RNF02 | Alimentação dos motores separada da lógica, com GND comum | ✅ |
| RNF03 | Não parar por leituras falsas do sensor | ✅ filtro de mediana |
| RNF04 | Funcionar de forma estável, sem falhas intermitentes | ✅ após correções |
| RNF05 | Carenagem com acesso aos componentes | PREENCHER |
| RNF06 | Código organizado e comentado | ✅ |

### Materiais

A lista de materiais, as opções de compra e a estimativa de custo foram levantadas na **Aula 13 (14/08/2026)**: [Lista de materiais](lista-de-materiais-aula13.md).

## 2. Proposta inicial e esboço

Carrinho de **duas rodas motrizes traseiras** e roda boba dianteira, com sensor ultrassônico na frente e controle pelo celular. O croqui do chassi (150 × 210 mm) define a posição de cada componente:

![Croqui do chassi](../image/croqui-chassi.png)

As justificativas do layout estão em [Projeto mecânico](02-projeto-mecanico.md#decisões-de-projeto-do-layout).

<!-- PREENCHER (opcional): fotos de outros esboços feitos em aula -->

## 3. MVP (produto mínimo viável)

1. Carrinho se move para frente, para trás e faz curvas.
2. Controle remoto sem fio pelo celular.
3. Sensor ultrassônico que **para o carrinho antes de bater**.
4. Tudo alimentado por bateria.

Além do MVP: alerta sonoro, envio da distância para o app, filtro das leituras do sensor, carenagem.

## 4. Backlog

| # | Item | Prioridade | Status |
|---|---|---|---|
| 1 | Levantar requisitos, materiais e custos | Alta | ✅ 14/08 |
| 2 | Croqui do chassi | Alta | ✅ 14/08 |
| 3 | Protótipo de bancada: motores + ponte H + controle remoto | Alta | ✅ 21/08 (ESP32 + Wi-Fi) |
| 4 | Fabricar/montar o chassi | Alta | ✅ (ver [mecânica](02-projeto-mecanico.md)) |
| 5 | Montagem final com Arduino Uno + HC-05 | Alta | ✅ |
| 6 | Programar movimentos e comandos Bluetooth | Alta | ✅ |
| 7 | Parada automática por obstáculo | Alta | ✅ |
| 8 | Alimentação por bateria | Alta | ✅ |
| 9 | Corrigir falhas intermitentes dos motores | Alta | ✅ |
| 10 | Filtrar leituras falsas do sensor | Média | ✅ |
| 11 | Buzzer de alerta | Média | ✅ |
| 12 | Enviar distância para o app | Baixa | ✅ |
| 13 | Compensar diferença de velocidade entre as rodas | Baixa | ❌ testado e descartado |
| 14 | Versão com app Dabble | Baixa | ✅ alternativa |
| 15 | Carenagem | Média | PREENCHER |
| 16 | Documentação no GitHub | Alta | ✅ |

## 5. Planejamento

| Data | Etapa |
|---|---|
| 14/08/2026 | Requisitos, lista de materiais, croqui e dimensões dos componentes |
| 21/08/2026 | Protótipo de bancada com ESP32 + Wi-Fi (fotos e vídeo) |
| PREENCHER | Fabricação/montagem do chassi |
| 18/09/2026 | Montagem final com Arduino Uno + Bluetooth, testes por módulo, integração e correções |
| PREENCHER | Carenagem |
| 18/09/2026 | Documentação final do repositório |

## 6. Decisões de projeto

| Decisão | Motivo |
|---|---|
| **Kit 2WD / peças avulsas + chassi próprio** | Estratégia discutida na [lista de materiais](lista-de-materiais-aula13.md#1-decisão-de-estratégia-kit-pronto-ou-peças-avulsas) |
| **Protótipo com ESP32 + Wi-Fi primeiro** | Validar motores, ponte H e controle remoto na bancada antes da montagem |
| **Versão final com Arduino Uno + HC-05 (Bluetooth)** | PREENCHER: motivo da troca do ESP32 pelo Arduino Uno |
| **HC-05 no lugar do HC-06 do croqui** | O HC-05 também funciona como escravo (slave), como o HC-06, e recebe os comandos do app do mesmo jeito |
| **HC-05 via SoftwareSerial (D10/D11)** | Deixa a serial USB livre para gravar o código e depurar pelo Monitor Serial |
| **Alimentação separada** (4 AA nos motores, 9 V no Arduino) | O pico de corrente dos motores não reinicia o Arduino |
| **GND comum direto no borne da ponte H** | Pela protoboard dava mau contato e falhas intermitentes ([T1](05-testes-e-resultados.md#t1--motores-falhando-de-forma-intermitente)) |
| **Sensor lido a cada 100 ms com `millis()`** | O carrinho continua recebendo comandos enquanto mede |
| **Mediana das 3 últimas leituras** | Leituras isoladas erradas paravam o carrinho ou liberavam com obstáculo ([T4](05-testes-e-resultados.md#t4--sensor-hc-sr04-isolado)) |
| **`pulseInLong()` em vez de `pulseIn()`** | `pulseIn()` encurta a medição quando chega dado do Bluetooth |
| **Parar a 15 cm e liberar a 20 cm** | A diferença evita ficar travando e destravando no limite |
| **ENA/ENB com jumper (velocidade máxima)** | O controle de velocidade por PWM deixou os motores sem força com 4 pilhas ([T7](05-testes-e-resultados.md#t7--desvio-lateral-e-controle-de-velocidade)) |
| **App serial (letras F/B/L/R/S)** | Simples e mostra as respostas do carrinho; a versão Dabble ficou como alternativa |
| **Testar peça por peça** | Facilitou achar a causa de cada problema |

## 7. Evolução e alterações

| Versão | Data | Mudança |
|---|---|---|
| v0 | 14/08/2026 | Requisitos, lista de materiais e croqui do chassi |
| v1 | 21/08/2026 | Protótipo **ESP32 + Wi-Fi**: página web com botões, velocidade (PWM) e distância; trava de segurança a 20 cm |
| v2 | 18/09/2026 | Versão **Arduino Uno + HC-05**: comandos por letra, parada por obstáculo a 15 cm, buzzer, distância enviada ao app |
| v2.1 | 18/09/2026 | GND comum direto no borne da ponte H (correção de hardware) |
| v2.2 | 18/09/2026 | Uma medição por ciclo, `pulseInLong()`, filtro de **mediana** |
| v2.3 | 18/09/2026 | Teste de controle de velocidade por PWM — **revertido** |
| v2.4 | 18/09/2026 | Versão alternativa com app **Dabble** |
| **v3 (final do Check Point 01)** | 18/09/2026 | App serial, motores em velocidade máxima, código reorganizado e comentado |
| v4 | 23–24/09/2026 | Volta para a **ESP32**: ligação da ponte H, teste dos motores e controle de **PS4** (Bluepad32) no estilo de jogo de corrida, com luz e vibração no controle; alternativas por Bluetooth clássico e Wi-Fi — ver [versão 2](07-versao-esp32-ps4.md) |
| **v5** | 24/09/2026 | **TinyML**: modelo do Edge Impulse classifica a distância (perto, médio, longe) dentro da ESP32 e bloqueia a frente com obstáculo perto — ver [TinyML](08-tinyml.md) |

Detalhes de cada problema e correção em [Testes e resultados](05-testes-e-resultados.md).
