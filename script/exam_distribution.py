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

#! /usr/bin/env python3

import sys
import os

if len(sys.argv) > 1:
  input = sys.argv[1]
else:
  print("Usage : " + sys.argv[0].split(os.sep)[-1] + " path/to/serials")
  exit(1)


import pandas as pd
import numpy as np

serials = pd.read_csv(input, skiprows=1, header=None, sep='\t', engine='python')
sorted_serials = [ list(row[1][2:len(row[1])-1]) for row in serials.iterrows()]
[ s.sort() for s in sorted_serials]

dist = []
cnt = 0
for i in range(len(sorted_serials)):
  for j in range(i+1,len(sorted_serials)):
    cnt += 1
    d = 0
    for a1, a2 in zip(sorted_serials[i],sorted_serials[j]):
      d += 0 if a1 == a2 else 1
    dist.append(d)

#print('tot',cnt,len(dist), int(len(sorted_serials)*(len(sorted_serials)-1)/2))

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
  for c, v in zip(cumul, bins):
    if c >= 0.5:
      median = 0.5*(v + v - 1)
      break
  sigma = np.sqrt(np.sum([ f*v**2 for f,v in zip(freq, bins)]) - mu**2)
  textstr = '\n'.join((
    r'$\mu=%.2f$' % (mu, ),
    r'$\mathrm{median}=%.2f$' % (median, ),
    r'$\sigma=%.2f$' % (sigma, )))
  props = dict(boxstyle='round', facecolor='wheat', alpha=1)
  ax2.text(0.05, 0.95, textstr, transform=ax.transAxes, fontsize=14,
        verticalalignment='top', bbox=props)

# dist histo
ranges=np.arange(0,32,1)
freq, bins = np.histogram(dist, bins=ranges, density=True)
cnt, bins = np.histogram(dist, bins=ranges)
cumul = np.cumsum(freq)
bins = np.array(bins[:-1])

# plot
import matplotlib.pyplot as plt
fig, ax1 = plt.subplots(1,1, figsize=(7, 5))
histoplot(freq, cnt, cumul, bins, ax1,
          title='Pair of exam \'distance\' distribution', xlabel='Number of repeated answers', ylabel='Fraction', y2label='Cumulative', color='#35a7ff')
plt.savefig(input.split('.')[-2] + '.png')
