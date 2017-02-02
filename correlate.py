#!/usr/bin/python

'''
Perform the correlation and other such stats on our produced CSV files
Mostly used for the count vectors
'''

import numpy as np
import os, sys, math, struct, codecs

from scipy import stats


def read_csv_file(filepath): 
  with open(filepath,'r') as f:

    header = f.readline()

    titles = header.split(",")

    human = []
    cc = []
    num_compares = len(titles) - 3

    titles = titles[2:len(titles)-2]

    for i in range(0,num_compares):
      cc.append([])

    for line in f.readlines():
      tokens = line.split(",")
      verb0 = tokens[0]
      verb1 = tokens[1]

      # check to see if there any mistakes
      # Basically, are any of our factors 2.0?
      for i in range(0,num_compares-1):
        if float(tokens[2+i]) >= 2.0:
          continue

      for i in range(0,num_compares):
        cc[i].append(float(tokens[2+i]))
    
      human.append(float(tokens[-1]))

  return cc, human, titles


if __name__ == "__main__" :
  cc, human, titles = read_csv_file(sys.argv[1])
  
  rho = []

  for title in titles:
    sys.stdout.write("rho_" + title + ",")

  print("")
  
  for comp in cc:
    rho.append(stats.spearmanr(comp,human))

  
  for r,pval in rho:
    sys.stdout.write(str(r) + ",")

  print("")

  for title in titles:
    sys.stdout.write("p_" + title + ",")

  print("")
  
  for r,pval in rho:
    sys.stdout.write(str(pval) + ",")
  print("")


