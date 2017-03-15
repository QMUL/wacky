#!/usr/bin/python

'''
Perform the correlation and other such stats on our produced CSV files
Mostly used for the count vectors
'''

import numpy as np
import os, sys, math, struct, codecs

from scipy import stats


def read_dictionary(BASE_DIR, DICT_FILE) :
  #print("Reading Dictionary")
  dictionary = []
  with open(BASE_DIR + "/" + DICT_FILE,'r') as f:
    for line in f.readlines():
      dictionary.append( line.replace("\n",""))

  idx = 0
  rdictionary = {}
  for d in dictionary:
    rdictionary[d] = idx
    idx+=1

  return dictionary, rdictionary

def read_sim_file(BASE_DIR, SIM_FILE):
  verbs_to_check_paired = []
  verbs_to_check = []

  with open(BASE_DIR + "/" + SIM_FILE,'r') as f:
    for line in f.readlines():
      tokens = line.split()
      
      if len(tokens) > 4:
        verbs_to_check_paired.append( (tokens[0], tokens[1], float(tokens[3]), tokens[4] ) )
      else:
        verbs_to_check_paired.append( (tokens[0], tokens[1], float(tokens[3]), "NONE" ) )

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


def read_csv_file(filepath): 
  with open(filepath,'r') as f:

    header = f.readline()

    titles = header.split(",")

    human = []
    cc = []
    num_compares = len(titles) - 3
    verbs0 = []
    verbs1 = []

    titles = titles[2:len(titles)-1]

    for i in range(0,num_compares):
      cc.append([])

    for line in f.readlines():
      tokens = line.split(",")
      verb0 = tokens[0]
      verb1 = tokens[1]


      # check to see if there any mistakes
      # Basically, are any of our factors 2.0?
      for i in range(0,num_compares-1):
        if float(tokens[2+i]) >= 2.0 or 'nan' in tokens[2+i] or float(tokens[2+i]) == 1.0:
          continue

      for i in range(0,num_compares):
        cc[i].append(float(tokens[2+i]))
    
      human.append(float(tokens[-1]))

      verbs0.append(verb0)
      verbs1.append(verb1)

  return cc, human, titles, verbs0, verbs1


def basic_rho(cc,human,titles):

  rho = []
  rho_sort = []
 
  for comp in cc:
    rho.append(stats.spearmanr(comp,human))
  
  idx = 0
  for r,pval in rho:
    rho_sort.append((titles[idx],r,pval))
    idx += 1

  # Sort out Rho
  rho_sort.sort(key=lambda tup: tup[1])
  rho_sort = rho_sort[::-1]

  print_rho(rho_sort)


# Here we drop these verb pairs that have too few or too many 
# subjects and objects based on an average value
def drop_rho(cc,human,titles,rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, avg, lower):

  rho = []
  rho_sort = []
  cc_trimmed = []
  human_trimmed = []

  for i in range(0,len(cc)):
    cc_trimmed.append([])
  
  idh = 0
  for h in human:
    if v0[idh] in rdictionary and v1[idh] in rdictionary:
      rd0 = rdictionary[v0[idh]]
      rd1 = rdictionary[v1[idh]]
      
      if lower:
        if rd0 in sbj_obj.keys() and rd1 in sbj_obj.keys(): 
          if len(sbj_obj[rd0]) <= avg and len(sbj_obj[rd1]) <= avg:
            for i in range(0,len(cc)):
              cc_trimmed[i].append(cc[i][idh])
            human_trimmed.append(h)

      else:
        if rd0 in sbj_obj.keys() and rd1 in sbj_obj.keys(): 
          if len(sbj_obj[rd0]) >= avg and len(sbj_obj[rd1]) >= avg:
            for i in range(0,len(cc)):
              cc_trimmed[i].append(cc[i][idh])
            human_trimmed.append(h)

    idh +=1

  for comp in cc_trimmed:
    rho.append(stats.spearmanr(comp,human_trimmed))
  
  idx = 0
  for r,pval in rho:
    rho_sort.append((titles[idx],r,pval))
    idx += 1

  # Sort out Rho
  rho_sort.sort(key=lambda tup: tup[1])
  rho_sort = rho_sort[::-1]

  print_rho(rho_sort)

# Here we keep or drop a specific word group 
def keep_rho_type(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, vtype, keep):

  rho = []
  rho_sort = []
  cc_trimmed = []
  human_trimmed = []

  for i in range(0,len(cc)):
    cc_trimmed.append([])

  idh = 0
  for h in human:
    if v0[idh] in rdictionary and v1[idh] in rdictionary:
      rd0 = rdictionary[v0[idh]]
      rd1 = rdictionary[v1[idh]]
     
      if keep:
        if rd0 in sbj_obj.keys() and rd1 in sbj_obj.keys():
          if vtype in verbs_paired[idh][3]:
            for i in range(0,len(cc)):
              cc_trimmed[i].append(cc[i][idh])
            human_trimmed.append(h)

      else:
        if rd0 in sbj_obj.keys() and rd1 in sbj_obj.keys():  
          if vtype not in verbs_paired[idh][3]:
            for i in range(0,len(cc)):
              cc_trimmed[i].append(cc[i][idh])
            human_trimmed.append(h)
      

    idh +=1

  for comp in cc_trimmed:
    rho.append(stats.spearmanr(comp,human_trimmed))
  
  idx = 0
  for r,pval in rho:
    rho_sort.append((titles[idx],r,pval))
    idx += 1

  # Sort out Rho
  rho_sort.sort(key=lambda tup: tup[1])
  rho_sort = rho_sort[::-1]

  print("total:", len(human_trimmed), len(human))
  print_rho(rho_sort)


