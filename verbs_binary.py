#!/usr/bin/python

'''
Read in the subjects.txt file and the DICTIONARY to create
a set of vectors for a verb, based on the subjects of these
verbs
'''

import numpy as np
import os, sys, math, struct, codecs

BASE_DIR = ""
DICT_FILE = "dictionary.txt"
VERB_FILE = "subjects.txt"
W2V_DICT_FILE = "vocab.txt"
VEC_FILE = ""
VEC_SIZE = 768
DICTIONARY = []
W2V_DICTIONARY = []

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
    ('sleep', 'feel'),
  ]
'''
verbs_to_check = [
    ('sleep', 'snooze'), 
  ] 
'''
# Read in the basic DICTIONARY file
# This is the one we generate basically
def read_dictionary() :
  #print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines()[1:]:
      DICTIONARY.append( line.replace("\n",""))


def read_w2v_dictionary():
  # This could be better :/
  tt = {}
  with open(BASE_DIR + "/" + W2V_DICT_FILE,'rb') as f:
    f_decoded = codecs.getreader("ISO-8859-15")(f)
    for line in f_decoded.readlines():
      tokens = line.split(" ")
      if len(tokens) > 1:
        word = tokens[0]
        idx = int(tokens[1])
        tt[idx] = word
      else:
        print(line, "causes issues")

  for i in range(0,len(tt)-1):
    W2V_DICTIONARY[i] = "t"
  
  for i in tt.keys():
    W2V_DICTIONARY[i] = tt[i]
  

# Read the subject file - each line is a verb subject list of numbers - indices into the DICTIONARY
# First number is the verb, all following are the subjects

def read_subjects() :
  global DICTIONARY
  global vec_data
  global VEC_SIZE

  verb_add = {}
  verb_multiply = {}
  verb_min = {}
  verb_max = {}
  verb_kronecker = {}
  verb_kronecker_mm = {}
  verb_sublength = {}

  for v in unique_verbs:
    verb_add[v] = np.zeros((1,VEC_SIZE))
    verb_multiply[v] = np.zeros((1,VEC_SIZE))

  #print("Reading Subjects")
  with open(BASE_DIR + "/" + VERB_FILE, 'r') as f:
    for line in f.readlines():
      tokens = line.split(" ")
      if (len(tokens) > 2):
        verb = DICTIONARY[int(tokens[0])]
        
        if verb in unique_verbs:
        
          #print(verb,":")
          add_vector = np.zeros((1,VEC_SIZE))
          mul_vector = np.ones((1,VEC_SIZE))
          krn_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
          kmm_vector = np.ones((1,VEC_SIZE * VEC_SIZE))
          min_vector = np.zeros((1,VEC_SIZE))
          max_vector = np.zeros((1,VEC_SIZE))

          sl = 0

          for sbj in tokens[1:]:
            try:
              sbj_idx = int(sbj)
              #print (" -", DICTIONARY[sbj_idx])

              # Now find the subject vectors
              vv = vec_data[sbj_idx]
              add_vector = np.add(add_vector,vv)
              #print(np.linalg.norm(mul_vector))
              mul_vector = mul_vector * vv
              # Kronecker product - we have add and mul versions
              krn_vector = np.kron(vv, vv) + krn_vector
              kmm_vector = np.kron(vv, vv) * kmm_vector
              
              # Now do the min and max
              for i in range(0,VEC_SIZE-1):
                if vv[i] < min_vector[0][i] or min_vector[0][i] == 0.0:
                  min_vector[0][i] = vv[i]
                if vv[i] > max_vector[0][i] or max_vector[0][i] == 0.0:
                  max_vector[0][i] = vv[i]

              sl += 1
            except Exception as e:
              pass
              #print("Exception occured",e)
              # Mostly just tokens not parsing properly

            #print (" ->", sub_vector)
            
          # Work out the cosine distances
          #print(min_vector)
          #print(mul_vector)
          verb_add[verb] = add_vector[0]
          verb_multiply[verb] = mul_vector[0]
          verb_min[verb] = min_vector[0]
          verb_max[verb] = max_vector[0]
          verb_kronecker[verb] = krn_vector
          verb_kronecker_mm[verb] = kmm_vector
          verb_sublength[verb] = sl

  return verb_add, verb_multiply, verb_kronecker, verb_kronecker_mm, verb_min, verb_max, verb_sublength

