import numpy as np
import os, sys, math

from random import randint

from data_buffer import read_freq

BASE_DIR = "./build"
DICT_FILE = "dictionary.txt"
FREQ_FILE = "freq.txt"

dictionary = []

def read_dictionary() : 
  print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines()[1:]:
      dictionary.append( line.replace("\n",""))


if __name__ == "__main__" :

  if len(sys.argv) > 1:
    BASE_DIR = sys.argv[1]

  data = np.load(BASE_DIR + "/final_standard_embeddings.npy")

  read_dictionary()

  count, count_order = read_freq(BASE_DIR + "/" + FREQ_FILE, len(dictionary))
  
  for i in range(0,200):
    print(count_order[i])


  print("Data sizes: ", data.size, data.shape, data.ndim)
  subset = data[500:512]
  print(subset)
  ri = randint(0,len(dictionary)-1)
  print(dictionary[ri])
  vv = 0
  for i in data[ri]:
    vv += i*i

  vv = math.sqrt(vv)
  print("Vector Length",vv)

  data = np.load(BASE_DIR + "/final_normalized_embeddings.npy")
  vv = 0
  for i in data[ri]:
    vv += i*i

  vv = math.sqrt(vv)
  print("Normal Vector Length",vv)


