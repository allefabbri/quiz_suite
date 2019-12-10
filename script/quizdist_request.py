#! /usr/bin/env python3

"""
// Copyright 2015, Alessandro Fabbri

/************************************************************************
* This program is free software: you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>. *
************************************************************************/
"""

import pandas as pd
import numpy as np
import argparse
import re

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", help="request output file", required=True)
args = parser.parse_args()
req_file = args.input

tags = [ 'VOTO', 'CORRETTE' ]
data = {}
with open(req_file) as f:
  for line in f.readlines():
    for tag in tags:
      try:
        data.setdefault(tag, []).append(float(re.search(tag + '[ ]+: (.*?)\n', line).group(1)))
      except AttributeError:
        pass #print('no match')

def histoplot(freq, cnt, cumul, bins, ax, title, xlabel, ylabel, y2label, color):
  ax.bar(bins, freq, color=color)
  ax.set_title(title)
  ax.set_xlabel(xlabel)
  ax.set_ylabel(ylabel)
  ax.set_xlim([bins[0] - 1.5, bins[-1] + 1.5])
  ax.set_ylim([0, max(freq)*1.2])

  rects = ax.patches
  for rect, c in zip(rects,cnt):
    y_value = rect.get_height()
    x_value = rect.get_x() + rect.get_width() / 2
    space = 5
    va = 'bottom'

    if y_value == 0:
      continue
    elif y_value < 0:
      space *= -1
      va = 'top'

    label = "{:d}".format(c)

    ax.annotate(
      label,
      (x_value, y_value),
      xytext=(0, space),
      textcoords="offset points",
      ha='center',
      va=va)

  ax2 = ax.twinx()
  ax2.set_axisbelow(True)
  ax2.grid()
  ax2.plot(bins, cumul, '-o', color='#f9a22f')
  ax2.set_ylabel(y2label)

  mu = np.sum([ f*v for f,v in zip(freq, bins)])
  median_idx = next(x for x, val in enumerate(cumul) if val > 0.5)
  median = 0.5*(bins[median_idx] + bins[median_idx-1])
  perc25_idx = next(x for x, val in enumerate(cumul) if val > 0.25)
  perc25 = 0.5*(bins[perc25_idx] + bins[perc25_idx-1])
  perc90_idx = next(x for x, val in enumerate(cumul) if val > 0.90)
  perc90 = 0.5*(bins[perc90_idx] + bins[perc90_idx-1])

  sigma = np.sqrt(np.sum([ f*v**2 for f,v in zip(freq, bins)]) - mu**2)
  textstr = '\n'.join([
    r'$\mu=%.2f$' % (mu, ),
    r'$\sigma=%.2f$' % (sigma, ),
    r'$\mathrm{25 perc}=%.2f$' % (perc25),
    r'$\mathrm{median}=%.2f$' % (median),
    r'$\mathrm{90 perc}=%.2f$' % (perc90)
  ])
  props = dict(boxstyle='round', facecolor='wheat', alpha=1)
  ax2.text(0.05, 0.95, textstr, transform=ax.transAxes, fontsize=14,
        verticalalignment='top', bbox=props)

# dist histo
ranges=np.arange(0,32,1)
freq, bins = np.histogram(data['VOTO'], bins=ranges, density=True)
cnt, bins = np.histogram(data['VOTO'], bins=ranges)
cumul = np.cumsum(freq)
bins = np.array(bins[:-1])

# plot
import matplotlib.pyplot as plt
fig, ax1 = plt.subplots(1,1, figsize=(7, 5))
histoplot(freq, cnt, cumul, bins, ax1,
          title='Grade distribution of request', xlabel='Grade', ylabel='Fraction', y2label='Cumulative', color='#35a7ff')
plt.savefig(req_file.split('.')[-2] + '.png')
