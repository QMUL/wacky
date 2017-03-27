#!/usr/bin/python

'''
Read in the subjects.txt file and the DICTIONARY to create
a set of vectors for a verb, based on the subjects of these
verbs
'''

import numpy as np
import os, sys, math, struct, codecs

from scipy import stats

BASE_DIR = ""
DICT_FILE = "dictionary.txt"
SUBJECTS_FILE = "verb_subjects.txt"
OBJECTS_FILE = "verb_objects.txt"
SBJ_OBJ_FILE = "verb_sbj_obj.txt"
W2V_DICT_FILE = "vocab.txt"
SIM_FILE = "SimVerb-3000-test.txt"
STATS_FILE = "sim_stats.txt"
VEC_FILE = ""
VEC_SIZE = -1
DICTIONARY = []
W2V_DICTIONARY = []
W2V_REVERSE = {}
VEC_DATA = []
VERB_TRANSITIVE = []
VERB_INTRANSITIVE = []

SUBJECTS = {}

SBJ_OBJ = {}

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
    for line in f.readlines():
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


# Read the subjects verb_idx -> [subject_idxs]
def read_subjects_file():
  with open(BASE_DIR + "/" + SUBJECTS_FILE, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      SUBJECTS[int(tokens[0])] = []
      if (len(tokens) > 1):
        for sbj in tokens[1:]:
          sbj_idx = int(sbj)
          SUBJECTS[int(tokens[0])].append(sbj_idx)  
 

# Read the subject object pairs
def read_sbj_obj_file():
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
 
# Read both the subjects and the objects. Used in the tran

def read_subject_object(verb, word2vec=False) :

  #print("Reading Subjects")
  global VEC_SIZE

  #print(verb,":")
  #base_vector = VEC_DATA[int(tokens[0])]
  
  verb_subjects = []

  with open(BASE_DIR + "/" + SUBJECTS_FILE, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      if (len(tokens) > 2):
      
        found_subject = DICTIONARY[int(tokens[0])]
        
        if verb == found_subject: 
          for sbj in tokens[1:]:
            sbj_idx = int(sbj)

            # Convert to the word2vec lookup
            verb_sbj = DICTIONARY[sbj_idx]
            if word2vec:
              sbj_idx = W2V_REVERSE[verb_sbj]

            # Now find the subject vectors
            vv = VEC_DATA[sbj_idx]
            verb_subjects.append(vv)

    
  with open(BASE_DIR + "/" + OBJECT_FILE, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      if (len(tokens) > 2):
      
        found_object = DICTIONARY[int(tokens[0])]
        
        if verb == found_object: 
          for obj in tokens[1:]:

            obj_idx = int(obj)

            # Convert to the word2vec lookup
            verb_obj = DICTIONARY[sbj_idx]
            if word2vec:
              obj_idx = W2V_REVERSE[verb_obj]

            # Now find the subject vectors
            vv = VEC_DATA[obj_idx]
            verb_objects.append(vv)

  # Now we combine - hopefully they will match up :/
  
# Read the subject object file - a little different. We have pairs of SBJ OBJ for each verb

def read_sbj_obj(verb, word2vec=False):
  global VEC_SIZE

  for found_idx in SBJ_OBJ.keys():
    found_verb = DICTIONARY[found_idx]
  
    if verb == found_verb: 
  
      base_vector = VEC_DATA[ found_idx ]

      if word2vec:
        base_vector = VEC_DATA[ W2V_REVERSE[verb] ]
    
      sbj_obj_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
      sbj_vector = np.zeros((1,VEC_SIZE))
      obj_vector = np.zeros((1,VEC_SIZE))
      sbj_obj_add_vector = np.zeros((1,VEC_SIZE * VEC_SIZE)) 
      sbj_obj_mul_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))

      for sbj_idx, obj_idx in SBJ_OBJ[found_idx]:

        verb_sbj = DICTIONARY[sbj_idx]
        verb_obj = DICTIONARY[obj_idx]

        if word2vec:
          if verb_sbj not in W2V_REVERSE or verb_obj not in W2V_REVERSE:
            continue

          sbj_idx = W2V_REVERSE[verb_sbj]
          obj_idx = W2V_REVERSE[verb_obj]

        vs = VEC_DATA[sbj_idx]
        vo = VEC_DATA[obj_idx]

        sbj_vector = np.add(sbj_vector,vs)
        obj_vector = np.add(obj_vector,vo)

        tk = np.kron(vs,vo)

        sbj_obj_vector = np.add(sbj_obj_vector, tk)
        sbj_obj_add_vector = np.add(sbj_obj_add_vector, tk)
        sbj_obj_mul_vector = np.add(sbj_obj_mul_vector, tk)

      sbj_obj_add_vector = np.add(sbj_obj_add_vector, np.kron(base_vector, base_vector))
      sbj_obj_mul_vector = sbj_obj_mul_vector * np.kron(base_vector, base_vector)

      sum_sbj_obj = np.add(sbj_vector,obj_vector)
      sum_sbj_obj_mul = sum_sbj_obj * base_vector
      sum_sbj_obj_add = np.add(sum_sbj_obj, base_vector)

      return (True, base_vector, sbj_obj_vector, sbj_obj_add_vector, sbj_obj_mul_vector, sum_sbj_obj, sum_sbj_obj_mul, sum_sbj_obj_add)

  return (False,0)


def read_sbj_obj2(verb, word2vec=False):
  global VEC_SIZE

  for found_idx in SBJ_OBJ.keys():
    found_verb = DICTIONARY[found_idx]
  
    if verb == found_verb: 
  
      base_vector = VEC_DATA[ found_idx ]

      if word2vec:
        base_vector = VEC_DATA[ W2V_REVERSE[verb] ]
  
      sbj_obj_vector = np.zeros((1,VEC_SIZE))
      sbj_obj_krn_vector = np.zeros((1,VEC_SIZE * VEC_SIZE)) 

      for sbj_idx, obj_idx in SBJ_OBJ[found_idx]:

        verb_sbj = DICTIONARY[sbj_idx]
        verb_obj = DICTIONARY[obj_idx]

        if word2vec:
          if verb_sbj not in W2V_REVERSE or verb_obj not in W2V_REVERSE:
            continue

          sbj_idx = W2V_REVERSE[verb_sbj]
          obj_idx = W2V_REVERSE[verb_obj]

        vs = VEC_DATA[sbj_idx]
        vo = VEC_DATA[obj_idx]

        sbj_obj_vector = np.add(sbj_obj_vector, np.add(vo,vs))
        
        tk = np.kron(vs,vo)
        sbj_obj_krn_vector = np.add(sbj_obj_krn_vector, tk)


      return (True, base_vector, sbj_obj_vector, sbj_obj_krn_vector)

  return (False,0)




# Read the subject file - each line is a verb subject list of numbers - indices into the DICTIONARY
# We don't read the entire file - we scoot to the index for the verb given
# Only called on intransitive verbs
def read_subjects(verb, word2vec=False) :

  #print("Reading Subjects")
  global VEC_SIZE

  for found_idx in SUBJECTS.keys():
  
    found_verb = DICTIONARY[found_idx]

    if verb == found_verb: 
 
      base_vector = VEC_DATA[found_idx]

      if word2vec:
        base_vector = VEC_DATA[ W2V_REVERSE[verb] ]

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
      
      for sbj_idx in SUBJECTS[found_idx]:

        # Convert to the word2vec lookup
        verb_sbj = DICTIONARY[sbj_idx]

        if word2vec:
          if verb_sbj not in W2V_REVERSE:
            continue

          sbj_idx = W2V_REVERSE[verb_sbj]

        # Now find the subject vectors
        vv = VEC_DATA[sbj_idx]

        # Add vector math
        add_vector = np.add(add_vector,vv)
      
        # Kronecker product - we have add and mul versions
        krn_vector = np.kron(vv, vv) + krn_vector
      
        # Now do the min and max
        for i in range(0,VEC_SIZE):
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

      return (True, base_vector, add_vector, min_vector, max_vector, add_base_add_vector, add_base_mul_vector, min_base_add_vector, min_base_mul_vector, max_base_add_vector, max_base_mul_vector, krn_vector, krn_base_add_vector, krn_base_mul_vector )

  return (False,0)


def read_subjects_2(verb, word2vec=False) :

  #print("Reading Subjects")
  global VEC_SIZE

  for found_idx in SUBJECTS.keys():
  
    found_verb = DICTIONARY[found_idx]

    if verb == found_verb: 
 
      base_vector = VEC_DATA[found_idx]

      if word2vec:
        base_vector = VEC_DATA[ W2V_REVERSE[verb] ]

      add_vector = np.zeros((1,VEC_SIZE))
      krn_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
      
      for sbj_idx in SUBJECTS[found_idx]:

        # Convert to the word2vec lookup
        verb_sbj = DICTIONARY[sbj_idx]

        if word2vec:
          if verb_sbj not in W2V_REVERSE:
            continue

          sbj_idx = W2V_REVERSE[verb_sbj]

        # Now find the subject vectors
        vv = VEC_DATA[sbj_idx]

        # Add vector math
        add_vector = np.add(add_vector,vv)
      
        # Kronecker product - we have add and mul versions
        krn_vector = np.kron(vv, vv) + krn_vector
      
      return (True, base_vector, add_vector, krn_vector )

  return (False,0)


# Determine the cosine similarity
# https://en.wikipedia.org/wiki/Cosine_similarity#Angular_distance_and_similarity
# Pass in the two vectors 

def cosine_sim(v0, v1) :

  dist = 0
  # Not needed in the trans case
  if v1.shape[0] == VEC_SIZE:
    v1 = v1.reshape(( v1.shape[0],))
  else:
    v1 = v1.reshape((v1.shape[1], 1))
  n = np.dot(v0, v1)
  n0 = np.linalg.norm(v0)
  n1 = np.linalg.norm(v1)
  d = n0 * n1

  try:
    if (d != 0):
      sim = n / d
      if (sim != 1.0 and sim != 0.0):
        dist = math.acos(sim) / math.pi
  except:
    return -1.0

  return 1.0 - dist



def kron_sim(v0, v1):
  try: 
    dist = 0
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

# Read the Tensorflow version - held as a numpy array
# Quite straight forward basically

def read_tensorflow(filepath):

  global VEC_SIZE

  VEC_DATA = np.load(filepath)
  VEC_SIZE = VEC_DATA.shape[1]

  return VEC_DATA

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

#transitive verb output

def trans(word2vec=False):

  print ("verb0,verb1,base_sim,sbj_obj_sim,sbj_obj_add,sbj_obj_mul,sum_sbj_obj,sum_sbj_obj_mul,sum_sbj_obj_add, human_sim")

  cc = []
  hc = []

  for i in range(0,7):
    cc.append([])

  for verb0, verb1, sim in verbs_to_check:
  
    if not(verb0 in VERB_TRANSITIVE and verb1 in VERB_TRANSITIVE):
      continue
    
    r0 = read_sbj_obj(verb0,word2vec)
    r1 = read_sbj_obj(verb1,word2vec)

    if r0[0] and r1[0]:  
      base_vector0 = r0[1] 
      sbj_obj_vector0 = r0[2]
      sbj_obj_add_vector0 = r0[3]
      sbj_obj_mul_vector0 = r0[4]
      sum_sbj_obj_vector0 = r0[5]
      sum_sbj_obj_mul_vector0 = r0[6]
      sum_sbj_obj_add_vector0 = r0[7]

      base_vector1 = r1[1] 
      sbj_obj_vector1 = r1[2]
      sbj_obj_add_vector1 = r1[3]
      sbj_obj_mul_vector1 = r1[4]
      sum_sbj_obj_vector1 = r1[5]
      sum_sbj_obj_mul_vector1 = r1[6]
      sum_sbj_obj_add_vector1 = r1[7]


      cc[0].append(cosine_sim(base_vector0, base_vector1))  
      cc[1].append(cosine_sim(sbj_obj_vector0, sbj_obj_vector1)) 
      cc[2].append(cosine_sim(sbj_obj_add_vector0, sbj_obj_add_vector1)) 
      cc[3].append(cosine_sim(sbj_obj_mul_vector0, sbj_obj_mul_vector1)) 

      cc[4].append(cosine_sim(sum_sbj_obj_vector0, sum_sbj_obj_vector1)) 
      cc[5].append(cosine_sim(sum_sbj_obj_mul_vector0, sum_sbj_obj_mul_vector1)) 
      cc[6].append(cosine_sim(sum_sbj_obj_add_vector0, sum_sbj_obj_add_vector1)) 
      
      sys.stdout.write(verb0 + "," + verb1 + ",")
       
      hc.append(sim)

      for i in range(0,len(cc)):
        sys.stdout.write(str(cc[i][-1]) + ",")
      
      print(str(sim)) 

  # Finally work out the spearman-rho
  # https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.spearmanr.html
  rho = []
  for i in range(0,len(cc)):
    rho.append(stats.spearmanr(cc[i],hc))
  
  print("----")
  print("rho_base,rho_sbj_obj,rho_sbj_obj_add,rho_sbj_obj_mul,rho_sum_sbj_obj,rho_sum_sbj_obj_mul,rho_sum_sbj_obj_add")
  for i in range(0,6):
    r, pval = rho[i]
    sys.stdout.write(str(r) + ",")
  print(str(rho[-1][0]))

  print("p_base,p_sbj_obj,p_sbj_obj_add,p_sbj_obj_mul,p_sum_sbj_obj,p_sum_sbj_obj_ml,p_sum_sub_obj_add")

  for i in range(0,6):
    r, pval = rho[i]
    sys.stdout.write(str(pval) + ",")
  print(str(rho[-1][1]))

# Intransitive case
def intrans(word2vec=False):

  print ("verb0,verb1,base_sim,add_sim,min_sim,max_sim,add_add_sim,add_mul_sim,min_add_sim,min_mul_sim,max_add_sim,max_mul_sim,krn_sim,krn_add_sim,krn_mul_sim,human_sim")

  cc = []
  hc = []
  for i in range(0,13):
    cc.append([])

  for verb0, verb1, sim in verbs_to_check:
  
    if not(verb0 in VERB_INTRANSITIVE and verb1 in VERB_INTRANSITIVE):
      continue

    #print(verb0,verb1)
    r0 = read_subjects(verb0,word2vec)
    r1 = read_subjects(verb1,word2vec)

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

      
      cc[0].append(cosine_sim(base_vector0, base_vector1)) 
      cc[1].append(cosine_sim(add_vector0, add_vector1))
      cc[2].append(cosine_sim(min_vector0, min_vector1))
      cc[3].append(cosine_sim(max_vector0, max_vector1))
      cc[4].append(cosine_sim(add_base_add_vector0, add_base_add_vector1))
      cc[5].append(cosine_sim(add_base_mul_vector0, add_base_mul_vector1))
      cc[6].append(cosine_sim(min_base_add_vector0, min_base_add_vector1))
      cc[7].append(cosine_sim(min_base_mul_vector0, min_base_mul_vector1))
      cc[8].append(cosine_sim(max_base_add_vector0, max_base_add_vector1))
      cc[9].append(cosine_sim(max_base_mul_vector0, max_base_mul_vector1))
      cc[10].append(kron_sim(krn_vector0, krn_vector1))
      cc[11].append(kron_sim(krn_base_add_vector0, krn_base_add_vector1))
      cc[12].append(kron_sim(krn_base_mul_vector0, krn_base_mul_vector1))
      
      sys.stdout.write(verb0 + "," + verb1 + ",")
      
      hc.append(sim)

      for i in range(0,len(cc)):
        sys.stdout.write(str(cc[i][-1]) + ",")
      
      print(str(sim)) 

  # Finally work out the spearman-rho
  # https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.spearmanr.html
  rho = []
  for i in range(0,len(cc)):
    rho.append(stats.spearmanr(cc[i],hc))
  
  print("----")
  print("rho_base,rho_add,rho_min,rho_max,rho_add_add,rho_add_mul,rho_min_add,rho_min_mul,rho_max_add,rho_max_mul,krn_rho,krn_add_rho,krn_mul_rho")
  for i in range(0,12):
    r, pval = rho[i]
    sys.stdout.write(str(r) + ",")
  print(str(rho[-1][0]))

  print("p_base,p_add,p_min,p_max,p_add_add,p_add_mul,p_min_add,p_min_mul,p_max_add,p_max_mul,krn_p,krn_add_p,krn_mul_p")

  for i in range(0,12):
    r, pval = rho[i]
    sys.stdout.write(str(pval) + ",")
  print(str(rho[-1][1]))


# transitive vs intransitive

def trans_intrans(word2vec=False):

  print ("verb0,verb1,base,add,add_verb,mul_verb,krn,krn_add_verb,krn_mul_verb,human_sim")

  cc = []
  hc = []
  for i in range(0,7):
    cc.append([])

  for verb0, verb1, sim in verbs_to_check:
  
    r0 = []
    r1 = []
  
    if verb0 in VERB_TRANSITIVE:
      r0 = read_sbj_obj2(verb0,word2vec)
    else:
      r0 = read_subjects_2(verb0,word2vec)
 
    if verb1 in VERB_TRANSITIVE:
      r1 = read_sbj_obj2(verb1,word2vec)
    else:
      r1 = read_subjects_2(verb1,word2vec)

    if r0[0] and r1[0]:

      base_sbj_obj = r0[1]
      sbj_obj_sum = r0[2]
      sbj_obj_krn = r0[3]

      base_sbj = r1[1]
      sbj_add = r1[2]
      sbj_krn = r1[3]
      
      cc[0].append(cosine_sim(base_sbj, base_sbj_obj))
      cc[1].append(cosine_sim(sbj_add, sbj_obj_sum))
      cc[2].append(cosine_sim(np.add(sbj_add,base_sbj), np.add(sbj_obj_sum, base_sbj_obj)))
      
      cc[3].append(cosine_sim(sbj_add * base_sbj, sbj_obj_sum * base_sbj_obj))
      cc[4].append(kron_sim(sbj_obj_krn, sbj_krn))
      cc[5].append(kron_sim( np.add(sbj_obj_krn, np.kron(base_sbj_obj, base_sbj_obj)), np.add(sbj_krn, np.kron(base_sbj, base_sbj))))
      cc[6].append(kron_sim(sbj_obj_krn * np.kron(base_sbj_obj, base_sbj_obj), sbj_krn * np.kron(base_sbj, base_sbj)))

      sys.stdout.write(verb0 + "," + verb1 + ",")
      
      hc.append(sim)
      
      for i in range(0,len(cc)):
        sys.stdout.write(str(cc[i][-1]) + ",")
      
      print(str(sim)) 

  rho = []
  for i in range(0,len(cc)):
    rho.append(stats.spearmanr(cc[i],hc))
  
  print("----")
  print("rho_base,rho_add,rho_add_verb,rho_mul_verb,krn,krn_add,krn_mul")
  for i in range(0,7):
    r, pval = rho[i]
    sys.stdout.write(str(r) + ",")
  print(str(rho[-1][0]))

  print("p_base,p_add,p_add_verb,p_mul_verb,p_krn,p_krn_add,p_krn_mul")

  for i in range(0,7):
    r, pval = rho[i]
    sys.stdout.write(str(pval) + ",")
  print(str(rho[-1][1]))


# Main function

if __name__ == "__main__" :
  BASE_DIR = sys.argv[1]

  transitive = False
  both = False
  word = False

  if len(sys.argv) >= 3:
    if sys.argv[2] == '-t':
      transitive = True

    elif sys.argv[2] == '-b':
      both = True
 

    if len(sys.argv) >= 4:
      if sys.argv[3] == '-w':
        word = True
  
  read_sim_stats()
  read_sim_file()
  read_dictionary()
  read_w2v_dictionary()
  read_subjects_file()
 
  if word:
    VEC_DATA = read_binary(BASE_DIR + "/vectors.bin")
  else:
    VEC_DATA = read_tensorflow(BASE_DIR + "/final_standard_embeddings.npy")

  if transitive:
    read_sbj_obj_file()
    trans(word)
  elif both:
    read_sbj_obj_file()
    trans_intrans(word)
  else:
    intrans(word)
