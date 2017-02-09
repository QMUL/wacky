import numpy as np
import math

def cosine_sim(v0, v1, VEC_SIZE) :

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


