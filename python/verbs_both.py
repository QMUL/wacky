#!/usr/bin/python

'''
Read the output from word2vec or tensorflow and run the 6 equations passed
the verb pairs we want to check

'''

import numpy as np
import os, sys, math, struct, codecs, verbs_read, verbs_math

from scipy import stats

def sum_subjects(vidx, verb, VEC_SIZE, SUBJECTS, DICTIONARY, VEC_DATA, W2V_REVERSE=[], word2vec=False) :

  '''Read the subject file - each line is a verb subject list of numbers - indices into the DICTIONARY
  We don't read the entire file - we scoot to the index for the verb given
  Only called on intransitive verbs. '''

  if vidx >= 0 and vidx < len(VEC_DATA): 

    base_vector = VEC_DATA[vidx]

    if word2vec:
      base_vector = VEC_DATA[ W2V_REVERSE[verb] ]

    add_vector = np.zeros((1,VEC_SIZE))
    krn_vector = np.zeros((1,VEC_SIZE * VEC_SIZE))
    
    for sbj_idx in SUBJECTS[vidx]:

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


def sum_subjects_objects(vidx, verb, VEC_SIZE, SBJ_OBJ, DICTIONARY, VEC_DATA, W2V_REVERSE=[], word2vec=False):
  '''Do the summing for the transitive verbs with both subjects and objects.'''
  if vidx >= 0 and vidx < len(VEC_DATA): 

    base_vector = VEC_DATA[ vidx ]

    if word2vec:
      base_vector = VEC_DATA[ W2V_REVERSE[verb] ]

    sbj_obj_vector = np.zeros((1,VEC_SIZE))
    sbj_obj_krn_vector = np.zeros((1,VEC_SIZE * VEC_SIZE)) 

    for sbj_idx, obj_idx in SBJ_OBJ[vidx]:

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


