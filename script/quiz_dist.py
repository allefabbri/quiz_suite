#! /usr/bin/env python3

import sys

if len(sys.argv) > 1:
  input = sys.argv[1]
else:
  print("Usage : " + sys.argv[0].split('/')[-1] + " path/to/grades")
  exit(1)


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

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

# import data
input = sys.argv[1]
outcome = pd.read_csv(input, skiprows=9, header=None, sep='[\t]+', engine='python')
outcome.columns = ['serial', 'answers', 'results', 'score', 'amended_score', 'delta_score', 'grade_d', 'grade', 'surname', 'name']

### call outcome
# score
ranges=np.arange(-10,32,1)
s_freq, s_bins = np.histogram(outcome.score, bins=ranges, density=True)
s_cnt, s_bins = np.histogram(outcome.score, bins=ranges)
s_cumul = np.cumsum(s_freq)
s_bins = np.array(s_bins[:-1])

# grades
ranges=np.arange(0,32,1)
corrected_grade = outcome.grade + 2
g_freq, g_bins = np.histogram(corrected_grade, bins=ranges, density=True)
g_cnt, g_bins = np.histogram(corrected_grade, bins=ranges)
g_cumul = np.cumsum(g_freq)
g_bins = np.array(g_bins[:-1])

# dump
outcome['grade_corrected'] = corrected_grade
outcome[['surname', 'name', 'score', 'grade', 'grade_corrected']].to_csv(input.split('.')[-2] + '-resume.txt',
                                                                         index=False, sep='\t')

# plot
fig, (ax1, ax2) = plt.subplots(1,2, figsize=(12, 5))
histoplot(s_freq, s_cnt, s_cumul, s_bins, ax1,
          title='Score Distribution', xlabel='Score', ylabel='Fraction', y2label='Cumulative', color='#35a7ff')
histoplot(g_freq, g_cnt, g_cumul, g_bins, ax2,
          title='Grade Distribution', xlabel='Grade', ylabel='Fraction', y2label='Cumulative', color='green')
plt.subplots_adjust(left=0.07, wspace=0.4)
plt.savefig(input.split('.')[-2] + '.png')
plt.clf()

### answers
# values
ans_len = []
for ans in outcome.answers:
  ans_len.append(len(ans) - ans.count('-'))
ans_len = np.array(ans_len)
ranges=np.arange(0,32,1)
freq, bins = np.histogram(ans_len, bins=ranges, density=True)
cnt, bins = np.histogram(ans_len, bins=ranges)
cumul = np.cumsum(freq)
bins = np.array(bins[:-1])

# plot
fig, ax1 = plt.subplots(1,1, figsize=(7, 5))
histoplot(freq, cnt, cumul, bins, ax1,
          title='Number of answers distribution', xlabel='Number of answers', ylabel='Fraction', y2label='Cumulative', color='#35a7ff')
plt.savefig(input.split('.')[-2] + '-answers.png')
