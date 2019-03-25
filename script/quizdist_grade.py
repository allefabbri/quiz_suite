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
ans_len = []
for ans in outcome.answers:
  ans_len.append(len(ans) - ans.count('-'))
ans_len = np.array(ans_len)
###
corrected_grade = outcome.grade + 2
###

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

ranges=np.arange(-10,32,1)
(s_freq, s_cnt, s_cumul, s_bins, s_mu, s_median, s_sigma) = computestats(outcome.score, ranges)
ranges=np.arange(0,32,1)
(g_freq, g_cnt, g_cumul, g_bins, g_mu, g_median, g_sigma) = computestats(corrected_grade, ranges)
(c_freq, c_cnt, c_cumul, c_bins, c_mu, c_median, c_sigma) = computestats(outcome.grade, ranges)
(a_freq, a_cnt, a_cumul, a_bins, a_mu, a_median, a_sigma) = computestats(ans_len, ranges)
ranges=np.arange(0,10,1)
(b_freq, b_cnt, b_cumul, b_bins, b_mu, b_median, b_sigma) = computestats(outcome.delta_score, ranges)

### dump
outcome['grade_corrected'] = corrected_grade
outcome[['surname', 'name', 'score', 'grade', 'grade_corrected']].to_csv(input.split('.')[-2] + '-resume.txt',
                                                                         index=False, sep='\t')
# plot
fig, axis = plt.subplots(2,2, figsize=(10, 8))
histoplot(a_freq, a_cnt, a_cumul, a_bins, axis[0,0],
          title='Number of answers distribution', xlabel='Number of answers', ylabel='Fraction', y2label='Cumulative', color='#35a7ff')
histoplot(g_freq, g_cnt, g_cumul, g_bins, axis[0,1],
          title='Grade Distribution', xlabel='Grade', ylabel='Fraction', y2label='Cumulative', color='green')
histoplot(b_freq, b_cnt, b_cumul, b_bins, axis[1,0],
          title='Bugs Distribution', xlabel='Number of bugs', ylabel='Fraction', y2label='Cumulative', color='red')
# final stats report
textstr = '\n'.join((
  r'GRADE avg   = %.2f' % (g_mu, ),
  r'GRADE med   = %.2f' % (g_median, ),
  r'GRADE sigma = %.2f' % (g_sigma, ),
  r'',
  r'ANS avg   = %.2f' % (a_mu, ),
  r'ANS med   = %.2f' % (a_median, ),
  r'ANS sigma = %.2f' % (a_sigma, ),
  r'',
  r'BUGS avg   = %.2f' % (b_mu, ),
  r'BUGS med   = %.2f' % (b_median, ),
  r'BUGS sigma = %.2f' % (b_sigma, )
))
axis[1,1].axis('off')
axis[1,1].text(0.0, 1.0, textstr, transform=axis[1,1].transAxes, fontfamily='monospace', fontsize=14, verticalalignment='top')
#
plt.subplots_adjust(left=0.1, wspace=0.4, hspace=0.3)
plt.savefig(input.split('.')[-2] + '.png')
plt.clf()
