#!/usr/bin/env python3.10

# from matplotlib import dates as mdates
import sys

import pandas as pd
from matplotlib import pyplot as plt

# import seaborn as sns
# import numpy as np

def load_data(filename: str) -> pd.DataFrame:
    return pd.read_csv(
        filename,
        sep=";",
        decimal=".",
    )


def _plot_ber(df: pd.DataFrame):
    df.plot.bar()
    plt.xticks(rotation = 5)
    plt.show()


def _plot_snr(df: pd.DataFrame):
    df.plot.bar()
    plt.xticks(rotation = 5)
    plt.show()


def plot_ber(df):
    df = df.drop('snr', axis=1)
    df = df.groupby(["method"]).mean()
    _plot_ber(df)


def plot_snr(df):
    df = df.groupby(["method"]).mean()
    _plot_snr(df['snr'])


def plot_param_comparison(df):
    for method in ("lsb", "echo", "echo-hc", "tone", "phase"):
        try:
            _df = df.loc[df["method"] == method]
            _df = _df.groupby(["method", "params"]).mean()
            # snr
            _plot_ber(_df["snr"])

            _df = _df.drop('snr', axis=1)
            # ber
            _df = _df.groupby(["method", "params"]).mean()
            _plot_ber(_df)
        except IndexError:
            pass


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.stderr.write("Expected two arguments")
        sys.exit(1)

    df = load_data(sys.argv[1])
    if sys.argv[2] == 'params':
        plot_param_comparison(df)
    elif sys.argv[2] == 'cmp':
        plot_snr(df)
        plot_ber(df)
