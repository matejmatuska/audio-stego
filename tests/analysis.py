#!/usr/bin/env python3.10

# from matplotlib import dates as mdates
import sys
import textwrap

import matplotlib
import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt

METHOD_NAME_MAP = {'lsb': 'Substitúcia najnižšieho bitu',
                   'phase': 'Kódovanie vo fáze',
                   'echo': 'Vkladanie ozveny',
                   'echo-hc': 'Navrhnutá metóda',
                   'tone': 'Vkladanie tónu',
                   }

METHOD_ORDER = ['Substitúcia najnižšieho bitu',
                'Kódovanie vo fáze',
                'Vkladanie tónu',
                'Vkladanie ozveny',
                'Navrhnutá metóda',
                ]


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


def plot_ber(df: pd.DataFrame, save_fig=False, filename='ber.pdf'):
    _df = df.drop('snr', axis=1)
    _df = _df.drop('params', axis=1)
    _df = _df.drop('type', axis=1)

    grouped = _df.groupby(['method']).mean()
    print(grouped)
    _df = _df.melt(['method'], var_name='cols', value_name='vals')

    plt.figure(figsize=(7, 4))

    ax = sns.barplot(x='method', y='vals', hue='cols',
                     data=_df, order=METHOD_ORDER, errorbar=None)
    ax.get_yaxis().set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    ax.get_yaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax.grid(visible=True, which='major', linewidth=0.8, axis='y', aa=True)
    ax.grid(visible=True, which='minor', linewidth=0.4,
            axis='y', linestyle='--', aa=True)

    ax.set(xlabel='Metóda', ylabel='Bitová chybovosť')
    sns.move_legend(ax, title='', loc='best')

    labels = [textwrap.fill(label.get_text(), 11)
              for label in ax.get_xticklabels()]
    ax.set_xticklabels(labels)

    if (save_fig):
        plt.tight_layout()
        plt.savefig(filename)
        plt.clf()
    else:
        plt.show()


def plot_ber_by_type(df, save_fig=False):
    df = df.drop('snr', axis=1)
    df = df.drop('params', axis=1)

    df = df.melt(['method', 'type'], var_name='cols', value_name='vals')

    plt.figure(figsize=(6, 12))
    g = sns.catplot(x='method', y='vals', hue='cols', errorbar=None, col_wrap=1,
                    col='type', data=df, kind='bar', order=METHOD_ORDER, legend_out=False)
    for ax in g.axes.flat:
        ax.get_yaxis().set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
        ax.get_yaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
        ax.grid(visible=True, which='major', linewidth=0.8, axis='y', aa=True)
        ax.grid(visible=True, which='minor', linewidth=0.4,
                axis='y', linestyle='--', aa=True)

        ax.set(xlabel='Metóda', ylabel='Bitová chybovosť')

        labels = [textwrap.fill(label.get_text(), 11)
                  for label in ax.get_xticklabels()]
        ax.set_xticklabels(labels)
        sns.move_legend(g, "upper left", bbox_to_anchor=(.55, .45), title='')
    if (save_fig):
        plt.savefig('ber_by_type.pdf')
    else:
        plt.show()


def plot_snr(df, save_fig=False):
    df = df.groupby(["method"]).mean()
    df = df.reset_index()

    plt.figure(figsize=(6, 4))
    ax = sns.barplot(x='method', y='snr', data=df,
                     palette=sns.color_palette(n_colors=1))
    ax.set(xlabel='Metóda', ylabel='Pomer signálu a šumu [dB]')
    if (save_fig):
        plt.savefig('snr.pdf')
        plt.clf()
    else:
        plt.show()