def do_the_maff(VEC_SIZE, SUBJECTS, DICTIONARY, SBJ_OBJ, VEC_DATA, W2V_REVERSE, VERBS_TO_CHECK, VERB_TRANSITIVE, word2vec=False):
  '''Perform the 6 equations summing over various things and printing.'''
  print ("verb0,verb1,base,cs1,cs2,cs3,cs4,cs5,cs6,human_sim")

  cc = []
  hc = []
  
  for i in range(0,7):
    cc.append([])

  for verb0, verb1, sim in VERBS_TO_CHECK:
    
    if verb0 not in DICTIONARY or verb1 not in DICTIONARY:
      continue

    sys.stdout.write(verb0 + "," + verb1 + ",")

    v0t = verb0 in VERB_TRANSITIVE
    v1t = verb1 in VERB_TRANSITIVE

    r0 = []
    r1 = []

    vidx0 = -1
    vidx1 = -1
    found = False
    for i in range(0,len(DICTIONARY)):
      if DICTIONARY[i] == verb0:
        vidx0 = i
        if found:
          break
        else:
          found = True
      if DICTIONARY[i] == verb1:
        vidx1 = i
        if found:
          break
        else:
          found = True

    if v0t and v1t:
      r0 = sum_subjects_objects(vidx0, verb0, VEC_SIZE, SBJ_OBJ, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)
      r1 = sum_subjects_objects(vidx1, verb1, VEC_SIZE, SBJ_OBJ, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)

    elif not v0t and not v1t:
      r0 = sum_subjects(vidx0, verb0, VEC_SIZE, SUBJECTS, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)
      r1 = sum_subjects(vidx1, verb1, VEC_SIZE, SUBJECTS, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)

    elif v0t and not v1t:
      r0 = sum_subjects_objects(vidx0, verb0, VEC_SIZE, SBJ_OBJ, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)
      r1 = sum_subjects(vidx1, verb1, VEC_SIZE, SUBJECTS, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)

    else:
      r0 = sum_subjects(vidx0, verb0, VEC_SIZE, SUBJECTS, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)
      r1 = sum_subjects_objects(vidx1, verb1, VEC_SIZE, SBJ_OBJ, DICTIONARY, VEC_DATA, W2V_REVERSE, word2vec)

    if r0[0] and r1[0]:

      base0 = r0[1]
      sum_sbj0 = r0[2]
      krn_sbj0 = r0[3]

      base1 = r1[1]
      sum_sbj1 = r1[2]
      krn_sbj1 = r1[3]
      
      cc[0].append(verbs_math.cosine_sim(base0, base1, VEC_SIZE))
      
      cc[1].append(verbs_math.cosine_sim(sum_sbj0, sum_sbj1, VEC_SIZE))
      cc[2].append(verbs_math.cosine_sim(np.add(sum_sbj0, base0), np.add(sum_sbj1, base1), VEC_SIZE)) 
      cc[3].append(verbs_math.cosine_sim(sum_sbj0 * base0, sum_sbj1 * base1, VEC_SIZE))

      cc[4].append(verbs_math.kron_sim(krn_sbj0, krn_sbj1))
      cc[5].append(verbs_math.kron_sim(np.add(krn_sbj0, np.kron(base0,base0)), np.add(krn_sbj1, np.kron(base1,base1))))
      cc[6].append(verbs_math.kron_sim( np.kron(base0,base0) * krn_sbj0, np.kron(base1,base1) * krn_sbj1  ))
 
      hc.append(sim)
      
      for i in range(0,len(cc)):
        sys.stdout.write(str(cc[i][-1]) + ",")
      
      print(str(sim)) 

  rho = []
  for i in range(0,len(cc)):
    rho.append(stats.spearmanr(cc[i],hc))
  
  print("----")
  print("rho_base,rho_c1,rho_c2,rho_c3,rho_c4,rho_c5,rho_c6")
  for i in range(0,6):
    r, pval = rho[i]
    sys.stdout.write(str(r) + ",")
  print(str(rho[-1][0]))

  print("p_base,p_c1,p_c2,p_c3,p_c4,p_c5,p_c6")

  for i in range(0,6):
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
  SIM_FILE = "SimVerb-500-dev.txt"
  STATS_FILE = "sim_stats.txt"

  word = False

  if len(sys.argv) > 2:
    if sys.argv[2] == '-b':
      word = True

  VERB_TRANSITIVE, VERB_INTRANSITIVE = verbs_read.read_sim_stats(BASE_DIR, STATS_FILE)
  VERBS_TO_CHECK = verbs_read.read_sim_file(BASE_DIR, SIM_FILE)
  DICTIONARY = verbs_read.read_dictionary(BASE_DIR, DICT_FILE)
  
  if word:
    W2V_DICTIONARY, W2V_REVERSE = verbs_read.read_w2v_dictionary(BASE_DIR, W2V_DICT_FILE)
    VEC_DATA, VEC_SIZE = verbs_read.read_binary(BASE_DIR + "/vectors.bin", VEC_SIZE, W2V_DICTIONARY, W2V_REVERSE )
  else:
    VEC_DATA, VEC_SIZE = verbs_read.read_tensorflow(BASE_DIR + "/final_standard_embeddings.npy")

  SUBJECTS = verbs_read.read_subjects_file(BASE_DIR, len(VEC_DATA), SUBJECTS_FILE)
  SBJ_OBJ = verbs_read.read_sbj_obj_file(BASE_DIR, len(VEC_DATA), SBJ_OBJ_FILE)


  for v0,v1,s in VERBS_TO_CHECK:
    if word:
      verbs_read.read_subject_object(v0, BASE_DIR, DICTIONARY, SUBJECTS_FILE, OBJECTS_FILE, VEC_DATA, VEC_SIZE, W2V_REVERSE, True)
      verbs_read.read_subject_object(v1, BASE_DIR, DICTIONARY, SUBJECTS_FILE, OBJECTS_FILE, VEC_DATA, VEC_SIZE, W2V_REVERSE, True)
    else:
      verbs_read.read_subject_object(v0, BASE_DIR, DICTIONARY, SUBJECTS_FILE, OBJECTS_FILE, VEC_DATA, VEC_SIZE)
      verbs_read.read_subject_object(v1, BASE_DIR, DICTIONARY, SUBJECTS_FILE, OBJECTS_FILE, VEC_DATA, VEC_SIZE)

  do_the_maff(VEC_SIZE, SUBJECTS, DICTIONARY, SBJ_OBJ, VEC_DATA, W2V_REVERSE, VERBS_TO_CHECK, VERB_TRANSITIVE, word2vec=word)
