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
SIM_FILE = "SimVerb-500-dev.txt"
STATS_FILE = "sim_stats.txt"
VEC_FILE = ""
VEC_SIZE = -1
DICTIONARY = []
W2V_DICTIONARY = []
W2V_REVERSE = {}
VEC_DATA = []
VERB_TRANSITIVE = []
VERB_INTRANSITIVE = []

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



# Read the sim_stats to check for intransitive and transitive
def read_sim_stats():
  with open(BASE_DIR + "/" + STATS_FILE,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      verb = tokens[0]
      obs = int(tokens[1])
      alone = int(tokens[2])
      total = int(tokens[3])
    
      if obs > alone:
        VERB_TRANSITIVE.append(verb)
      else:
        VERB_INTRANSITIVE.append(verb)

# read the similarity file from http://people.ds.cam.ac.uk/dsg40/simverb.html
def read_sim_file():
  global verbs_to_check
  verbs_to_check = []
  with open(BASE_DIR + "/" + SIM_FILE,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      verbs_to_check.append( (tokens[0], tokens[1], float(tokens[3])) )


# Read in the basic DICTIONARY file
# This is the one we generate basically
def read_dictionary() :
  #print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines()[1:]:
      DICTIONARY.append( line.replace("\n",""))


def read_w2v_dictionary():
  #print ("Reading w2v vocab")
  # This could be better :/
  tt = {}
  with open(BASE_DIR + "/" + W2V_DICT_FILE,'rb') as f:
    f_decoded = codecs.getreader("ISO-8859-15")(f)
    for line in f_decoded.readlines():
      tokens = line.split(" ")
      word = tokens[0]
      W2V_DICTIONARY.append(word)

  idx = 0
  for i in W2V_DICTIONARY:
    W2V_REVERSE[W2V_DICTIONARY[idx]] = idx;
    idx += 1

# Read the subject file - each line is a verb subject list of numbers - indices into the DICTIONARY
# We don't read the entire file - we scoot to the index for the verb given

def read_subjects(verb, choose_trans=True) :

  #print("Reading Subjects")
  global VEC_SIZE

  with open(BASE_DIR + "/" + VERB_FILE, 'r') as f:
    for line in f.readlines():
      tokens = line.split(" ")
      if (len(tokens) > 2):
      
        found_verb = DICTIONARY[int(tokens[0])]
        
        if verb == found_verb: 
        
          #print(verb,":")
          base_vector = VEC_DATA[int(tokens[0])]
          base_vector_flip = base_vector.reshape((VEC_SIZE,1))
          add_vector = np.zeros((1,VEC_SIZE))
          min_vector = np.zeros((1,VEC_SIZE))
          max_vector = np.zeros((1,VEC_SIZE))

          add_base_add_vector = np.zeros((1,VEC_SIZE))
          min_base_add_vector = np.zeros((1,VEC_SIZE))
          max_base_add_vector = np.zeros((1,VEC_SIZE))

          add_base_mul_vector = np.zeros((1,VEC_SIZE))
          min_base_mul_vector = np.zeros((1,VEC_SIZE))
          max_base_mul_vector = np.zeros((1,VEC_SIZE))

          krn_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
          krn_base_mul_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
          krn_base_add_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
          
          for sbj in tokens[1:]:
            try:
              sbj_idx = int(sbj)

              # Convert to the word2vec lookup
              verb_sbj = DICTIONARY[sbj_idx]
              sbj_idx = W2V_REVERSE[verb_sbj]

              # Now find the subject vectors
              vv = VEC_DATA[sbj_idx]

              if choose_trans and verb in VERB_TRANSITIVE :
                pass

              elif verb in VERB_INTRANSITIVE:
                # Add vector math
                add_vector = np.add(add_vector,vv)
              
                # Kronecker product - we have add and mul versions
                krn_vector = np.kron(vv, vv) + krn_vector
              
                # Now do the min and max
                for i in range(0,VEC_SIZE-1):
                  if vv[i] < min_vector[0][i] or min_vector[0][i] == 0.0:
                    min_vector[0][i] = vv[i]

                  if vv[i] > max_vector[0][i] or max_vector[0][i] == 0.0:
                    max_vector[0][i] = vv[i]

                # Now we perform the modifications with the bases
                add_base_add_vector = np.add(add_vector, base_vector)
                add_base_mul_vector = add_vector * base_vector

                min_base_add_vector = np.add(min_vector, base_vector)
                min_base_mul_vector = min_vector * base_vector

                max_base_add_vector = np.add(max_vector, base_vector)
                max_base_mul_vector = max_vector * base_vector

                krn_base_add_vector = np.add (krn_vector, np.kron(base_vector,base_vector))
                krn_base_mul_vector = krn_vector * np.kron(base_vector,base_vector)

              else:
                # This verb doesnt exist in trans or intrans so ignore
                return (False,0)

            except Exception as e:
              return (False,0)

              #print("Exception occured",e)
              # Mostly just tokens not parsing properly
              # Should probably look at this more carefully
           
          # Work out the cosine distances
          #print(min_vector)
          #print(mul_vector)
          #verb_base = bas_vector[0]
          #verb_add = add_vector[0]
          #verb_min = min_vector[0]
          #verb_max = max_vector[0]
          #verb_kronecker = krn_vector
          #verb_sublength = sl

          return (True, base_vector, add_vector, min_vector, max_vector, add_base_add_vector, add_base_mul_vector, min_base_add_vector, min_base_mul_vector, max_base_add_vector, max_base_mul_vector, krn_vector, krn_base_add_vector, krn_base_mul_vector )
  
  return (False,0)

# Determine the cosine distance
# https://en.wikipedia.org/wiki/Cosine_similarity#Angular_distance_and_similarity
# Pass in the two vectors 

def cosine_distance(v0, v1) :

  dist = 0

  n = np.dot(v0, v1)
  n0 = np.linalg.norm(v0)
  n1 = np.linalg.norm(v1)
  d = n0 * n1

  #print ("n over d",n,d,n/d)

  try:
    if (d != 0):
      sim = n / d
      if (sim != 1.0 and sim != 0.0):
        dist = math.acos(sim) / math.pi
  except:
    pass

  return 1.0 - dist

# Return the Kronecker product, converted to a cosine distance for the subjects of the verbs

def kron_distance(v0, v1):
  # For now, just return the matrices
  # return (verb_vec[verb0], verb_vec[verb1])
  try: 
    dist = 0

    v1 = v1.reshape((v1.shape[1],1))
    
    n = np.dot(v0, v1)[0]
    n0 = np.linalg.norm(v0)
    n1 = np.linalg.norm(v1)
   
    d = n0 * n1

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

  global VEC_SIZE

  #print("Reading binary")
  vectors = []
 
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

    for i in range(0,len(W2V_DICTIONARY)):
      vectors.append( np.zeros(VEC_SIZE) )

    #print("Loading vectors.bin with", words, "words with", VEC_SIZE, "vec size")

    # TODO - for some reason this explodes memory (the numbers bit at the end) even though
    # the C version copes fine. I suspect we must be duplicating something? 
    # regardless we need to figure out why that is or restrict to top 50,000 words :/

    widx = 0
    while(widx < words):
      skip = False
      word = ""
      while (True):
        byte = f.read(1)
        try:
          char = byte.decode("ascii")
        
          if char == " ":
           break
          word += char
        except :
          # We got a character that is non ascii so skip this row - but keep reading
          skip = True
          

      word = word.replace("\n", "")
      
      # Now read the numbers, should be VEC_SIZE of them (4 bytes probably)
      tidx = 0
      pidx = -1
      if word in W2V_REVERSE.keys():
        pidx = W2V_REVERSE[word]
      
      for i in range(0,VEC_SIZE):
        byte = f.read(4)
        num = struct.unpack('f',byte)

        if pidx != -1 and not skip:
          vectors[pidx].itemset(tidx, num[0])

        tidx += 1
     
      widx += 1

  vectors = np.vstack(vectors)
  #print(vectors.shape)
  #return DICTIONARY, vectors
  
  return vectors

# Intransitive Verb output

def intrans():

  for verb0, verb1, sim in verbs_to_check:
  
    r0 = read_subjects(verb0,False)
    r1 = read_subjects(verb1,False)
    
    if r0[0] and r1[0]:
      
      base_vector0 = r0[1] 
      add_vector0 = r0[2] 
      min_vector0 = r0[3] 
      max_vector0 = r0[4] 
      add_base_add_vector0 = r0[5]  
      add_base_mul_vector0 = r0[6] 
      min_base_add_vector0 = r0[7] 
      min_base_mul_vector0 = r0[8] 
      max_base_add_vector0 = r0[9] 
      max_base_mul_vector0 = r0[10] 
      krn_vector0 = r0[11] 
      krn_base_add_vector0 = r0[12] 
      krn_base_mul_vector0 = r0[13] 
  
      base_vector1 = r1[1] 
      add_vector1 = r1[2] 
      min_vector1 = r1[3] 
      max_vector1 = r1[4] 
      add_base_add_vector1 = r1[5]  
      add_base_mul_vector1 = r1[6] 
      min_base_add_vector1 = r1[7] 
      min_base_mul_vector1 = r1[8] 
      max_base_add_vector1 = r1[9] 
      max_base_mul_vector1 = r1[10] 
      krn_vector1 = r1[11] 
      krn_base_add_vector1 = r1[12] 
      krn_base_mul_vector1 = r1[13] 

      print ("verb0,verb1,base_sim,add_sim,min_sim,max_sim,add_add_sim,add_mul_sim,min_add_sim,min_mul_sim,max_add_sim,max_mul_sim,krn_sim,krn_add_sim,krn_mul_sim")

      print (str(cosine_distance(base_vector0, base_vector1)) + "," + 
          str(cosine_distance(add_vector0, add_vector1)) + "," +
          str(cosine_distance(min_vector0, min_vector1)) + "," +
          str(cosine_distance(max_vector0, max_vector1)) + "," +
          str(cosine_distance(add_base_add_vector0, add_base_add_vector1)) + "," +
          str(cosine_distance(add_base_mul_vector0, add_base_mul_vector1)) + "," +
          str(cosine_distance(min_base_add_vector0, min_base_add_vector1)) + "," +
          str(cosine_distance(min_base_mul_vector0, min_base_mul_vector1)) + "," +
          str(cosine_distance(max_base_add_vector0, max_base_add_vector1)) + "," +
          str(cosine_distance(max_base_mul_vector0, max_base_mul_vector1)) + "," +
          str(kron_distance(krn_vector0, krn_vector1)) + "," +
          str(kron_distance(krn_base_add_vector0, krn_base_add_vector1)) + "," +
          str(kron_distance(krn_base_mul_vector0, krn_base_mul_vector1))
          )
         

# Main function

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]

  read_sim_stats()
  read_sim_file()
  read_dictionary()
  read_w2v_dictionary()
  
  VEC_DATA = read_binary(BASE_DIR + "/vectors.bin")
 
  intrans() 
