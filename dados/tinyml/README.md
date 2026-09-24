# Dados e modelo TinyML (Aulas 19 e 20)

[← Voltar ao README](../../README.md) · [Relatório completo](../../docs/08-tinyml.md)

Sensor: **HC-SR04** (ultrassônico) ligado à ESP32. O modelo classifica a distância à frente em três classes, usando a média das 5 últimas leituras:

| Classe | Nome | Distância |
|---|---|---|
| 0 | PERTO | menos de 20 cm |
| 1 | MEDIO | de 20 a 60 cm |
| 2 | LONGE | 60 cm ou mais |

Projeto no Edge Impulse (público): <https://studio.edgeimpulse.com/public/1120730/live>

## Arquivos

| Arquivo | O que é |
|---|---|
| `captura_perto.csv`, `captura_medio.csv`, `captura_longe.csv`, `captura_transicao.csv` | capturas feitas com a ESP32 (cada linha é `feature,label`) |
| `dataset_distancia.csv` | as quatro capturas juntas (1.980 linhas): é o arquivo enviado ao Edge Impulse |
| `capturar_csv.py` | lê a serial da ESP32 e grava só as linhas válidas do CSV |
| `juntar_csv.py` | junta as capturas em `dataset_distancia.csv` |
| `testar_modelo.py` | lê a saída do sketch `tinyml_c_modelo` e compara o modelo com a regra de 20 e 60 cm |
| `teste_modelo_placa.txt` | log do teste do modelo rodando na ESP32 (395 leituras) |
| `ei-tinyml-ldr-zoe-arduino-1.0.1.zip` | biblioteca Arduino exportada do Edge Impulse (o modelo treinado) |

## Como reproduzir

1. Grave `code/tinyml/tinyml_b_preprocessamento` na ESP32 (placa **DOIT ESP32 DEVKIT V1**, pacote ESP32) e **feche o Monitor Serial**, senão a porta fica ocupada.
2. Instale a dependência: `pip install -r requirements.txt`.
3. Capture cada classe movendo um objeto liso e reto na frente do sensor. A porta padrão é `COM8`; use `--porta` para mudar:
   ```bash
   python capturar_csv.py --segundos 60 --saida captura_perto.csv
   python capturar_csv.py --segundos 60 --saida captura_medio.csv
   python capturar_csv.py --segundos 40 --saida captura_longe.csv
   python capturar_csv.py --segundos 40 --saida captura_transicao.csv
   ```
   (a transição cruza devagar os limites de 20 e 60 cm)
4. Junte tudo: `python juntar_csv.py`.
5. No Edge Impulse, envie `dataset_distancia.csv` (CSV Wizard: separador vírgula, com cabeçalho, **não** é série temporal, rótulo = coluna `label`, valores = coluna `feature`). Impulse com *Raw data* e *Classification*, 20 ciclos, learning rate 0.005. Depois *Deployment > Arduino Library*.
6. Na Arduino IDE: *Sketch > Incluir Biblioteca > Adicionar biblioteca .ZIP* com o zip deste diretório.
7. Grave `code/tinyml/tinyml_c_modelo` e rode `python testar_modelo.py --segundos 40 --saida teste_modelo_placa.txt` movendo o objeto.
