import numpy as np
import os, sys, math

from random import randint

BASE_DIR = "./build"
DICT_FILE = "dictionary.txt"
VEC_SIZE = 256

dictionary = []
dictionary_fast = {}

verbs_to_check = [
  'sleep',
  'snooze',
  'nap',
  'doze',
  'slumber',
  'snore',
  'laugh',
  'giggle',
  'stroll',
  'walk',
  'march',
  'die',
  'pass away',
  'disappear',
  'vanish',
  'fade',
  'shout',
  'yell',
  'cry',
  'freeze',
  'melt',
  'talk',
  'chat'
]


def read_dictionary() : 
  print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    idx = 0
    for line in f.readlines()[1:]:
      word = line.replace("\n","")
      dictionary.append(word)
      dictionary_fast[word] = idx
      idx += 1
    

def cosine_distance(word0, word1, data_vec) :
  global VEC_SIZE

  v0 = data_vec[word0]
  v1 = data_vec[word1]
  v1 = v1.reshape((VEC_SIZE,1))
  
  n = np.dot(v0, v1)[0]
  n0 = np.linalg.norm(v0)
  n1 = np.linalg.norm(v1)
  d = n0 * n1
  
  sim = n / d
  return 1.0 - (math.acos(sim) / math.pi)

  # if vectors are always positive use the one below
  #dist = 1.0 - ((2.0 * math.acos(sim)) / math.pi)
  
  

def word_to_index(word) :
  if word in dictionary_fast.keys():
    return dictionary_fast[word]

  return -1


if __name__ == "__main__" :

  compare_words = []

  if len(sys.argv) > 1:
    BASE_DIR = sys.argv[1]
  
  if len(sys.argv) > 2:
    verbs_to_check = [sys.argv[2]]
 
  if len(sys.argv) > 3:
    for i in range(3,len(sys.argv)):
      compare_words.append(sys.argv[i])

  data = np.load(BASE_DIR + "/final_standard_embeddings.npy")

  read_dictionary()

  print("Data sizes: ", data.size, data.shape, data.ndim)

  VEC_SIZE = data.shape[1]

  words_against = dictionary
  if len(compare_words) > 0:
    words_against = compare_words

  for verb in verbs_to_check:
    top_ten = []
    lower_bound = -1.0
  
    vi = word_to_index(verb)

    if vi != -1:
      for word in words_against:
        if word == verb:
          continue
        wi = word_to_index(word)
        cc = cosine_distance(vi, wi, data) 
        if len(top_ten) == 10:
          if cc > lower_bound:
            top_ten.append((word,cc))
            top_ten.sort(key=lambda tup: tup[1])
            top_ten = top_ten[::-1]
            top_ten = top_ten[:10]
            lower_bound = top_ten[9][1]
        else:
          top_ten.append( (word,cc) )

      print("------------")
      print(verb)
      for (word,cc) in top_ten:
        print (word,cc)
      print("")
