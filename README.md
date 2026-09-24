# 🚗 Carrinho-Robô Bluetooth com Detecção de Obstáculos

**Check Point 01 — Project-based Maker Lab (FIAP)** · Profª Dra. Gedeane G. S. Kenshima

## 👥 Integrantes do Grupo

| Nome                           | RM     |
| ------------------------------ | ------ |
| Kaiky Alvaro Miranda           | 98118  |
| Guilherme Morais  Barbosa      | 551981 |
| Juan Pinheiro de França        | 552202 |
| Matheus Gusmão Aragão          | 550826 |
| Júlia Marques Mendes das Neves | 98680  |

---

## 🎯 Objetivo

Desenvolver um carrinho-robô funcional, controlado pelo celular sem fio, que **detecta obstáculos à frente e para sozinho antes de colidir**, integrando projeto mecânico, eletrônica, programação e documentação.

## 📝 Descrição

O carrinho usa um **Arduino Uno** como controlador, uma **ponte H L298N** para acionar dois motores DC com caixa de redução, um módulo **Bluetooth HC-05** para receber comandos de um app no celular e um **sensor ultrassônico HC-SR04** na frente.

Enquanto anda para frente, o Arduino mede a distância 10 vezes por segundo. Se algo aparece a menos de **15 cm**, ele para os motores, apita o **buzzer**, avisa o app e **bloqueia o avanço** até o caminho ficar livre (mais de 20 cm). Ré e curvas continuam liberadas para o carrinho sair do obstáculo.

A alimentação é separada: **4 pilhas AA** para os motores e **bateria 9 V** para o Arduino, com GND comum.

