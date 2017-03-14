'''
A quick python script to generate statistics on the 
sbj obj and verb data.

'''

import numpy as np
import sys
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


def stats(SBJ_OBJ, DICTIONARY, verbs_to_check, checked):

  values = []
  avg = 0
  high = -5000000
  low = 5000000
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
        if word_idx == len(DICTIONARY):
          unks += 1

      if ll < low:
        low = ll
      if ll > high:
        high = ll

  avg = avg / float(total)
  unks = unks / float(total)

  print("Dups?: ", str(dups))
  print("std-dev:", str(np.std(values)), "avg:", str(avg), "high:", str(high), "low:", str(low), "unks:", str(unks))

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]
  SBJ_OBJ_FILE = "verb_sbj_obj.txt"
  DICT_FILE = "dictionary.txt"
  SIM_FILE = "SimVerb-3000-test.txt"
  SBJ_OBJ = read_sbj_obj_file(BASE_DIR, SBJ_OBJ_FILE)
  DICTIONARY = read_dictionary(BASE_DIR, DICT_FILE)
  verbs_to_check, paired = read_sim_file(BASE_DIR, SIM_FILE)

  stats(SBJ_OBJ, DICTIONARY, verbs_to_check, False)
  stats(SBJ_OBJ, DICTIONARY, verbs_to_check, True)

