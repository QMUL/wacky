import numpy as np
import os, sys

BASE_DIR = "./build"
DICT_FILE = "dictionary.txt"

dictionary = []

def read_dictionary() : 
  print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines()[1:]:
      dictionary.append( line.replace("\n",""))


if __name__ == "__main__" :

  data = np.load("final_embeddings.npy")

  if len(sys.argv) > 1:
    BASE_DIR = sys.argv[1]

  read_dictionary()

  print("Data sizes: ", data.size, data.shape, data.ndim)
  subset = data[500:512]
  print(subset)
  print(dictionary[1])
