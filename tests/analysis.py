#!/usr/bin/env python3.10

# from matplotlib import dates as mdates
import sys

import pandas as pd
import seaborn as sns
import matplotlib
from matplotlib import pyplot as plt

# import numpy as np


def load_data(filename: str) -> pd.DataFrame:
    return pd.read_csv(
        filename,
        sep=";",
        decimal=".",
    )


def _plot_ber(df: pd.DataFrame):
    df = df.drop('params', axis=1)
    df = df.reset_index()
    df = df.melt(['method', 'type'], var_name='cols', value_name='vals')

    g = sns.catplot(x='method', y='vals', hue='cols',
                    col='type', data=df, kind='bar', legend_out=True)
    plt.show()


def _plot_snr(df: pd.DataFrame):
    df.plot.bar()
    plt.xticks(rotation=5)
    plt.show()


def plot_ber(df: pd.DataFrame, save_fig=False):
    df = df.drop('snr', axis=1)
    df = df.drop('params', axis=1)
    df = df.drop('type', axis=1)

    df = df.groupby(['method']).mean()
    df = df.reset_index()
    df = df.melt(['method'], var_name='cols', value_name='vals')

    plt.figure(figsize=(6,4))
    ax = sns.barplot(x='method', y='vals', hue='cols', data=df)
    ax.get_yaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax.grid(visible=True, which='major', linewidth=0.8, axis='y', aa=True)
    ax.grid(visible=True, which='minor', linewidth=0.4, axis='y', linestyle='--', aa=True)

    ax.set(xlabel='Metóda', ylabel='Bit Error Rate')
    sns.move_legend(ax, title='', loc='best')

    if (save_fig):
        plt.tight_layout()
        plt.savefig('ber.pdf')
        plt.clf()
    else:
        plt.show()


def plot_ber_by_type(df, save_fig=False):
    df = df.drop('snr', axis=1)
    df = df.drop('params', axis=1)

    df = df.groupby(['method', 'type']).mean()
    df = df.reset_index()
    df = df.melt(['method', 'type'], var_name='cols', value_name='vals')

    g = sns.catplot(x='method', y='vals', hue='cols',
                    col='type', data=df, kind='bar', legend_out=True)
    g._legend.set_title('')

    g.set_axis_labels("Metóda", "Bit Error Rate")
    if (save_fig):
        plt.savefig('ber_by_type.pdf')
    else:
        plt.show()


def plot_snr(df, save_fig=False):
    df = df.groupby(["method"]).mean()
    df = df.reset_index()

    plt.figure(figsize=(6,4))
    ax = sns.barplot(x='method', y='snr', data=df,
                     palette=sns.color_palette(n_colors=1))
    ax.set(xlabel='Metóda', ylabel='Signal to Noise Ratio [dB]')
    if (save_fig):
        plt.savefig('snr.pdf')
        plt.clf()
    else:
        plt.show()


def plot_snr_by_type(df, save_fig=False):
    df = df.groupby(['method', 'type']).mean()
    df = df.reset_index()

    ax = sns.barplot(x='method', y='snr', hue='type', data=df)
    ax.set(xlabel='Metóda', ylabel='Signal to Noise Ratio [dB]')
    sns.move_legend(ax, title='Typ nahrávky', loc='best')
    plt.tight_layout()
    if (save_fig):
        plt.savefig('snr_by_type.pdf')
        plt.clf()
    else:
        plt.show()


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
        mod_map = {'resampling': 'Prevzorkovanie', 'attenuation': 'Stlmenie',
                   'amplification': 'Zosilnenie', 'extraction': 'Extrakcia',
                   'requantization': 'Prekvantovanie'}
        df.rename(columns=mod_map, inplace=True)

        df['type'] = df['type'].map(
            {'music': 'Hudba', 'noise': 'Zvuky', 'speech': 'Reč'})

        save_fig = True
        sns.set_style('whitegrid')
        plot_snr(df, save_fig)
        plot_snr_by_type(df, save_fig)
        plot_ber(df, save_fig)
        plot_ber_by_type(df, save_fig)
