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

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", help="grades file", required=True)
args = parser.parse_args()
grade_file = args.input

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

    label = "{:d}".format(int(c))

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

def histo2dplot(freq, cnt, binx, biny, ax, xlabel, ylabel):

#  subx=5
#  ax.set_xticks(binx[::subx])
#  ax.set_xticklabels(binx[::subx])
  suby=5
  ax.set_yticks(biny[::suby])
  ax.set_yticklabels(biny[::suby][::-1])
  ax.set_xlabel(xlabel)
  ax.set_ylabel(ylabel)
  ax.imshow(np.flipud(c))#, interpolation='nearest')

  return ax

# import data
outcome = pd.read_csv(grade_file, skiprows=9, header=None, sep='[\t]+', engine='python')
outcome.columns = ['serial', 'answers', 'results', 'score', 'amended_score', 'delta_score', 'grade_d', 'grade', 'surname', 'name']
print(outcome.head)
### answers
# values
answers = { 'exact' : [], 'wrong' : [], 'blank' : [], 'tot' : [] }
for i, vals in outcome[['answers','results']].iterrows():
  #print(i, vals, vals['answers'], vals['results'])
  e = 0
  w = 0
  b = 0
  for k in range(len(vals['answers'])):
    if vals['answers'][k] == '-':
      b += 1
    elif vals['answers'][k] == vals['results'][k]:
      e += 1
    elif vals['answers'][k] != vals['results'][k]:
      w += 1
  t = e + w
  answers['exact'].append(e)
  answers['wrong'].append(w)
  answers['blank'].append(b)
  answers['tot'].append(t)

#print(answers)
ranges=np.arange(0,32,1)
k2=['grade']
freq = {}
cnt = {}
for k in answers:
 freq[k] = {}
 cnt[k] = {}

for i in answers:
  for j in k2:
    f, binx, biny = np.histogram2d(answers[i], outcome[j], ranges, density=True)
    c, binx, biny = np.histogram2d(answers[i], outcome[j], ranges)
    freq[i][j] = f
    cnt[i][j] = c

import itertools
fig, ((ax11, ax21), (ax12, ax22)) = plt.subplots(2, 2, figsize=(8,8))
for j in k2:
  for i, ax in list(itertools.product(answers, [ax11, ax21, ax12, ax22])):
  #for j, ax in zip(k2, [ax11, ax21, ax12, ax22]):
    histo2dplot(freq[i][j], cnt[i][j], binx, biny, ax=ax,
              xlabel='Number of ' + i + ' answers', ylabel='Value of ' + j)
  plt.subplots_adjust(hspace=0.45, wspace=0.45)
  plt.savefig(grade_file.split('.')[-2] + '-joint_' + i + '_answers.png')
plt.clf()


##########################
# plot
freq = {}
cnt = {}
cumul = {}
for at in answers:
#  print(at)
  freq[at], bins = np.histogram(answers[at], bins=ranges, density=True)
  cnt[at], bins = np.histogram(answers[at], bins=ranges)
  cumul[at] = np.cumsum(freq[at])
bins = np.array(bins[:-1]) # for plotting purpose

params = {
  'xlabel' : {
    'exact' : 'Number of exact answers',
    'wrong' : 'Number of wrong answers',
    'blank' : 'Number of blank answers',
    'tot' : 'Number of tot answers'
  },
  'color' : {
    'exact' : 'green',
    'wrong' : 'red',
    'blank' : 'gray',
    'tot' : 'blue'
  }
}
fig, ((ax11, ax12), (ax21, ax22)) = plt.subplots(2,2, figsize=(8, 8))
for at, ax in zip(answers, [ax11, ax12, ax21, ax22]):
  histoplot(freq[at], cnt[at], cumul[at], bins, ax,
          title='Distributions', xlabel=params['xlabel'][at], ylabel='Fraction', y2label='Cumulative', color=params['color'][at])
plt.subplots_adjust(hspace=0.35, wspace=0.45)
plt.savefig(grade_file.split('.')[-2] + '-2d.png')






