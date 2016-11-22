''' 
Visualise the output with matplot lib
'''
import numpy as np
import matplotlib.pyplot as plt

from sklearn.manifold import TSNE
from data_buffer import read_dictionary, find_integer_files, read_freq, read_unk_count

import sys,os

DICTIONARY_FILE     = "dictionary.txt"
FREQ_FILE           = "freq.txt"
INTEGER_DIR         = "."
UNKNOWN_FILE        = "unk_count.txt"
TOTAL_FILE          = "total_count.txt"


# Visualize the embeddings.

def plot_with_labels(low_dim_embs, labels, filename='tsne.png'):
  assert low_dim_embs.shape[0] >= len(labels), "More labels than embeddings"
  plt.figure(figsize=(36, 36))  #in inches
  for i, label in enumerate(labels):
    x, y = low_dim_embs[i,:]
    plt.scatter(x, y)
    plt.annotate(label, xy=(x, y), xytext=(5, 2), textcoords='offset points', ha='right', va='bottom')

  plt.savefig(filename)


if __name__ == "__main__":
  
  BASE_DIR = sys.argv[1] + "/"

  print("Reading dictionary")
  dictionary, reverse_dictionary, vocabulary_size = read_dictionary(BASE_DIR + DICTIONARY_FILE) 
 
  data_files, size_files = find_integer_files(BASE_DIR + INTEGER_DIR)
  count = read_freq(BASE_DIR + FREQ_FILE, vocabulary_size)
  count[0][1] = read_unk_count(BASE_DIR + UNKNOWN_FILE)

  tsne = TSNE(perplexity=30, n_components=2, init='pca', n_iter=8000)
  plot_only = 2000

  final_embeddings = np.load("final_embeddings.npy")
  
  # Find interesting words and their embeddings and use these
 
  offset = 0
  useful_embeddings = []
  labels = []
  for word in count[offset:offset+plot_only]:  
    useful_embeddings.append( final_embeddings[ dictionary[ word[0]] ])
    labels.append(word[0])

  low_dim_embs = tsne.fit_transform(useful_embeddings)
  plot_with_labels( low_dim_embs, labels, filename=BASE_DIR + "tsne.png")

