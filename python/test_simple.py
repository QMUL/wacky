''' 
A small test file to take a peek at the data we are working on with the simple example and the ukWaC
dataset. First, we look at the sentences and secondly, we test tensorflow proper
'''

import tensorflow as tf
import numpy as np
import os
import time
import datetime
import random

from tensorflow.contrib import learn

from data_buffer import read_dictionary, find_integer_files, set_integer_files, read_freq, read_unk_count, generate_batch, read_total_size, random_sentence   

from visualize import plot_with_labels

from sklearn.manifold import TSNE

# Options and config
DICTIONARY_FILE     = "./build/dictionary.txt"
FREQ_FILE           = "./build/freq.txt"
INTEGER_DIR         = "./build"
UNKNOWN_FILE        = "./build/unk_count.txt"
TOTAL_FILE          = "./build/total_count.txt"
OUT_DIR             = "."
CHECKPOINT_DIR = "./checkpoints"

test_words = ["flibble", "toast", "amenity", "ben", "computer"]

# Basic tests to check the data

dictionary, reverse_dictionary, vocabulary_size = read_dictionary(DICTIONARY_FILE) 
print ("Vocab Size:", vocabulary_size)

data_files, size_files = find_integer_files(INTEGER_DIR)
count = read_freq(FREQ_FILE, vocabulary_size)
count[0][1] = read_unk_count(UNKNOWN_FILE)

#print("Reading integer data files")
#set_integer_files(data_files, size_files)
  
#print("Reading total data size")
#data_size = read_total_size(TOTAL_FILE)

#for i in range(1,10):
#  sentence = random_sentence()

#  translated = ""
#  for word in sentence:
#    translated += reverse_dictionary[word] + " "

#  print(translated)


# Tensorflow tests

checkpoint_file = tf.train.latest_checkpoint(CHECKPOINT_DIR)

graph = tf.Graph()

with graph.as_default():
  sess = tf.Session()

  with sess.as_default():
    # Load the saved meta graph and restore variables
    saver = tf.train.import_meta_graph("{}.meta".format(checkpoint_file))
    saver.restore(sess, checkpoint_file)

    # Set variables to the same sort of level as the main program
    vocab_size = 50001
  
    emb_dim = 768

    init_width = 0.5 / emb_dim

    embeddings = tf.Variable( tf.random_uniform( [vocab_size, emb_dim], -init_width, init_width), name="emb")

    norm = tf.sqrt(tf.reduce_sum(tf.square(embeddings), 1, keep_dims=True))
    normalized_embeddings = embeddings / norm

    # Test data we want to play with
    a = [dictionary["mr"], dictionary["mrs"], dictionary["total"]]
    test_examples = np.array(a)
    test_dataset = tf.constant(test_examples, dtype=tf.int32)
    test_embeddings = tf.nn.embedding_lookup(normalized_embeddings, test_dataset)
    
    init = tf.initialize_all_variables()
    init.run()

    #vals, idx = sess.run([normalized_embeddings])

    #final_embeddings = normalized_embeddings.eval()

    similarity = tf.matmul(test_embeddings, normalized_embeddings, transpose_b=True)
    #sim = similarity.eval()
   
    #sess.run([similarity])

    #nearest = (-sim[i, :]).argsort()[1:10]
    #print(nearest)

    #nemb = tf.nn.l2_normalize(emb, 1)

    nearby_word = tf.placeholder(dtype=tf.int32)  # word id
    nearby_val, nearby_idx = tf.nn.top_k(similarity, min(1000, vocab_size))
    
    vals, idx = sess.run([nearby_val, nearby_idx], {nearby_word: a})

    print(vals,idx)

    final_embeddings = normalized_embeddings.eval()

    #print(len(vals))

    #for i in idx:
    #  print(reverse_dictionary[i[0]], reverse_dictionary[i[1]], reverse_dictionary[i[2]], vals[0])
    #
    
    tsne = TSNE(perplexity=30, n_components=2, init='pca', n_iter=8000)
    #plot_only = 2000
  
    # Find interesting words and their embeddings and use these
    #offset = 0
    useful_embeddings = [final_embeddings[a[0]], final_embeddings[a[1]], final_embeddings[a[2]]]
    labels = ["mr","mrs","total"]

    low_dim_embs = tsne.fit_transform(useful_embeddings)

    print(low_dim_embs)

    plot_with_labels(low_dim_embs, labels, filename="test.png")
