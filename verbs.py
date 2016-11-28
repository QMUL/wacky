#!/usr/bin/python

'''
Read in the subjects.txt file and the dictionary to create
a set of vectors for a verb, based on the subjects of these
verbs
'''

import numpy as np
import os, sys, math

BASE_DIR = ""
DICT_FILE = "dictionary.txt"
VERB_FILE = "subjects.txt"
VEC_FILE = ""
VEC_SIZE = 768

dictionary = []

unique_verbs = []

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
    ('talk', 'chat'),
    ('read', 'punch'),
    ('type', 'scrub'),
    ('program','scratch'),
    ('murder', 'love'),
    ('wander', 'burn')
  ]

# Read in the basic dictionary file
def read_dictionary() : 
  print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines()[1:]:
      dictionary.append( line.replace("\n",""))


# Read the subject file - each line is a verb subject list of numbers - indices into the dictionary
# First number is the verb, all following are the subjects

def read_subjects() :
  verb_add = {}
  verb_multiply = {}
  verb_sublength = {}

  for v in unique_verbs:
    verb_add[v] = np.zeros((1,VEC_SIZE))
    verb_multiply[v] = np.zeros((1,VEC_SIZE))

  print("Reading Subjects")
  with open(BASE_DIR + "/" + VERB_FILE, 'r') as f:
    for line in f.readlines():
      tokens = line.split(" ")
      if (len(tokens) > 2):
        verb = dictionary[int(tokens[0])]
        
        if verb in unique_verbs:
        
          #print(verb,":")
          add_vector = np.zeros((1,VEC_SIZE))
          mul_vector = np.ones((1,VEC_SIZE))
          sl = 0

          for sbj in tokens[1:]:
            try:
              sbj_idx = int(sbj)
              #print (" -", dictionary[sbj_idx])

              # Now find the subject vectors
              vv = vec_data[sbj_idx]
              add_vector = np.add(add_vector,vv)
              #print(np.linalg.norm(mul_vector))
              mul_vector = mul_vector * vv
              sl += 1
            except Exception as e:
              pass 
              # print("Exception occured",e)
              # Mostly just tokens not parsing properly

            #print (" ->", sub_vector)

          # Work out the cosine distances
          #print(add_vector)
          #print(mul_vector)
          verb_add[verb] = add_vector[0]
          verb_multiply[verb] = mul_vector[0]
          verb_sublength[verb] = sl

  return verb_add, verb_multiply, verb_sublength

# Determine the cosine distance
# https://en.wikipedia.org/wiki/Cosine_similarity#Angular_distance_and_similarity

def cosine_distance(verb0, verb1, verb_vec) :

  dist = 0
  if verb0 not in verb_vec:
    print(verb0,"not in vectors")
    return dist

  if verb1 not in verb_vec:
    print(verb1,"not in vectors")
    return dist

  v0 = verb_vec[verb0]
  v1 = verb_vec[verb1]
  v1 = v1.reshape((VEC_SIZE,1))
  
  n = np.dot(v0,v1)
  n0 = np.linalg.norm(v0)
  n1 = np.linalg.norm(v1)
  d = n0 * n1

  if (d != 0):
    sim = n / d
    dist = math.acos(sim) / math.pi
  
  return dist

# Return a vector divided by the number of subjects (an average basically)

def centroid (verb, verb_vec, verb_sl) : 
  return verb_vec[verb] / verb_sl[verb]

# Main function

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]

  # TODO - replace with a proper path
  vec_data = np.load("final_embeddings.npy")
  VEC_SIZE = vec_data.shape[1]

  for v in verbs_to_check:
    if v[0] not in unique_verbs:
      unique_verbs.append(v[0])
    if v[1] not in unique_verbs:
      unique_verbs.append(v[1])

  read_dictionary()

  verb_add, verb_mul, verb_sl = read_subjects()

  # Create our verb to verb vector dictionary
  verb_vrb = {}
   
  for idx in range(0,vec_data.shape[0]-1):
    v = dictionary[idx]
    if v in unique_verbs:
      verb_vrb[v] = vec_data[idx,:]

  for verb0, verb1 in verbs_to_check:
    if verb0 in dictionary and verb1 in dictionary:
      dist_vrb = cosine_distance(verb0, verb1, verb_vrb)
      dist_add = cosine_distance(verb0, verb1, verb_add)
      dist_mul = cosine_distance(verb0, verb1, verb_mul) 
      print ("-----------------")

      print (verb0, verb1, dist_vrb, dist_add, dist_mul, verb_sl[verb0], verb_sl[verb1])
      print ("centroid",verb0,"add")
      print (centroid(verb0, verb_add, verb_sl))
      print ("centroid",verb0,"mul")
      print (centroid(verb0, verb_mul, verb_sl))
      print ("centroid",verb1,"add")
      print (centroid(verb1, verb_add, verb_sl))
      print ("centroid",verb1,"mul")
      print (centroid(verb1, verb_mul, verb_sl))
      print ("cosine centroid", verb0,verb1, "add")
      
      tt = {}
      tt[verb0] = centroid(verb0,verb_add, verb_sl)
      tt[verb1] = centroid(verb1,verb_add,verb_sl)

      print (verb0, verb1, cosine_distance(verb0,verb1, tt))
    
      print ("cosine centroid", verb0,verb1, "mul")
      tt = {}
      tt[verb0] = centroid(verb0,verb_mul, verb_sl)
      tt[verb1] = centroid(verb1,verb_mul, verb_sl)

      print (verb0, verb1, cosine_distance(verb0,verb1, tt))
      print ("")
    else :
      print (verb0, "and/or", verb1, "not in dictionary")

