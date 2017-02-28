'''
A module for all the things we want to read in
from wacky files
'''

import numpy as np
import codecs, struct

# Read the sim_stats to check for intransitive and transitive
def read_sim_stats(BASE_DIR, STATS_FILE):
  VERB_TRANSITIVE = []
  VERB_INTRANSITIVE = []

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

  return (VERB_TRANSITIVE, VERB_INTRANSITIVE)

# read the similarity file from http://people.ds.cam.ac.uk/dsg40/simverb.html
def read_sim_file(BASE_DIR, SIM_FILE):
  verbs_to_check = []
  with open(BASE_DIR + "/" + SIM_FILE,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      verbs_to_check.append( (tokens[0], tokens[1], float(tokens[3])) )
  
  return verbs_to_check


# Read in the basic DICTIONARY file
# This is the one we generate basically
def read_dictionary(BASE_DIR, DICT_FILE) :
  DICTIONARY = []
  #print("Reading Dictionary")
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines():
      DICTIONARY.append( line.replace("\n",""))

  return DICTIONARY

def read_w2v_dictionary(BASE_DIR, W2V_DICT_FILE):
  W2V_DICTIONARY = []
  W2V_REVERSE = {}
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

  return (W2V_DICTIONARY, W2V_REVERSE)

# Read the subjects verb_idx -> [subject_idxs]
def read_subjects_file(BASE_DIR, VOCAB_SIZE, SUBJECTS_FILE):
  SUBJECTS = []
  
  for i in range(0,VOCAB_SIZE):
    SUBJECTS.append([])

  with open(BASE_DIR + "/" + SUBJECTS_FILE, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      if (len(tokens) > 1):
        for sbj in tokens[1:]:
          sbj_idx = int(sbj)
          SUBJECTS[int(tokens[0])].append(sbj_idx)  

  return SUBJECTS

# Read the subject object pairs
def read_sbj_obj_file(BASE_DIR, VOCAB_SIZE, SBJ_OBJ_FILE):
  SBJ_OBJ = []
  
  for i in range(0,VOCAB_SIZE):
    SBJ_OBJ.append([])

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


# Read both the subjects and the objects. Used in the tran

def read_subject_object(verb, BASE_DIR, DICTIONARY, SUBJECTS_FILE, OBJECT_FILE, VEC_DATA, VEC_SIZE, W2V_REVERSE=[], word2vec=False) :

  #print("Reading Subjects")
  #print(verb,":")
  #base_vector = VEC_DATA[int(tokens[0])]
  
  verb_subjects = []
  verb_objects = []

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
              if verb_sbj in W2V_REVERSE:
                sbj_idx = W2V_REVERSE[verb_sbj]
              else:
                continue

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
            verb_obj = DICTIONARY[obj_idx]
            if word2vec:
              if verb_obj in W2V_REVERSE:
                obj_idx = W2V_REVERSE[verb_obj]
              else:
                continue

            # Now find the subject vectors
            vv = VEC_DATA[obj_idx]
            verb_objects.append(vv)

  return (verb_subjects, verb_objects)


# Read the Tensorflow version - held as a numpy array
# Quite straight forward basically
def read_tensorflow(filepath):

  VEC_DATA = np.load(filepath)
  VEC_SIZE = VEC_DATA.shape[1]

  return (VEC_DATA, VEC_SIZE)

# Read the binary file from word2vec, returning a numpy array
# we use the DICTIONARY created by tensorflow, because all the integer
# files, DICTIONARYs and verb/subject lookups are based on that/
def read_binary(filepath, VEC_SIZE, W2V_DICTIONARY, W2V_REVERSE):

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
  
  return (vectors, VEC_SIZE)


