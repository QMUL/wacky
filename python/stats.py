'''
A quick python script to generate statistics on the 
sbj obj and verb data.

'''

import numpy as np
import sys, math
from collections import Counter


def read_dictionary(BASE_DIR, DICT_FILE) :
  #print("Reading Dictionary")
  DICTIONARY = []
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines():
      DICTIONARY.append( line.replace("\n",""))

  return DICTIONARY

def read_sim_file(BASE_DIR, SIM_FILE):
  verbs_to_check_paired = []
  verbs_to_check = []

  with open(BASE_DIR + "/" + SIM_FILE,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      verbs_to_check_paired.append( (tokens[0], tokens[1], float(tokens[3])) )
      
      if tokens[0] not in verbs_to_check:
        verbs_to_check.append(tokens[0])

      if tokens[1] not in verbs_to_check:
        verbs_to_check.append(tokens[1])

  return verbs_to_check, verbs_to_check_paired

# Read the subject object pairs
def read_sbj_obj_file(BASE_DIR, SBJ_OBJ_FILE):

  SBJ_OBJ = {}

  with open(BASE_DIR + "/" + SBJ_OBJ_FILE, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      SBJ_OBJ[int(tokens[0])] = []
      if (len(tokens) > 1):
        for i in range(1,len(tokens),2):
          sbj_idx = int(tokens[i])
          obj_idx = int(tokens[i+1])
          SBJ_OBJ[int(tokens[0])].append( (sbj_idx, obj_idx) )  

  return SBJ_OBJ

# Read the subject or objecst of a verb
def read_sbj_file(BASE_DIR, SBJ_FILE):

  SBJS = {}

  with open(BASE_DIR + "/" + SBJ_FILE, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      SBJS[int(tokens[0])] = []
      if (len(tokens) > 1):
        for i in range(1,len(tokens)):
          sbj_idx = int(tokens[i])
          SBJS[int(tokens[0])].append( sbj_idx )  

  return SBJS



def counts(SBJS, OBJS, DICTIONARY, verbs_to_check):
     
  for vv in SBJS.keys():
    verb = DICTIONARY[vv]
    if verb in verbs_to_check:
      u = 0
      n = 0
      cc = Counter(SBJS[vv])
      for k in cc.keys():
        if cc[k] > 1:
          u += 1
        else:
          n += 1

      print("verb:",verb,"unique sbjs:",u,"non-unique sbjs:",n)

  for vv in OBJS.keys():
    verb = DICTIONARY[vv]
    if verb in verbs_to_check:
      u = 0
      n = 0
      cc = Counter(OBJS[vv])
      for k in cc.keys():
        if cc[k] > 1:
          u += 1
        else:
          n += 1

      print("verb:",verb,"unique objs:",u,"non-unique objs:",n)


def stats(SBJ_OBJ, DICTIONARY, verbs_to_check, checked):

  values = []
  avg = 0
  high = -5000000
  low = 5000000
  low_verb = ""
  high_verb = ""
  dups = False
  unks = 0
  total = 0

  # Basic count
  for verb in SBJ_OBJ.keys():
      
    if not checked or (checked and DICTIONARY[verb] in verbs_to_check):
      total += 1    
      ll = len(SBJ_OBJ[verb])
      avg += ll
      values.append(ll)
    
      if not dups:
        cc = Counter(SBJ_OBJ[verb])
        for vv in cc.values():
          if vv > 1:
            dups = True
            break

      for word_idx in SBJ_OBJ[verb]:
        if word_idx == len(DICTIONARY)-1:
          unks += 1

      if ll < low:
        low = ll
        low_verb = DICTIONARY[verb]
      if ll > high:
        high = ll
        high_verb = DICTIONARY[verb]


  avg = avg / float(total)
  unks = unks / float(total)

  values.sort()

  median = values[ int(math.floor(len(values) / 2))]
  ninefive = values[ int(math.floor(len(values) / 100 * 95)) ]

  print("Dups?: ", str(dups))
  print("std-dev:", str(np.std(values)), "avg:", str(avg), "high:", str(high), "low:", str(low), "unks:", str(unks), "low verb:", low_verb, "high verb:", high_verb, "median:", median, "95%:", ninefive)

  
  import matplotlib.pyplot as plt

  plt.hist(values, bins=1000)
  plt.show()

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]
  SBJ_OBJ_FILE = "verb_sbj_obj.txt"
  SBJ_FILE = "verb_subjects.txt"
  OBJ_FILE = "verb_objects.txt"
  DICT_FILE = "dictionary.txt"
  #SIM_FILE = "SimVerb-500-dev.txt"
  SIM_FILE = "SimVerb-3000-test.txt"
  SBJ_OBJ = read_sbj_obj_file(BASE_DIR, SBJ_OBJ_FILE)
  
  SBJS = read_sbj_file(BASE_DIR, SBJ_FILE)
  OBJS = read_sbj_file(BASE_DIR, OBJ_FILE)
  
  DICTIONARY = read_dictionary(BASE_DIR, DICT_FILE)
  verbs_to_check, paired = read_sim_file(BASE_DIR, SIM_FILE)

  stats(SBJ_OBJ, DICTIONARY, verbs_to_check, False)
  stats(SBJ_OBJ, DICTIONARY, verbs_to_check, True)

  #counts(SBJS, OBJS, DICTIONARY, verbs_to_check)
