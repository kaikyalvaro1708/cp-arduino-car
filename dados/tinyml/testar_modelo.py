"""Lê por alguns segundos a saída do sketch tinyml_c_modelo e resume:
quanto o modelo concordou com a regra (20 e 60 cm) e onde discordou.

Uso:
    python testar_modelo.py --segundos 40 --saida teste_modelo_placa.txt
"""

import argparse
import re
import sys
import time
from collections import Counter

import serial

LINHA = re.compile(
    r"dist=\s*([\d.]+) cm \| feature=([\d.]+) \| "
    r"modelo=(\w+) \(\s*(\d+)%\) \| regra=(\w+) \| (OK|DIFF)"
)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--porta", default="COM8")
    parser.add_argument("--segundos", type=int, default=40)
    parser.add_argument("--saida", required=True)
    args = parser.parse_args()

    amostras = []

    try:
        with serial.Serial(args.porta, 115200, timeout=0.5) as ser, open(
            args.saida, "w", encoding="utf-8"
        ) as arquivo:
            fim = time.time() + args.segundos

            while time.time() < fim:
                linha = ser.readline().decode("utf-8", errors="ignore").rstrip()

                achou = LINHA.search(linha)

                if not achou:
                    continue

                arquivo.write(linha + "\n")

                dist, feature, modelo, conf, regra, status = achou.groups()

                amostras.append(
                    (float(dist), modelo, int(conf), regra, status == "OK")
                )
    except serial.SerialException as erro:
        print(f"ERRO ao abrir {args.porta}: {erro}")
        print("Feche o Monitor Serial da IDE e tente de novo.")
        sys.exit(1)

    total = len(amostras)

    if total == 0:
        print("Nenhuma linha do modelo foi lida.")
        sys.exit(1)

    acertos = sum(1 for a in amostras if a[4])

    print(f"Amostras lidas: {total}")
    print(f"Modelo = regra: {acertos} ({100 * acertos / total:.1f}%)")

    print("Classes previstas pelo modelo:", dict(Counter(a[1] for a in amostras)))
    print("Classes pela regra:            ", dict(Counter(a[3] for a in amostras)))

    dists = [a[0] for a in amostras]
    print(f"Distancias vistas: {min(dists):.1f} a {max(dists):.1f} cm")

    confs = [a[2] for a in amostras]
    print(
        f"Confianca do modelo: media {sum(confs) / total:.0f}%, "
        f"minima {min(confs)}%"
    )

    diffs = [a for a in amostras if not a[4]]

    print(f"Discordancias: {len(diffs)}")

    for dist, modelo, conf, regra, _ in sorted(diffs)[:30]:
        print(f"  {dist:6.1f} cm  modelo={modelo} ({conf}%)  regra={regra}")


if __name__ == "__main__":
    main()
