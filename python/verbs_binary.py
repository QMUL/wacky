#!/usr/bin/python

'''
Read in the subjects.txt file and the DICTIONARY to create
a set of vectors for a verb, based on the subjects of these
verbs
'''

import numpy as np
import os, sys, math, struct, codecs

from scipy import stats

def read_sim_stats(base_dir, stats_file):
  '''Read the sim_stats to check for intransitive and transitive.'''
  verb_transitive = []
  verb_intransitive = []

  with open(base_dir + "/" + stats_file,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      verb = tokens[0]
      obs = int(tokens[1])
      alone = int(tokens[2])
      total = int(tokens[3])
    
      if obs > alone:
        verb_transitive.append(verb)
      else:
        verb_intransitive.append(verb)

  verb_transitive, verb_intransitive

def read_sim_file(base_dir, sim_file):
  '''Read the similarity file from http://people.ds.cam.ac.uk/dsg40/simverb.html '''
  verbs_to_check = []
  with open(base_dir + "/" + sim_file,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      verbs_to_check.append( (tokens[0], tokens[1], float(tokens[3])) )
  
  return verbs_to_check


def read_dictionary(base_dir, dict_file) :
  '''Read in the basic DICTIONARY file.'''
  dictionary = []
  with open(base_dir + "/" + dict_file,'r') as f:
    for line in f.readlines():
      dictionary.append( line.replace("\n",""))
  return dictionary

def read_w2v_dictionary(base_dir, w2v_dict_file):
  ''' Read in the word2vec vocab and reverse it.'''
  tt = {}
  w2v_dictionary = []
  w2v_reverse = {}

  with open(base_dir + "/" + w2v_dict_file,'rb') as f:
    f_decoded = codecs.getreader("ISO-8859-15")(f)
    for line in f_decoded.readlines():
      tokens = line.split(" ")
      word = tokens[0]
      w2v_dictionary.append(word)

  idx = 0
  for i in w2v_dictionary:
    w2v_reverse[w2v_dictionary[idx]] = idx;
    idx += 1
  
  return w2v_dictionary, w2v_reverse


def read_subjects_file(base_dir, subjects_file):
  '''Read the subjects verb_idx -> [subject_idxs]'''
  subjects = {}
  with open(base_dir + "/" + subjects_file, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      subjects[int(tokens[0])] = []
      if (len(tokens) > 1):
        for sbj in tokens[1:]:
          sbj_idx = int(sbj)
          subjects[int(tokens[0])].append(sbj_idx)  
  return subjects


def read_sbj_obj_file(base_dir, sbj_obj_file):
  '''Read the subject object pairs.'''
  sbj_obj = {}
  with open(base_dir + "/" + sbj_obj_file, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      sbj_obj[int(tokens[0])] = []
      if (len(tokens) > 1):
        for i in range(1,len(tokens),2):
          sbj_idx = int(tokens[i])
          obj_idx = int(tokens[i+1])
          sbj_obj[int(tokens[0])].append( (sbj_idx, obj_idx) ) 
  
  return sbj_obj
 

def read_subject_object(base_dir, subjects_file, object_file, dictionary, w2v_reverse, vec_data, verb, vec_size, word2vec=False) :
  ''' Read both the subjects and the objects. Used in the tran.'''
  verb_subjects = []
  verb_objects = []

  with open(base_dir + "/" + subjects_file, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      if (len(tokens) > 2):
      
        found_subject = dictionary[int(tokens[0])]
        
        if verb == found_subject: 
          for sbj in tokens[1:]:
            sbj_idx = int(sbj)

            # Convert to the word2vec lookup
            verb_sbj = dictionary[sbj_idx]
            if word2vec:
              sbj_idx = w2v_reverse[verb_sbj]

            # Now find the subject vectors
            vv = vec_data[sbj_idx]
            verb_subjects.append(vv)
    
  with open(base_dir + "/" + object_file, 'r') as f:
    for line in f.readlines():
      line = line.replace("\n","")
      tokens = line.split()
      if (len(tokens) > 2):
      
        found_object = dictionary[int(tokens[0])]
        
        if verb == found_object: 
          for obj in tokens[1:]:

            obj_idx = int(obj)

            # Convert to the word2vec lookup
            verb_obj = dictionary[sbj_idx]
            if word2vec:
              obj_idx = w2v_reverse[verb_obj]

            # Now find the subject vectors
            vv = vec_data[obj_idx]
            verb_objects.append(vv)

  return verb_objects, verb_subjects
  

def read_sbj_obj(verb, dictionary, w2v_reverse, sbj_obj, vec_data, vec_size, word2vec=False):
  ''' Read the subject object file - a little different. We have pairs of SBJ OBJ for each verb.'''
  for found_idx in sbj_obj.keys():
    found_verb = dictionary[found_idx]
  
    if verb == found_verb: 
  
      base_vector = vec_data[ found_idx ]

      if word2vec:
        base_vector = vec_data[ w2v_reverse[verb] ]
    
      sbj_obj_vector = np.zeros((1,vec_size * vec_size))
      sbj_vector = np.zeros((1,vec_size))
      obj_vector = np.zeros((1,vec_size))
      sbj_obj_add_vector = np.zeros((1,vec_size * vec_size)) 
      sbj_obj_mul_vector = np.zeros((1,vec_size * vec_size))

      for sbj_idx, obj_idx in sbj_obj[found_idx]:

        verb_sbj = dictionary[sbj_idx]
        verb_obj = dictionary[obj_idx]

        if word2vec:
          if verb_sbj not in w2v_reverse or verb_obj not in w2v_reverse:
            continue

          sbj_idx = w2v_reverse[verb_sbj]
          obj_idx = w2v_reverse[verb_obj]

        vs = vec_data[sbj_idx]
        vo = vec_data[obj_idx]

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


def read_sbj_obj2(verb, dictionary, w2v_reverse, vec_data, vec_size, sbj_obj, word2vec=False):

  for found_idx in sbj_obj.keys():
    found_verb = dictionary[found_idx]
  
    if verb == found_verb: 
  
      base_vector = vec_data[ found_idx ]

      if word2vec:
        base_vector = vec_data[ w2v_reverse[verb] ]
  
      sbj_obj_vector = np.zeros((1,vec_size))
      sbj_obj_krn_vector = np.zeros((1,vec_size * vec_size)) 

      for sbj_idx, obj_idx in sbj_obj[found_idx]:

        verb_sbj = dictionary[sbj_idx]
        verb_obj = dictionary[obj_idx]

        if word2vec:
          if verb_sbj not in w2v_reverse or verb_obj not in w2v_reverse:
            continue

          sbj_idx = w2v_reverse[verb_sbj]
          obj_idx = w2v_reverse[verb_obj]

        vs = vec_data[sbj_idx]
        vo = vec_data[obj_idx]

        sbj_obj_vector = np.add(sbj_obj_vector, np.add(vo,vs))
        
        tk = np.kron(vs,vo)
        sbj_obj_krn_vector = np.add(sbj_obj_krn_vector, tk)

      return (True, base_vector, sbj_obj_vector, sbj_obj_krn_vector)

  return (False,0)


def read_subjects(verb, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec=False) :
  ''' Read the subject file - each line is a verb subject list of numbers - indices into the DICTIONARY
  We don't read the entire file - we scoot to the index for the verb given
  Only called on intransitive verbs. '''

  for found_idx in subjects.keys():
  
    found_verb = dictionary[found_idx]

    if verb == found_verb: 
 
      base_vector = vec_data[found_idx]

      if word2vec:
        base_vector = vec_data[ w2v_reverse[verb] ]

      add_vector = np.zeros((1,vec_size))
      min_vector = np.zeros((1,vec_size))
      max_vector = np.zeros((1,vec_size))

      add_base_add_vector = np.zeros((1,vec_size))
      min_base_add_vector = np.zeros((1,vec_size))
      max_base_add_vector = np.zeros((1,vec_size))

      add_base_mul_vector = np.zeros((1,vec_size))
      min_base_mul_vector = np.zeros((1,vec_size))
      max_base_mul_vector = np.zeros((1,vec_size))

      krn_vector = np.zeros((1,vec_size * vec_size))
      krn_base_mul_vector = np.zeros((1,vec_size * vec_size))
      krn_base_add_vector = np.zeros((1,vec_size * vec_size))
      
      for sbj_idx in subjects[found_idx]:

        # Convert to the word2vec lookup
        verb_sbj = dictionary[sbj_idx]

        if word2vec:
          if verb_sbj not in w2v_reverse:
            continue

          sbj_idx = w2v_reverse[verb_sbj]

        # Now find the subject vectors
        vv = vec_data[sbj_idx]

        # Add vector math
        add_vector = np.add(add_vector,vv)
      
        # Kronecker product - we have add and mul versions
        krn_vector = np.kron(vv, vv) + krn_vector
      
        # Now do the min and max
        for i in range(0,vec_size):
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


def read_subjects_2(verb, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec=False) :
  
  for found_idx in subjects.keys():
  
    found_verb = dictionary[found_idx]

    if verb == found_verb: 
 
      base_vector = vec_data[found_idx]

      if word2vec:
        base_vector = vec_data[ w2v_reverse[verb] ]

      add_vector = np.zeros((1,vec_size))
      krn_vector = np.zeros((1,vec_size * vec_size))
      
      for sbj_idx in subjects[found_idx]:

        # Convert to the word2vec lookup
        verb_sbj = dictionary[sbj_idx]

        if word2vec:
          if verb_sbj not in w2v_reverse:
            continue

          sbj_idx = w2v_reverse[verb_sbj]

        # Now find the subject vectors
        vv = vec_data[sbj_idx]

        # Add vector math
        add_vector = np.add(add_vector,vv)
      
        # Kronecker product - we have add and mul versions
        krn_vector = np.kron(vv, vv) + krn_vector
      
      return (True, base_vector, add_vector, krn_vector )

  return (False,0)


def cosine_sim(v0, v1, vec_size) :
  ''' Determine the cosine similarity
  https://en.wikipedia.org/wiki/Cosine_similarity#Angular_distance_and_similarity
  Pass in the two vectors. ''' 

  dist = 0
  # Not needed in the trans case
  if v1.shape[0] == vec_size:
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
  ''' Kronecker cosine similarity.'''
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

def read_tensorflow(filepath):
  ''' Read the Tensorflow version - held as a numpy array
  Quite straight forward basically.'''

  vec_data = np.load(filepath)
  vec_size = vec_data.shape[1]

  return vec_data, vec_size

def read_binary(filepath, w2v_dictionary):

  ''' Read the binary file from word2vec, returning a numpy array
  we use the DICTIONARY created by tensorflow, because all the integer
  files, DICTIONARYs and verb/subject lookups are based on that.'''

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
    vec_size = int( tt[1] )

    for i in range(0,len(w2v_dictionary)):
      vectors.append( np.zeros(vec_size) )

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
      
      # Now read the numbers, should be vec_size of them (4 bytes probably)
      tidx = 0
      pidx = -1
      if word in w2v_reverse.keys():
        pidx = w2v_reverse[word]
      
      for i in range(0,vec_size):
        byte = f.read(4)
        num = struct.unpack('f',byte)

        if pidx != -1 and not skip:
          vectors[pidx].itemset(tidx, num[0])

        tidx += 1
      widx += 1

  vectors = np.vstack(vectors)
  
  return vectors, vec_size

def trans(verbs_to_check, verb_transitive,  dictionary, w2v_reverse, sbj_obj, vec_data, vec_size, word2vec=False):
  ''' transitive verb output.'''
  print ("verb0,verb1,base_sim,sbj_obj_sim,sbj_obj_add,sbj_obj_mul,sum_sbj_obj,sum_sbj_obj_mul,sum_sbj_obj_add, human_sim")


  cc = []
  hc = []

  for i in range(0,7):
    cc.append([])

  for verb0, verb1, sim in verbs_to_check:
  
    if not(verb0 in verb_transitive and verb1 in verb_transitive):
      continue
    
    r0 = read_sbj_obj(verb0, dictionary, w2v_reverse, sbj_obj, vec_data, vec_size, word2vec)
    r1 = read_sbj_obj(verb1, dictionary, w2v_reverse, sbj_obj, vec_data, vec_size, word2vec)

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


def intrans(verbs_to_check, verb_intransitive, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec=False):
  ''' Intransitive case.'''
  print ("verb0,verb1,base_sim,add_sim,min_sim,max_sim,add_add_sim,add_mul_sim,min_add_sim,min_mul_sim,max_add_sim,max_mul_sim,krn_sim,krn_add_sim,krn_mul_sim,human_sim")

  cc = []
  hc = []
  for i in range(0,13):
    cc.append([])

  for verb0, verb1, sim in verbs_to_check:
  
    if not(verb0 in verb_intransitive and verb1 in verb_intransitive):
      continue

    #print(verb0,verb1)
    r0 = read_subjects(verb0, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec)
    r1 = read_subjects(verb1, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec)

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


def trans_intrans(verbs_to_check, verb_transitive, dictionary, w2v_reverse, vec_data, vec_size, sbj_obj,  word2vec=False):
  ''' transitive vs intransitive '''
  print ("verb0,verb1,base,add,add_verb,mul_verb,krn,krn_add_verb,krn_mul_verb,human_sim")

  cc = []
  hc = []
  for i in range(0,7):
    cc.append([])

  for verb0, verb1, sim in verbs_to_check:
  
    r0 = []
    r1 = []
  
    if verb0 in verb_transitive:
      r0 = read_sbj_obj2(verb0, dictionary, w2v_reverse, vec_data, vec_size, sbj_obj, word2vec)
    else:
      r0 = read_subjects_2(verb0, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec)
 
    if verb1 in verb_transitive:
      r1 = read_sbj_obj2(verb1, dictionary, w2v_reverse, vec_data, vec_size, sbj_obj, word2vec)
    else:
      r1 = read_subjects_2(verb1, dictionary, w2v_reverse, vec_data, vec_size, subjects, word2vec)

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
  DICT_FILE = "dictionary.txt"
  SUBJECTS_FILE = "verb_subjects.txt"
  OBJECTS_FILE = "verb_objects.txt"
  SBJ_OBJ_FILE = "verb_sbj_obj.txt"
  W2V_DICT_FILE = "vocab.txt"
  SIM_FILE = "SimVerb-3000-test.txt"
  STATS_FILE = "sim_stats.txt"
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
  
  # Begin to read in all the files
  verb_transitive, verb_intransitive = read_sim_stats(BASE_DIR, STATS_FILE)
  verbs_to_check = read_sim_file(BASE_DIR, SIM_FILE)
  dictionary = read_dictionary(BASE_DIR, DICT_FILE)
  w2v_dictionary, w2v_reverse = read_w2v_dictionary(BASE_DIR, W2V_DICT_FILE)
  subjects = read_subjects_file(BASE_DIR, SUBJECTS_FILE)
 
  if word:
    vec_data, vec_size = read_binary(BASE_DIR + "/vectors.bin", w2v_dictionary)
  else:
    vec_data, vec_size = read_tensorflow(BASE_DIR + "/final_standard_embeddings.npy", w2v_dictionary)

  if transitive:
    sbj_obj = read_sbj_obj_file(BASE_DIR, SBJ_OBJ_FILE)
    trans(verbs_to_check, verb_transitive, dictionary, w2v_reverse, sbj_obj, vec_data, vec_size, word)
  elif both:
    read_sbj_obj_file(BASE_DIR, SBJ_OBJ_FILE)
    trans_intrans(verbs_to_check, verb_transitive, dictionary, w2v_reverse, vec_data, vec_size, sbj_obj, word)
  else:
    intrans(verbs_to_check, verb_intransitive, dictionary, w2v_reverse, vec_data, vec_size, subjects, word)
