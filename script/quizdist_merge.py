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

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import argparse

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
  ax2.grid()
  ax2.plot(bins, cumul, '-o', color='#f9a22f')
  ax2.set_ylabel(y2label)

### stats
def computestats(data, ranges):
  freq, bins = np.histogram(data, bins=ranges, density=True)
  cnt, bins = np.histogram(data, bins=ranges)
  cumul = np.cumsum(freq)
  bins = np.array(bins[:-1])
  mu = np.sum([ f*v for f,v in zip(freq, bins)])
  for c, v in zip(cumul, bins):
    if c >= 0.5:
      median = 0.5*(v + v - 1)
      break
  sigma = np.sqrt(np.sum([ f*v**2 for f,v in zip(freq, bins)]) - mu**2)
  return (freq, cnt, cumul, bins, mu, median, sigma)

### parse cli
parser = argparse.ArgumentParser()
parser.add_argument("-i1", "--input1", help="first grades file", required=True)
parser.add_argument("-i2", "--input2", help="second grades file", required=True)
parser.add_argument("-b1", "--bonus1", help="first flat bonus", default=0)
parser.add_argument("-b2", "--bonus2", help="second flat bonus", default=0)
#parser.add_argument("-t", "--threshold", help="auto threshold", default=18)
args = parser.parse_args()
grade_file1 = args.input1
grade_file2 = args.input2
bonus1 = float(args.bonus1)
bonus2 = float(args.bonus2)
#thresh = int(args.threshold)

### import data
outcome1 = pd.read_csv(grade_file1, skiprows=9, header=None, sep='[\t]+', engine='python')
outcome1.columns = ['serial', 'answers', 'results', 'score', 'amended_score', 'delta_score', 'grade_d', 'grade', 'surname', 'name']
outcome1 = outcome1.astype({ 'grade' : 'float' })
outcome1.grade += bonus1
outcome1.grade.values[ outcome1.grade.values > 30 ] = 30

outcome2 = pd.read_csv(grade_file2, skiprows=9, header=None, sep='[\t]+', engine='python')
outcome2.columns = ['serial', 'answers', 'results', 'score', 'amended_score', 'delta_score', 'grade_d', 'grade', 'surname', 'name']
outcome2 = outcome2.astype({ 'grade' : 'float' })
outcome2.grade += bonus2
outcome2.grade.values[ outcome2.grade.values > 30 ] = 30

total = pd.merge(
  outcome1[['surname', 'name', 'grade']],
  outcome2[['surname', 'name', 'grade']],
  on=['surname', 'name'],
  how='inner'
)
total['final_d'] = 0.5 * (total['grade_x'] + total['grade_y'])
total['final'] = total['final_d'] + 0.5001
total = total.astype({ 'final' : 'int' })
total.to_csv('total-present.csv', sep='\t', index=None, header=True)


print('Student exam1 : {}'.format(outcome1.shape))
print('Student exam2 : {}'.format(outcome2.shape))
print('Student total : {}'.format(total.shape))

diff1=pd.concat([
  outcome1[['surname', 'name']],
  total[['surname', 'name']]
])
diff1 = diff1.drop_duplicates(keep=False)
diff1['exam'] = 1

diff2=pd.concat([
  outcome2[['surname', 'name']],
  total[['surname', 'name']]
])
diff2 = diff2.drop_duplicates(keep=False)
diff2['exam'] = 2

print('Student diff1 : {}'.format(diff1.shape))
print('Student diff2 : {}'.format(diff2.shape))

missing = pd.concat([ diff1, diff2 ], sort=False)
missing = missing.sort_values('surname')
missing.to_csv('total-missing.csv', sep='\t', index=None, header=True)

### stats
ranges=np.arange(0,32,1)
(freq_1, cnt_1, cumul_1, bins_1, mu_1, median_1, sigma_1) = computestats(outcome1.grade, ranges)
(freq_2, cnt_2, cumul_2, bins_2, mu_2, median_2, sigma_2) = computestats(outcome2.grade, ranges)
(freq_t, cnt_t, cumul_t, bins_t, mu_t, median_t, sigma_t) = computestats(total.final, ranges)

### plot
fig, axis = plt.subplots(2,2, figsize=(10, 8))
histoplot(freq_1, cnt_1, cumul_1, bins_1, axis[0,0], title='Grade exam 1 bonus {}'.format(bonus1), xlabel='Grade', ylabel='Fraction', y2label='Cumulative', color='blue')
histoplot(freq_2, cnt_2, cumul_2, bins_2, axis[0,1], title='Grade exam 2 bonus {}'.format(bonus2), xlabel='Grade', ylabel='Fraction', y2label='Cumulative', color='green')
histoplot(freq_t, cnt_t, cumul_t, bins_t, axis[1,0], title='Final grade', xlabel='Grade', ylabel='Fraction', y2label='Cumulative', color='red')

textstr = '\n'.join((
  'exam1 avg    = {:.2f}'.format(mu_1),
  'exam2 avg    = {:.2f}'.format(mu_2),
  'final avg    = {:.2f}'.format(mu_t),
  '',
  *['final auto = {:.1f}% @ {}'.format(100*(1-cumul_t[thresh]), thresh) for thresh in [18,19,20,21]]
))
axis[1,1].axis('off')
axis[1,1].text(0.0, 1.0, textstr, transform=axis[1,1].transAxes, fontfamily='monospace', fontsize=14, verticalalignment='top')
#
plt.subplots_adjust(left=0.1, wspace=0.4, hspace=0.3)
plt.savefig('total-present.png')
plt.clf()