O projeto começou com um **protótipo em ESP32 controlado por Wi-Fi** (21/08) e evoluiu para a versão final com **Arduino Uno e Bluetooth** — veja a [evolução](docs/01-requisitos-e-planejamento.md#7-evolução-e-alterações). Depois do Check Point 01 o projeto **voltou para a ESP32**, agora com controle de PS4 e TinyML — veja a seção **Versão 2** abaixo.

## ⚙️ Principais funcionalidades

- **Controle remoto sem fio** por Bluetooth: frente, ré, esquerda, direita e parar.
- **Parada automática por obstáculo** com sensor ultrassônico.
- **Bloqueio de avanço** enquanto houver obstáculo à frente.
- **Alerta sonoro** (buzzer) ao detectar obstáculo.
- **Filtro de mediana** nas leituras do sensor para ignorar leituras falsas.
- **Telemetria**: distância enviada ao app a cada 300 ms.
- **Alimentação por bateria**, sem depender do cabo USB.

## 🕹️ Como usar (resumo)

1. Ligue as pilhas da ponte H e a bateria 9 V do Arduino.
2. Pareie o celular (Android) com o **HC-05** — senha `1234` ou `0000`.
3. No app de controle Bluetooth serial, use os comandos:

| Comando | Letra | Ação |
|---|---|---|
| Frente | `F` | anda para frente (recusado se houver obstáculo) |
| Ré | `B` | anda para trás |
| Esquerda | `L` | vira para a esquerda |
| Direita | `R` | vira para a direita |
| Parar | `S` | para |

Instruções completas: [docs/06-evidencias-e-uso.md](docs/06-evidencias-e-uso.md)

## 🚀 Versão 2 — ESP32, controle de PS4 e TinyML

Depois do Check Point 01, o carrinho voltou para a **ESP32**, agora controlado por um **controle de PS4** (Bluepad32) no estilo de jogo de corrida — **R2** acelera, **L2** dá ré e o analógico esquerdo vira — e com resposta no controle (cor da luz e vibração). Um **modelo de TinyML**, treinado no Edge Impulse com dados reais do sensor HC-SR04, roda dentro da ESP32 e classifica a distância à frente em **perto**, **médio** ou **longe**: com obstáculo perto, a frente é bloqueada.

- **Projeto no Edge Impulse (público):** <https://studio.edgeimpulse.com/public/1120730/live>
- Modelo com 3 classes, **98,5% de acurácia** na validação, quantizado (int8), de 0 a 1 ms por previsão na ESP32.
- Código: [`code/carrinho_esp32_ps4_ia`](code/carrinho_esp32_ps4_ia/)
- Hardware, controles, como compilar e problemas resolvidos: [docs/07-versao-esp32-ps4.md](docs/07-versao-esp32-ps4.md)
- Relatório do TinyML (Aulas 19 e 20): [docs/08-tinyml.md](docs/08-tinyml.md) · dados e scripts: [dados/tinyml](dados/tinyml/README.md)

## 📁 Organização do repositório

```
cp-arduino-car/
├── README.md
├── docs/
│   ├── 01-requisitos-e-planejamento.md  → requisitos, MVP, backlog, decisões e evolução
│   ├── 02-projeto-mecanico.md           → croqui, dimensões, chassi e carenagem
│   ├── 03-hardware.md                   → componentes, pinagem, alimentação e esquemas
│   ├── 04-software.md                   → funcionamento do código
│   ├── 05-testes-e-resultados.md        → testes, problemas e correções
│   ├── 06-evidencias-e-uso.md           → fotos, vídeos e instruções de uso
│   ├── 07-versao-esp32-ps4.md           → versão 2: ESP32, controle de PS4, códigos e problemas resolvidos
│   ├── 08-tinyml.md                     → relatório do TinyML (Aulas 19 e 20)
│   └── lista-de-materiais-aula13.md     → materiais, preços e custo (Aula 13)
├── code/
│   ├── arduino_carrinho_codigo/         → ✅ CÓDIGO FINAL do Check Point 01 (Arduino Uno + Bluetooth)
│   ├── carrinho_dabble/                 → versão alternativa com o app Dabble
│   ├── carrinho_wifi_esp32/             → protótipo ESP32 + Wi-Fi (21/08)
│   ├── carrinho_esp32_ps4_ia/           → ✅ CÓDIGO FINAL DA VERSÃO 2 (ESP32 + PS4 + modelo TinyML)
│   ├── carrinho_esp32_ps4/              → versão 2 só com o controle de PS4
│   ├── carrinho_esp32_wifi/             → versão 2: alternativa por Wi-Fi (página web)
│   ├── carrinho_esp32/                  → versão 2: alternativa por Bluetooth clássico (Android)
│   ├── tinyml/                          → TinyML: leitura bruta, pré-processamento e modelo na ESP32
│   └── testes/                          → testes isolados: motores, bluetooth, sensor, buzzer e motores da ESP32
├── dados/tinyml/                        → dataset, scripts de captura e biblioteca do modelo
├── mecanico/                            → arquivos de modelagem/fabricação do chassi e carenagem
├── image/                               → croqui, esquemas elétricos e fotos
└── video/                               → vídeos de funcionamento
```

## 📚 Documentação

| Documento | Conteúdo |
|---|---|
| [Requisitos, planejamento e evolução](docs/01-requisitos-e-planejamento.md) | requisitos, proposta, MVP, backlog, planejamento, decisões e histórico de versões |
| [Projeto mecânico e fabricação](docs/02-projeto-mecanico.md) | croqui, dimensões, chassi e carenagem |
| [Hardware e eletrônica](docs/03-hardware.md) | componentes, pinagem, alimentação, esquemas e fotos da montagem |
| [Software](docs/04-software.md) | estrutura do código, motores, Bluetooth e sensor |
| [Testes e resultados](docs/05-testes-e-resultados.md) | testes realizados, problemas encontrados e soluções |
| [Evidências e uso](docs/06-evidencias-e-uso.md) | fotos, vídeos e instruções de utilização |
| [Versão 2: ESP32 e PS4](docs/07-versao-esp32-ps4.md) | componentes, pinos, controles, como compilar, problemas e soluções |
| [TinyML](docs/08-tinyml.md) | sensor, pré-processamento, classes, dataset, treino no Edge Impulse, teste na placa e integração |
| [Lista de materiais (Aula 13)](docs/lista-de-materiais-aula13.md) | BOM, fornecedores e estimativa de custo |

## 🛠️ Tecnologias

Arduino Uno · C++ (Arduino IDE) · Ponte H L298N · Bluetooth HC-05 · Sensor HC-SR04 · ESP32 · Controle de PS4 (Bluepad32) · Edge Impulse (TinyML) · Python (pyserial)
