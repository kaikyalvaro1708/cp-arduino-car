# Projeto mecânico e fabricação

[← Voltar ao README](../README.md)

## 1. Croqui do chassi (Aula 13 — 14/08/2026)

Base de **150 × 210 mm** com duas rodas motrizes atrás, roda boba na frente e os componentes distribuídos conforme peso e ruído elétrico.

![Croqui do chassi](../image/croqui-chassi.png)

### Decisões de projeto do layout

- **Bateria sobre o eixo motriz.** O suporte de 4× AA é o item mais pesado (~110 g com as pilhas) e está posicionado entre Y=145 e Y=203 mm, exatamente sobre a linha dos motores. Isso joga o centro de massa em cima das rodas de tração e evita que o carrinho patine na partida ou empine ao frear.
- **Roda boba na frente, motores atrás.** Como o peso está atrás, a roda boba fica descarregada — ela só apoia, não sustenta. É por isso que ela pode ser pequena (Ø 30 mm) sem comprometer a estabilidade.
- **Módulo Bluetooth no lado oposto ao L298N.** A ponte H chaveia corrente nos motores e gera ruído eletromagnético; o módulo Bluetooth é o componente mais sensível a isso. Separá-los diagonalmente é praticamente de graça no croqui e evita queda de conexão durante a locomoção. *(No croqui aparece o HC-06; na versão final foi usado o HC-05, que ocupa o mesmo lugar.)*
- **Sensor na borda frontal, sem parafuso à frente.** O HC-SR04 tem cone de detecção de 15°. Qualquer parafuso ou espaçador na frente dele vira eco falso.

### Pontos levantados para a revisão do croqui

- [ ] Cotas de furação — a posição de cada furo M3 medida a partir de uma referência única (canto frontal esquerdo, por exemplo).
- [ ] Recorte do eixo dos motores — os eixos atravessam a lateral da base (fenda ou rebaixo).
- [ ] Vista lateral — altura livre do solo (~15 a 20 mm com roda de 65 mm e motor sob a chapa) e altura total da pilha de componentes.
- [ ] Passagem de fiação — dois ou três rasgos oblongos entre a região dos motores e a ponte H.
- [ ] Acesso ao USB e ao reset do Arduino.
- [ ] Legenda numerada casando com os IDs da ficha de requisitos.
- [ ] Desenho em escala 1:1 ou 1:2 (Fusion 360, Inkscape ou Figma) exportado em DXF/STL para fabricação.

<!-- PREENCHER: marque [x] no que foi feito na versão final -->

## 2. Dimensões dos componentes (mm)

| Componente     | Comprimento | Largura | Altura | Forma de Fixação |
| -------------- | ----------- | ------- | ------ | ---------------- |
| Motor Esquerdo | 68,9        | 17,1    | 21,5   |                  |
| Motor direito  | 68,9        | 17,1    | 21,5   |                  |
| Arduino/ESP32  | 50,7        | 27,6    | 11,1   |                  |
| Ponte H        | 41,3        | 41,3    | 25,6   |                  |
| Bateria        | 74,05       | 20,7    | 20,8   |                  |
| Sensor         | 44,6        | 19,1    | 14,4   |                  |

<!-- PREENCHER: coluna "Forma de Fixação" (ex.: parafuso M3, abraçadeira, fita dupla-face) -->

## 3. Fabricação do chassi

> **PREENCHER** — descrever como o chassi final foi feito:
>
> - **Processo:** impressão 3D / corte a laser / kit comercial adaptado / outro
> - **Material:** PLA / MDF 3 mm / acrílico / outro
> - **Arquivos-fonte e STL/DXF:** colocar na pasta [`mecanico/`](../mecanico/)
> - **Versões:** o que mudou entre o croqui e a peça final (furação, recortes, medidas)
> - **Fotos da fabricação/impressão:** salvar em `image/` e listar abaixo

| Versão | Data | Alteração |
|---|---|---|
| Croqui | 14/08/2026 | Layout inicial 150 × 210 mm |
| PREENCHER | PREENCHER | PREENCHER |

<!-- ![Fabricação do chassi](../image/fabricacao-chassi.jpg) -->

## 4. Carenagem

> **PREENCHER** — material usado (papelão, MDF, impressão 3D, reciclado...), como foi fixada no chassi e como se acessa os componentes (tampa removível, encaixe, etc.).

<!-- ![Carenagem](../image/carenagem.jpg) -->
