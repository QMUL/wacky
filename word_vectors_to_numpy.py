'''
A small program that reads the word_vector output from wacky and
converts it to a numpy array and then saves
'''
import sys
import numpy as np

VEC_SIZE = 50000

if __name__ == "__main__" :

  BASE_DIR = sys.argv[1] + "/"
  print("Running from base_dir:", BASE_DIR)
  full_path = BASE_DIR + "word_vectors.txt"
 
  word_vector_np = np.zeros((VEC_SIZE,VEC_SIZE))

  idx = 0
  with open(full_path,'r') as f:
    for line in f.readlines():
      ll = line.replace("\n","")
      tokens = line.split(" ")
      
      # We dont count the last two tokens as VEC_SIZE is the unk count
      jdx = 0
      for t in tokens[:VEC_SIZE]:
        word_vector_np[idx][jdx] = int(t)
        jdx += 1

      idx +=1

  np.save(BASE_DIR + "/final_standard_embeddings", word_vector_np)
