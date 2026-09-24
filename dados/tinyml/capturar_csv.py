"""Captura o CSV (feature,label) que o sketch tinyml_b_preprocessamento
imprime na porta serial da ESP32.

Uso:
    python capturar_csv.py --segundos 60 --saida captura_perto.csv

Só guarda as linhas no formato "0.1234,0" (ignora as mensagens do boot
da ESP32 e qualquer outra coisa que apareça na serial).
"""

import argparse
import re
import sys
import time
from collections import Counter

import serial

LINHA_CSV = re.compile(r"^\d\.\d{4},[012]$")


def capturar(porta, baud, segundos, saida):
    contagem = Counter()
    features = []

    with serial.Serial(porta, baud, timeout=0.5) as ser, open(
        saida, "w", encoding="utf-8", newline=""
    ) as arquivo:
        arquivo.write("feature,label\n")

        fim = time.time() + segundos

        while time.time() < fim:
            linha = ser.readline().decode("utf-8", errors="ignore").strip()

            if not LINHA_CSV.match(linha):
                continue

            feature, label = linha.split(",")

            arquivo.write(linha + "\n")

            contagem[label] += 1
            features.append(float(feature))

    return contagem, features


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--porta", default="COM8")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--segundos", type=int, default=60)
    parser.add_argument("--saida", required=True)
    args = parser.parse_args()

    try:
        contagem, features = capturar(
            args.porta, args.baud, args.segundos, args.saida
        )
    except serial.SerialException as erro:
        print(f"ERRO ao abrir {args.porta}: {erro}")
        print("Feche o Monitor Serial da IDE e tente de novo.")
        sys.exit(1)

    total = sum(contagem.values())

    print(f"Arquivo: {args.saida}")
    print(f"Linhas capturadas: {total}")

    for label in sorted(contagem):
        print(f"  classe {label}: {contagem[label]}")

    if features:
        print(
            f"feature: min={min(features):.4f} max={max(features):.4f} "
            f"(x198+2 = {min(features) * 198 + 2:.1f} a "
            f"{max(features) * 198 + 2:.1f} cm)"
        )


if __name__ == "__main__":
    main()
