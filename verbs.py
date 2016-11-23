#!/usr/bin/python

'''
Read in the subjects.txt file and the dictionary to create
a set of vectors for a verb, based on the subjects of these
verbs
'''

import numpy as np
import os, sys

BASE_DIR = ""
DICT_FILE = "dictionary.txt"
VERB_FILE = "subjects.txt"
VEC_FILE = ""
VEC_SIZE = 200

dictionary = []

verbs_to_check = [
    ('sleep', 'snooze'), 
    ('sleep', 'nap'), 
    ('sleep', 'doze'), 
    ('sleep', 'slumber'), 
    ('sleep', 'snore'),
    ('smile', 'laugh'), 
    ('smile', 'giggle'),
    ('walk', 'stroll'), 
    ('walk', 'march'),
    ('die', 'pass away'),
    ('disappear', 'vanish'), 
    ('disappear', 'fade'),
    ('shout', 'yell'),
    ('shout', 'cry'),
    ('freeze', 'melt'), 
    ('talk', 'chat')
  ]

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]

  # TODO - replace with a proper path
  vec_data = np.load("final_embeddings.npy")
  VEC_SIZE = vec.data.shape(1)

  unique_verbs = []
  for v in verbs:
    if v[0] not in unique_verbs:
      unique_verbs.append(v[0])
    if v[1] not in unique_verbs:
      unique_verbs.append(v[1])

  # Allocate our vectors give 
  verb_vectors = {}
  for v in unique_verbs:
    verb_vectors[v] = [i for i in range(0:VEC_SIZE)]

  print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines():
      dictionary.append( line.replace("\n",""))

  print("Reading Subjects")
  with open(BASE_DIR + "/" + VERB_FILE, 'r') as f:
    for line in f.readlines():
      tokens = line.split(" ")
      verb = dictionary[int(tokens[0])]
      
      print(verb,":")

      if verb in unique_verbs:
      
        sub_vector = np.zeros((1,VEC_SIZE))

        for sbj in tokens[1:]:
          sbj_idx = int(sbj)
          print (" -", dictionary[sbj_idx])

          # Now find the subject vectors
          vv = vec_data[sbj_idx]
          np.add(sub_vector,vv)

        print (" ->", sub_vector)