# Print out the rho in order
def print_rho(rho_sort):
  for title,r,pval in rho_sort:
    sys.stdout.write("rho_" + title + ",")

  print("")
  
  for title,r,pval in rho_sort:
    sys.stdout.write(str(r) + ",")

  print("")

  for title,r,pval in rho_sort:
    sys.stdout.write("p_" + title + ",")

  print("")
  
  for title,r,pval in rho_sort:
    sys.stdout.write(str(pval) + ",")
  print("")



# The avg_rho where we see how many follow the alignment
# Basically what we do is see how many of the models fall above or below the average, compared
# with the human generated values. We then see how many subjects and objects each one had 
# and whether they were aligned or misaligned

def avg_rho (cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1):
  # Figure out these that are above and below the human average
  
  hum_avg = sum(human) / len(human)
  cpu_avg = []
  for i in range(0, len(cc)):
    cpu_avg.append( sum(cc[i]) / len(cc[i]))
 

  aligned = []
  misaligned = []
  
  v0a = []
  v0m = []
  v1a = []
  v1m = []

  for m in range(0,len(cc)):
    aligned.append([])
    misaligned.append([])
    v0a.append([]) 
    v0m.append([]) 
    v1a.append([]) 
    v1m.append([]) 
  
  for n in range(0,len(cc)):
    idh = 0
    for m in cc[n]:
     
      rd0 = rdictionary[v0[idh]]
      rd1 = rdictionary[v1[idh]]
      
      if (m < cpu_avg[n] and human[idh] < hum_avg) or (m > cpu_avg[n] and human[idh] > hum_avg):
        aligned[n].append(idh)
        if rd0 in sbj_obj.keys() and rd1 in sbj_obj.keys(): 
          v0a[n].append( len(sbj_obj[rd0]))
          v1a[n].append( len(sbj_obj[rd1]))
        else:
          v0a[n].append(0)
          v1a[n].append(0)

      else:
        misaligned[n].append(idh)

        if rd0 in sbj_obj.keys() and rd1 in sbj_obj.keys(): 
          v0m[n].append( len(sbj_obj[rd0]))
          v1m[n].append( len(sbj_obj[rd1]))
        else:
          v0m[n].append(0)
          v1m[n].append(0)
  
      idh += 1

   
  for m in range(0,len(cc)):
    
    a0 = a1 = a2 = a3 = 0

    try:
      a0 = sum(v0a[m]) / len(v0a[m])
      a1 = sum(v1a[m]) / len(v1a[m])

      a2 = sum(v0m[m]) / len(v0m[m])
      a3 = sum(v1m[m]) / len(v1m[m])
    except:
      pass

    print(titles[m], "a:", len(aligned[m]), "m:", len(misaligned[m]), "v0a:", a0, "v1a:", a1, "v0m:", a2, "v1m:", a3)

def hist(sbj_obj, dictionary, verbs_to_check):

  values = []
  avg = 0
  total = 0

  # Basic count
  for verb in sbj_obj.keys():

    if dictionary[verb] in verbs_to_check:
      total += 1    
      ll = len(sbj_obj[verb])
      avg += ll
      values.append(ll)
    
       
  avg = avg / float(total)

  values.sort()

  median = values[ int(math.floor(len(values) / 2))]
  ninefive = values[ int(math.floor(len(values) / 100.0 * 95.0)) ]
  five =  values[ int(math.floor(len(values) / 100.0 * 95.0)) ]

  return (median, ninefive, five)


if __name__ == "__main__" :
  
  cc, human, titles, v0, v1 = read_csv_file(sys.argv[1] )
  
  BASE_DIR = os.path.dirname(os.path.abspath(sys.argv[1]))
  DICT_FILE = "dictionary.txt"
  # Sim file must match the results file made from it
  SIM_FILE = "SimVerb-500-dev.txt"
  #SIM_FILE = "SimVerb-3000-test.txt"
  SBJ_OBJ_FILE = "verb_sbj_obj.txt"

  if len(sys.argv) > 2:
    dictionary, rdictionary = read_dictionary(BASE_DIR,DICT_FILE)
    verbs_check, verbs_paired = read_sim_file(BASE_DIR, SIM_FILE)
    sbj_obj = read_sbj_obj_file(BASE_DIR, SBJ_OBJ_FILE)
    
    if sys.argv[2] == '-a':
      # Split based on the averages
      avg_rho(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1)
    elif sys.argv[2] == '-l':  
      avg = sum( [len(sbj_obj[x]) for x in sbj_obj.keys()] )  / len(sbj_obj)
      print("cutoff:",avg)
      drop_rho(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, avg, True)
    elif sys.argv[2] == '-h':  
      avg = sum( [len(sbj_obj[x]) for x in sbj_obj.keys()] )  / len(sbj)
      print("cutoff:",avg)
      drop_rho(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, avg, False)
 
    elif sys.argv[2] == '-ql':
      m, n, f = hist(sbj_obj, dictionary, verbs_check)
      print("cutoff:",n)
      drop_rho(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, n, True)
 
    elif sys.argv[2] == '-qh':  
      m, n, f = hist(sbj_obj, dictionary, verbs_check)
      print("cutoff:",f)
      drop_rho(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, f, False)
 
    elif sys.argv[2] == '-tk':
      if len(sys.argv) > 3:
        keep_rho_type(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, sys.argv[3], True)

    elif sys.argv[2] == '-td':
      if len(sys.argv) > 3:
        keep_rho_type(cc, human, titles, rdictionary, verbs_check, verbs_paired, sbj_obj, v0, v1, sys.argv[3], False)


  else:
    basic_rho(cc,human,titles)


