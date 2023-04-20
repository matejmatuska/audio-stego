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

def plot_ber(df):
    df = df.drop('snr', axis=1)
    df = df.groupby(["method"]).mean()
    #print(df)
    df.plot.bar()
    plt.show()

def plot_snr(df):
    df = df.groupby(["method"]).mean()
    #print(df)
    df['snr'].plot.bar()
    plt.show()


def print_averages(df):
#    df['EXTRACT'] *= 100
#    df['DOWNSAMPLE'] *= 100
#    df['UPSAMPLE'] *= 100
#    df['AMPLIFY'] *= 100
#    df['ATTENUATE'] *= 100
#    df['MORE_BITS'] *= 100
#    df['LESS_BITS'] *= 100
    df = df.groupby(["method"]).mean()
    print(df)
    df.plot()
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.stderr.write("Expected one argument")
        sys.exit(1)

    df = load_data(sys.argv[1])
    plot_snr(df)
    plot_ber(df)
