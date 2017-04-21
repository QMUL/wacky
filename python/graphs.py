#!/usr/bin/python

'''

Create some graphs of the final results. We use a little matplotlib

python3 graphs.py path/to/csv.file

'''

import numpy as np
import os, sys, math, struct, codecs

from scipy import stats

import matplotlib.pyplot as plt


def read_csv_file(filepath): 
  ''' Read the CSV file of results we are looking to generate stats for '''

  with open(filepath,'r') as f:

    header = f.readline()

    titles = header.split(",")

    human = []
    cc = []
    num_compares = len(titles) - 3
    verbs0 = []
    verbs1 = []

    titles = titles[2:len(titles)-1]

    for i in range(0,num_compares):
      cc.append([])

    for line in f.readlines():
      tokens = line.split(",")
      verb0 = tokens[0]
      verb1 = tokens[1]

      # check to see if there any mistakes
      # Basically, are any of our factors 2.0?
  
      #for i in range(0,num_compares):
      for i in range(0,num_compares): 
        tt = float(tokens[2+i])
        if tt >= 2.0 or 'nan' in tokens[2+i] or tt == 1.0:
          tt = 0
        cc[i].append(tt)
      
      human.append(float(tokens[-1]))

      verbs0.append(verb0)
      verbs1.append(verb1)

  return cc, human, titles, verbs0, verbs1



if __name__ == "__main__" :
  
  cc, human, titles, v0, v1 = read_csv_file(sys.argv[1] )

  # Scatter plot of models against human
  
  fig, ax = plt.subplots()

  scale = 10
  
  ax.scatter(human, cc[0], c='blue', s=scale, label='base', alpha=0.2, edgecolors='none')
  ax.scatter(human, cc[1], c='red', s=scale, label='cs1', alpha=0.2, edgecolors='none')
  ax.scatter(human, cc[2], c='green', s=scale, label='cs2', alpha=0.2, edgecolors='none')
  ax.scatter(human, cc[3], c='purple', s=scale, label='cs3', alpha=0.2, edgecolors='none')
  ax.scatter(human, cc[4], c='yellow', s=scale, label='cs4', alpha=0.2, edgecolors='none')
  ax.scatter(human, cc[5], c='orange', s=scale, label='cs5', alpha=0.2, edgecolors='none')
  ax.scatter(human, cc[6], c='black', s=scale, label='cs6', alpha=0.2, edgecolors='none')

  # Trend lines
  
  r = range(0,10)
  z = np.polyfit(human, cc[0], 1)
  p = np.poly1d(z)
  ax.plot(r,p(r), c='blue')
 
  z = np.polyfit(human, cc[1], 1)
  p = np.poly1d(z)
  plt.plot(r,p(r), c='red')
  
  z = np.polyfit(human, cc[2], 1)
  p = np.poly1d(z)
  plt.plot(r,p(r), c='green')
 
  z = np.polyfit(human, cc[3], 1)
  p = np.poly1d(z)
  plt.plot(r,p(r), c='purple')
  
  z = np.polyfit(human, cc[4], 1)
  p = np.poly1d(z)
  plt.plot(r,p(r), c='yellow')
  
  z = np.polyfit(human, cc[5], 1)
  p = np.poly1d(z)
  plt.plot(r,p(r), c='orange')
  
  z = np.polyfit(human, cc[6], 1)
  p = np.poly1d(z)
  plt.plot(r,p(r), c='black')

  print ("y=%.6fx+(%.6f)"%(z[0],z[1]))

  ax.legend()
  ax.grid(True)

  plt.show()