def plot_snr_by_type(df, save_fig=False):
    # df = df.groupby(['method', 'type']).mean()
    # df = df.reset_index()

    plt.figure(figsize=(7, 4))
    ax = sns.barplot(x='method', y='snr', hue='type', data=df,
                     errorbar='sd', order=METHOD_ORDER)
    ax.set(xlabel='Metóda', ylabel='Pomer signálu a šumu [dB]')

    labels = [textwrap.fill(label.get_text(), 11)
              for label in ax.get_xticklabels()]
    ax.set_xticklabels(labels)

    sns.move_legend(ax, title='Typ nahrávky', loc='best')
    plt.tight_layout()

    if (save_fig):
        plt.savefig('snr_by_type.pdf')
        plt.clf()
    else:
        plt.show()


def plot_used_capacity_dep_length(save_fig=False):
    SAMPLERATE = 44100
    dur = np.arange(0, 60, 0.1)

    df = pd.DataFrame()
    df['duration'] = pd.Series(dur)
    df['samples'] = pd.Series(dur * SAMPLERATE)

    df['phase'] = 186-23
    df['echo'] = pd.Series(df.samples // 4096)
    df['echo-hc'] = pd.Series(df.samples // 4096 * 4)
    df['lsb'] = df.samples * 2
    df['tone'] = pd.Series(df.samples // 1024)

    df.rename(columns=METHOD_NAME_MAP, inplace=True)

    df = df.melt(['samples', 'duration'],
                 var_name='Metóda', value_name='vals')

    df['vals'] = df['vals'].astype(int)

    sns.set_style('whitegrid')

    plt.figure(figsize=(6, 4))
    # TODO order
    ax = sns.lineplot(data=df, x='duration', y='vals', hue='Metóda')
    ax.set(xlabel='Dĺžka signálu [s]', ylabel='Kapacita')
    ax.margins(x=0)
    plt.yscale('log', base=2)

    if (save_fig):
        plt.savefig('capacity.pdf')
        plt.clf()
    else:
        plt.show()


def plot_capacity_dep_length():

    SAMPLERATE = 44100
    x = np.array([512, 2048, 4096, 8192, 1024])
    dur = np.arange(0, 60, 0.1)
    ndur = np.repeat(dur, x.shape[0])
    x = np.tile(x, ndur.shape[0] // x.shape[0])

    df = pd.DataFrame()
    df['framesize'] = pd.Series(x)
    df['duration'] = pd.Series(ndur)
    df['samples'] = pd.Series(ndur * SAMPLERATE)

    df['phase'] = pd.Series(df.samples / x)  # TODO
    df['echo'] = pd.Series(df.samples // df.framesize)
    df['echo-hc'] = pd.Series(df.samples // df.framesize * 4)
    df['lsb'] = df.samples
    df['tone'] = pd.Series(df.samples // df.framesize)

    df = df.melt(['samples', 'duration', 'framesize'],
                 var_name='cols', value_name='vals')

    def map_chosen(method, framesize):
        if (method == 'phase' and framesize == 1024):
            return 'chosen'
        if (method == 'echo' and framesize == 4096):
            return 'chosen'
        if (method == 'echo-hc' and framesize == 4096):
            return 'chosen'
        if (method == 'lsb' and framesize == 8192):
            return 'chosen'
        if (method == 'tone' and framesize == 2046):
            return 'chosen'

        return 'not chosen'

    df['meaning'] = df.apply(lambda x: map_chosen(
        x['cols'], x['framesize']), axis=1)

    df = df[(df.meaning == 'chosen') | df.framesize.isin((512, 8192))]
    # df = df[~((df.cols == 'lsb') & (df.framesize != 'chosen'))]

    df['vals'] = df['vals'].astype(int)

    sns.set_style('whitegrid')
    sns.set_context('paper')
    ax = sns.lineplot(data=df, x='duration', y='vals',
                      hue='cols', style='framesize', size='meaning', size_order=['chosen', 'not chosen'])

    ax.set(xlabel='Dĺžka signálu [s]', ylabel='Kapacita')
    ax.set(yscale='log')
    ax.margins(x=0)

    plt.show()


def plot_capacity():
    SAMPLERATE = 44100
    DUR = 10
    samples = SAMPLERATE * DUR
    x = np.arange(512, 8192, 128)

    df = pd.DataFrame()
    df['framesize'] = pd.Series(x)
    df['tone'] = pd.Series(samples / x)
    # df['phase']=pd.Series(samples / x) # TODO
    df['echo'] = pd.Series(samples / x)
    df['echo-hc'] = pd.Series(samples / x * 4)
    df['lsb'] = samples

    df = df.melt(['framesize'], var_name='Metóda', value_name='capacity')

    dashmap = {'tone': (5, 5, 10, 0), 'echo': (10, 10),
               'lsb': '', 'echo-hc': '', 'phase': ''}
    ax = sns.lineplot(data=df, x='framesize', y='vals',
                      hue='Metóda', dashes=dashmap)
    ax.set(xlabel='Dĺžka úseku', ylabel='Kapacita v bitoch')
    ax.set(yscale='log')
    ax.margins(x=0)
    ticks = [2**x for x in np.arange(9, 14)]
    plt.xticks(ticks)
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


def plot_mos(save_fig):
    df = pd.read_csv(
        sys.argv[2],
        sep=",",
        decimal=".",
    )
    df = df.drop(df[df.method == 'none'].index)
    df.score = pd.to_numeric(df.score, errors='coerce')
    df['type'] = df.filename.str.split('-', n=1).str[0]
    df['type'] = df['type'].map(
        {'music': 'Hudba', 'noise': 'Zvuky', 'speech': 'Reč'})

    df['method'] = df['method'].map(METHOD_NAME_MAP)

    plt.figure(figsize=(6, 3))
    ax = sns.barplot(x='method', y='score', hue='type',
                     data=df, errorbar='sd', order=METHOD_ORDER)

    ax.set_ylim([1, 4])
    ax.get_yaxis().set_minor_locator(matplotlib.ticker.AutoMinorLocator())
    ax.grid(visible=True, which='major', linewidth=0.8, axis='y', aa=True)
    ax.grid(visible=True, which='minor', linewidth=0.3,
            axis='y', linestyle='--', aa=True)

    labels = [textwrap.fill(label.get_text(), 11)
              for label in ax.get_xticklabels()]
    ax.set_xticklabels(labels)

    ax.set(xlabel='Metóda', ylabel='Priemerné hodnotenie')
    sns.move_legend(ax, title='', loc='best')

    # g.set_xticklabels(["Men", "Women", "Children"])
    # g.despine(left=True)
    # g.set(ylim=(1, 4))
    plt.tight_layout()
    if (save_fig):
        plt.savefig('mos.pdf')
        plt.clf()
    else:
        plt.show()


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.stderr.write("Expected two arguments")
        sys.exit(1)

    df = load_data(sys.argv[1])

    if sys.argv[3] == 'params':
        plot_param_comparison(df)
    elif sys.argv[3] == 'cmp':
        df['method'] = df['method'].map(METHOD_NAME_MAP)

        mod_map = {'resampling': 'Prevzorkovanie', 'attenuation': 'Stlmenie',
                   'amplification': 'Zosilnenie', 'extraction': 'Extrakcia',
                   'requantization': 'Prekvantovanie', 'mp3-compress': 'MP3 kompresia',
                   'aac-compress': 'AAC kompresia'}
        df.rename(columns=mod_map, inplace=True)

        df['type'] = df['type'].map(
            {'music': 'Hudba', 'noise': 'Zvuky', 'speech': 'Reč'})

        save_fig = True
        sns.set_style('whitegrid')
        sns.set_context('paper')
        plot_used_capacity_dep_length(save_fig)
        # plot_snr(df, save_fig)
        plot_snr_by_type(df, save_fig)
        plot_mos(save_fig)
        plot_ber(df, save_fig)
        if False:
            for t in ('Hudba', 'Zvuky', 'Reč'):
                tdf = df.loc[df['type'] == t]
                plot_ber(tdf, save_fig, 'ber_' + t + '.pdf')

        #plot_ber_by_type(df, save_fig)
