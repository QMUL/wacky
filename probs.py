#!/usr/bin/python

'''
Perform the correlation and other such stats on our produced CSV files
Mostly used for the count vectors
'''

import numpy as np
import os, sys, math, struct, codecs

from scipy import stats

verb_pairs = [ 
    [("drink", "empty"), ("glass", "empty"), ("wine", "empty"), ("fill", "empty") ],
    [("drink", "full"), ("empty","full"), ("glass","full"), ("wine","full"), ("fill", "full")],
    [("find", "thief"), ("gold", "thief"), ("run-away", "thief"), ("flee", "thief")],
    [("find", "prospector"), ("gold", "prospector"), ("run-away", "prospector"), ("flee", "prospector")],
    [("find", "discoverer"), ("gold", "discoverer"), ("run-away", "discoverer"), ("flee", "discoverer")]
  ]


def read_dictionary(base_dir, dict_file) :
  #print("Reading Dictionary")
  dictionary = []
  with open(base_dir + "/" + dict_file,'r') as f:
    for line in f.readlines():
      dictionary.append( line.replace("\n",""))

  idx = 0
  rdictionary = {}
  for d in dictionary:
    rdictionary[d] = idx
    idx+=1

  return dictionary, rdictionary

# Read the number of unknowns we have
def read_unk_count(unk_file):
  l = 0
  with open(unk_file, 'r') as f:
    l = int(f.readlines()[0])
  return l

# Read the total data count file
def read_total_size(total_file):
  total_size = 0
  with open(total_file,'r') as f:
    total_size = int(f.readlines()[0])

  return total_size

def read_freq(freq_file):
  count = {}
  order = []

  with open(freq_file,'r') as f:
    for line in f.readlines():      
      tokens = line.split(", ")

      if len(tokens) == 2:
        key = tokens[0].replace(" ","")
        freq  = int(tokens[1])
        count[key] = freq

  count['UNK'] =-1

  count_order = [pair[0] for pair in sorted(count.items(), key=lambda item: item[1])]
  count_order = count_order[::-1]

  return count, count_order

# Read the count vectors but do so just for the words we are interested in
def read_count_vectors(vector_file, dictionary, rdictionary, word_list):
  
  vectors = {}
  indices = []
  for word in word_list:
    if word in dictionary:
      indices.append(rdictionary[word])
    else:
      print (word,"is not in dictionary")


  with open(vector_file,'r') as f:
    idx = 0
    for line in f.readlines():
      if idx in indices:
        tokens = line.split()
        vectors[dictionary[idx]] = []

        for token in tokens:
          vectors[dictionary[idx]].append(float(token)) 

      idx +=1
      
  return vectors

# Read in the numbers that form the basis
def read_basis(basis_file):
  basis = []
  with open(basis_file,'r') as f:
    for line in f.readlines():
      basis.append(int(line))

  return basis


# Convert count vectors to the log based probability
def probs(vectors, dictionary, rdictionary, freq, basis, total):

  vl = {}

  for word in vectors.keys():
    vv = vectors[word]
    vl[word] = []
 
    idx = 0
    for tf in vv:
    
      vl[word].append(0)
      ct = freq[dictionary[idx]]
      pmi = 0
      
      if ct != 0.0:
        cc = freq[word]
        
        if cc != 0.0:
          cct = tf

          if cct != 0.0:
            pmi = math.log( (cct/ct) / (cc / total))
            vl[word][-1] = pmi

      idx+=1

  return vl

def words_to_check():

  vv = []
  for ll in verb_pairs:
    for vp in ll:
      if vp[0] not in vv:
        vv.append(vp[0])
      if vp[1] not in vv:
        vv.append(vp[1])

  return vv

# Where does a word sit in the basis?
def basis_position(basis, rdictionary, word):
  if rdictionary[word] in basis:
    for i in range(0,len(basis)):
      if basis[i] == rdictionary[word]:
        return i

  return -1

if __name__ == "__main__" :
  base_dir = sys.argv[1]
  
  vector_file = "word_vectors.txt"
  if len(sys.argv) > 2:
    if sys.argv[2] == "-w":
      vector_file = "vectors.bin"
    elif sys.argv[2] == "-t":
      vector_file = "final_standard_embeddings.npy"  
  
  dict_file = "dictionary.txt"
  unk_file = "unk_count.txt"
  total_file = "total_count.txt"
  freq_file = "freq.txt"
  basis_file = "basis.txt"

  word_list = words_to_check()
  unk_count = read_unk_count(base_dir + "/" + unk_file)
  total_count = read_total_size(base_dir + "/" + total_file)
  basis = read_basis(base_dir + "/" + basis_file)

  dictionary, rdictionary = read_dictionary(base_dir,  dict_file)
  freq, freq_order = read_freq(base_dir + "/" + freq_file)
  
  vectors = read_count_vectors(base_dir + "/" + vector_file, dictionary, rdictionary, word_list)

  # Check which words are not in our basis
  for word in word_list:
    if word in dictionary:
      if rdictionary[word] not in basis:
        print(word,"not in basis")
    else:
      print(word,"not in basis or dict")

  #for v in vectors.keys():
  #  print(v, vectors[v])

  vector_probs = probs(vectors, dictionary, rdictionary, freq, basis, total_count)
  
  for ss in verb_pairs:
    for pair in ss:
      bp = basis_position(basis, rdictionary, pair[1])
      if pair[0] in vector_probs.keys():
        if bp != -1:
          print (pair, vector_probs[pair[0]][bp])

