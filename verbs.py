#!/usr/bin/python

'''
Read in the subjects.txt file and the dictionary to create
a set of vectors for a verb, based on the subjects of these
verbs
'''

import numpy as np
import os, sys, math, struct

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
    ('wander', 'burn'),
    ('read', 'write'),
    ('read', 're-read'),
    ('read', 'see')
  ]
'''
verbs_to_check = [
    ('sleep', 'snooze'), 
  ] 
'''
# Read in the basic dictionary file
def read_dictionary() :
  global dictionary
  print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines()[1:]:
      dictionary.append( line.replace("\n",""))


# Read the subject file - each line is a verb subject list of numbers - indices into the dictionary
# First number is the verb, all following are the subjects

def read_subjects() :
  global dictionary
  global vec_data
  global VEC_SIZE

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
        
        print(verb)
        
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
              #print("Exception occured",e)
              # Mostly just tokens not parsing properly

            #print (" ->", sub_vector)
          if verb == "giggle":
            print(verb,sl,add_vector[0],mul_vector[0])
            
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
  
  n = np.dot(v0, v1)[0]
  n0 = np.linalg.norm(v0)
  n1 = np.linalg.norm(v1)
  d = n0 * n1

  #print ("n over d",n,d,n/d)

  if (d != 0):
    sim = n / d
    dist = math.acos(sim) / math.pi
  
  return 1.0 - dist

# Return a vector divided by the number of subjects (an average basically)

def centroid (verb, verb_vec, verb_sl) :
  return verb_vec[verb] / float(verb_sl[verb])

# Read the binary file from word2vec, returning a numpy array
# we use the dictionary created by tensorflow, because all the integer
# files, dictionarys and verb/subject lookups are based on that/

def read_binary(filepath):

  vectors = []
  dlookup = {}
  global dictionary
  global VEC_SIZE
  
  # As the tensorflow dict is different to the word2vec
  # create a quick lookup table
  for i in range(0,len(dictionary)):
    dlookup[dictionary[i]] = i

  with open(filepath,'rb') as f:
    nums = []
    byte = f.read(1)
    char = byte.decode("ascii")
    nums.append(char)
    
    while (char != '\n'):
      byte = f.read(1)
      char = byte.decode("ascii")
      nums.append(char)

    tt = ''.join(nums).split(" ") 
    words = int( tt[0] ) # long long int?
    VEC_SIZE = int( tt[1] )

    for i in range(0,len(dictionary)):
      vectors.append( np.zeros(VEC_SIZE) )

    print("Loading vectors.bin with", words, "words with", VEC_SIZE, "vec size")

    # TODO - for some reason this explodes memory (the numbers bit at the end) even though
    # the C version copes fine. I suspect we must be duplicating something? 
    # regardless we need to figure out why that is or restrict to top 50,000 words :/

    widx = 0
    while(widx < words):
    
      word = ""
      while (True):
        byte = f.read(1)
        try:
          char = byte.decode("ascii")
        
          if char == " ":
           break
          word += char
        except :
          pass

      word = word.replace("\n","")
      #dictionary.append(word)
      
      # Lookup the word using the dictonary we generated with tensorflow
      pidx = -1
      if word in dlookup.keys():
        pidx = dlookup[word]

      # Now read the numbers, should be VEC_SIZE of them (4 bytes probably)
      tidx = 0
      for i in range(0,VEC_SIZE):
        byte = f.read(4)
        num = struct.unpack('f',byte)

        if pidx != -1: 
          vectors[pidx].itemset(tidx, num[0])

        tidx += 1
      
      widx += 1

  vectors = np.vstack(vectors)
  print(vectors.shape)
  #return dictionary, vectors
  return vectors

# Main function

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]

  binversion = False
  if len(sys.argv) > 2:
    if sys.argv[2] == '-b':
      binversion = True

  vec_data = []
  read_dictionary()

  if binversion:
    vec_data = read_binary(BASE_DIR + "/vectors.bin")
  else:
    # TODO - replace with a proper path
    vec_data = np.load(BASE_DIR + "/final_embeddings.npy")
    VEC_SIZE = vec_data.shape[1]
    print("Vec Size", VEC_SIZE)

  for v in verbs_to_check:
    if v[0] not in unique_verbs:
      unique_verbs.append(v[0])
    if v[1] not in unique_verbs:
      unique_verbs.append(v[1])

  verb_add, verb_mul, verb_sl = read_subjects()

  # Create our verb to verb vector dictionary
  verb_vrb = {}
   
  for idx in range(0,vec_data.shape[0]-1):
    v = dictionary[idx]
    if v in unique_verbs:
      verb_vrb[v] = vec_data[idx,:]

  for verb0, verb1 in verbs_to_check:
    if verb0 in dictionary and verb1 in dictionary and verb0 in verb_sl and verb1 in verb_sl:
      dist_vrb = cosine_distance(verb0, verb1, verb_vrb)
      dist_add = cosine_distance(verb0, verb1, verb_add)
      dist_mul = cosine_distance(verb0, verb1, verb_mul) 
      print ("-----------------")

      print (verb0, verb1, dist_vrb, dist_add, dist_mul, verb_sl[verb0], verb_sl[verb1])
      #print ("centroid",verb0,"add")
      #print (centroid(verb0, verb_add, verb_sl))
      #print ("centroid",verb0,"mul")
      #print (centroid(verb0, verb_mul, verb_sl))
      #print ("centroid",verb1,"add")
      #print (centroid(verb1, verb_add, verb_sl))
      #print ("centroid",verb1,"mul")
      #print (centroid(verb1, verb_mul, verb_sl))
      #print ("cosine centroid", verb0,verb1, "add")
      
      tt = {}
      tt[verb0] = centroid(verb0, verb_add, verb_sl)
      tt[verb1] = centroid(verb1, verb_add, verb_sl)

      #print ("centroid add", tt[verb0])
      #print ("normal add", verb_add[verb0])

      print ("cosine centroid add",verb0, verb1, cosine_distance(verb0,verb1,tt))
    
      #print ("cosine centroid", verb0,verb1, "mul")
      tt = {}
      tt[verb0] = centroid(verb0, verb_mul, verb_sl)
      tt[verb1] = centroid(verb1, verb_mul, verb_sl)

      print ("consine centroid mul", verb0, verb1, cosine_distance(verb0,verb1,tt))
      print ("")
    else :
      print (verb0, "and/or", verb1, "not in dictionary or subject matter.")

