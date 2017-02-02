import numpy as np
import sys
from scipy.stats import spearmanr

def read_final(path):
  models = []
  headings = []
  
  with open(path,'r') as f:
    
    header = f.readline() 
    tokens = header.replace("\n","").split(",")
    headings = tokens[2:]

    for i in range(0,len(tokens)-2):
      models.append([])

    for line in f.readlines():
      if "----" in line:
        break

      tokens = line.replace("\n","").split(",")
      
      for i in range(0,len(tokens)-2):
        models[i].append(float(tokens[i+2]))

  return models, headings


'''
M1, M2 : the lists of scores for the two models to be compared,
H      : the list of human judgements
it     : number of iterations

Returns 1-tailed and 2-tailed p-scores
'''

def perm_test(M1,M2,H, it=500):
  l = len(M1)
  orig_dif = spearmanr(H,M1)[0] - spearmanr(H,M2)[0]
  perm_list = []
  pdif_list = []

  for i in range(it):
    perm = np.random.binomial(1,0.5,l).tolist()

    while perm in perm_list:
      perm = np.random.binomial(1,0.5,l).tolist()
		
    perm_list.append(perm)
		
    M1_perm = [M2[i] if perm[i] else M1[i] for i in range(l)]
    M2_perm = [M1[i] if perm[i] else M2[i] for i in range(l)]
    perm_dif = spearmanr(H,M1_perm)[0] - spearmanr(H,M2_perm)[0]
    pdif_list.append(perm_dif)

  pd_arr = np.array(pdif_list)
  p1 = np.mean(pd_arr >= orig_dif)               # 1-tailed p-score
  p2 = np.mean(np.abs(pd_arr) >= abs(orig_dif))  # 2-tailed p-score

  return (p1,p2)

if __name__ == "__main__":

  models, headings = read_final(sys.argv[1])

  sys.stdout.write("_,")
  for i in range(0,len(headings)):
    sys.stdout.write(headings[i] + "-p1," + headings[i] + "-p2,")
  
  print("")
  
  for i in range(0,len(models)):
    
    sys.stdout.write(headings[i]+",")

    for j in range(0, len(models)):

      M1 = models[i]
      M2 = models[j]
      H = models[-1]
      p1,p2 = perm_test(M1,M2,H)
      sys.stdout.write(str(p1) + "," + str(p2) + ",")
    
    print("")
  
  print("")