# Determine the cosine distance
# https://en.wikipedia.org/wiki/Cosine_similarity#Angular_distance_and_similarity

def cosine_distance(verb0, verb1, verb_vec) :

  dist = 0
  if verb0 not in verb_vec:
    #print(verb0,"not in vectors")
    return dist

  if verb1 not in verb_vec:
    #print(verb1,"not in vectors")
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

# Return the Kronecker product, converted to a cosine distance for the subjects of the verbs

def kron_distance(verb0, verb1, verb_vec):
  # For now, just return the matrices
  # return (verb_vec[verb0], verb_vec[verb1])
  try: 
    dist = 0
    if verb0 not in verb_vec:
      #print(verb0,"not in vectors")
      return dist

    if verb1 not in verb_vec:
      #print(verb1,"not in vectors")
      return dist

    v0 = verb_vec[verb0]
    v1 = verb_vec[verb1]
    v1 = v1.reshape((v1.shape[1],1))
    
    n = np.dot(v0, v1)[0]
    n0 = np.linalg.norm(v0)
    n1 = np.linalg.norm(v1)
    d = n0 * n1

    #print ("n over d",n,d,n/d)

    if (d != 0):
      sim = n / d
      dist = math.acos(sim) / math.pi
   
    return 1.0 - dist
 
  except:
    return -1.0


# Return a vector divided by the number of subjects (an average basically)

def centroid (verb, verb_vec, verb_sl) :
  return verb_vec[verb] / float(verb_sl[verb])

# Read the binary file from word2vec, returning a numpy array
# we use the DICTIONARY created by tensorflow, because all the integer
# files, DICTIONARYs and verb/subject lookups are based on that/

def read_binary(filepath):

  vectors = []
  dlookup = {}
  global DICTIONARY
  global VEC_SIZE
  
  # As the tensorflow dict is different to the word2vec
  # create a quick lookup table
  for i in range(0,len(DICTIONARY)):
    dlookup[DICTIONARY[i]] = i

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

    for i in range(0,len(DICTIONARY)):
      vectors.append( np.zeros(VEC_SIZE) )

    #print("Loading vectors.bin with", words, "words with", VEC_SIZE, "vec size")

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
      #DICTIONARY.append(word)
      
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

  #vectors = np.vstack(vectors)
  #print(vectors.shape)
  #return DICTIONARY, vectors
  return vectors

# Main function

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]

  vec_data = []
  read_dictionary()
  read_w2v_dictionary()

  sys.exit()
  
  vec_data = read_binary(BASE_DIR + "/vectors.bin")
  
  for v in verbs_to_check:
    if v[0] not in unique_verbs:
      unique_verbs.append(v[0])
    if v[1] not in unique_verbs:
      unique_verbs.append(v[1])

  verb_add, verb_mul, verb_krn, verb_kmm, verb_min, verb_max, verb_sl = read_subjects()

  # Create our verb to verb vector DICTIONARY
  verb_vrb = {}

  print ("verb0,verb1,verb_dist,sub_add_dist,sub_mul_dist,num_sub_v0,num_sub_v1,kron_add_dist,kron_mul_dist,min_sub_dist,max_sub_dist")

  for idx in range(0,vec_data.shape[0]-1):
    v = DICTIONARY[idx]
    if v in unique_verbs:
      verb_vrb[v] = vec_data[idx,:]

  for verb0, verb1 in verbs_to_check:

    if verb0 in DICTIONARY and verb1 in DICTIONARY and verb0 in verb_sl and verb1 in verb_sl:
      dist_vrb = cosine_distance(verb0, verb1, verb_vrb)
      dist_add = cosine_distance(verb0, verb1, verb_add)
      dist_mul = cosine_distance(verb0, verb1, verb_mul) 
      dist_krn = kron_distance(verb0, verb1, verb_krn)
      dist_kmm = kron_distance(verb0, verb1, verb_kmm)
      dist_min = cosine_distance(verb0, verb1, verb_min)
      dist_max = cosine_distance(verb0, verb1, verb_max)
    
      print (verb0 + "," + verb1 + "," + str(dist_vrb) + "," + str(dist_add) + "," + str(dist_mul) + "," +  str(verb_sl[verb0]) + "," + str(verb_sl[verb1]) + "," + str(dist_krn) + "," + str(dist_kmm) + "," + str(dist_min) + "," + str(dist_max))
    
