"""Junta as capturas em um único dataset (feature,label).

Gera dataset_distancia.csv, com as linhas na ordem da captura.
"""

import csv
from collections import Counter

CAPTURAS = [
    "captura_perto.csv",
    "captura_medio.csv",
    "captura_longe.csv",
    "captura_transicao.csv",
]

DIST_MIN = 2
DIST_MAX = 200

NOMES = {"0": "perto", "1": "medio", "2": "longe"}


def ler(nome):
    with open(nome, encoding="utf-8", newline="") as arquivo:
        return list(csv.DictReader(arquivo))


def escrever(nome, linhas):
    with open(nome, "w", encoding="utf-8", newline="") as arquivo:
        arquivo.write("feature,label\n")

        for linha in linhas:
            arquivo.write(f"{linha['feature']},{linha['label']}\n")


def main():
    linhas = []

    for nome in CAPTURAS:
        linhas += ler(nome)

    escrever("dataset_distancia.csv", linhas)

    contagem = Counter(linha["label"] for linha in linhas)

    print(f"Total: {len(linhas)} linhas")

    for label in sorted(contagem):
        valores = [
            float(linha["feature"]) * (DIST_MAX - DIST_MIN) + DIST_MIN
            for linha in linhas
            if linha["label"] == label
        ]

        print(
            f"  classe {label} ({NOMES[label]}): {contagem[label]} linhas "
            f"({100 * contagem[label] / len(linhas):.0f}%), "
            f"{min(valores):.1f} a {max(valores):.1f} cm"
        )

    print(f"Valores diferentes de feature: {len({l['feature'] for l in linhas})}")


if __name__ == "__main__":
    main()
