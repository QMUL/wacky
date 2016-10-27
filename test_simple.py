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

print("Reading integer data files")
set_integer_files(data_files, size_files)
  
print("Reading total data size")
data_size = read_total_size(TOTAL_FILE)

for i in range(1,10):
  sentence = random_sentence()

  translated = ""
  for word in sentence:
    translated += reverse_dictionary[word] + " "

  print(translated)


for idx in count:
  if idx[0] == 'increased' :
    print(count[idx])
    break

print(dictionary['increased'])

import sys
sys.exit()


# Tensorflow tests

checkpoint_file = tf.train.latest_checkpoint(CHECKPOINT_DIR)

graph = tf.Graph()

with graph.as_default():
  sess = tf.Session()

  with sess.as_default():
    # Load the saved meta graph and restore variables
    saver = tf.train.import_meta_graph("{}.meta".format(checkpoint_file))
    saver.restore(sess, checkpoint_file)

    # Get the placeholders from the graph by name
    #input_x = graph.get_operation_by_name("input_x").outputs[0]
    # input_y = graph.get_operation_by_name("input_y").outputs[0]
    #dropout_keep_prob = graph.get_operation_by_name("dropout_keep_prob").outputs[0]

    # Tensors we want to evaluate
    #predictions = graph.get_operation_by_name("output/predictions").outputs[0]

    # Generate batches for one epoch
    #batches = batch_iter(list(x_test), FLAGS.batch_size, 1, shuffle=False)

    # Collect the predictions here
    #all_predictions = []

    #for x_test_batch in batches:
    #    batch_predictions = sess.run(predictions, {input_x: x_test_batch, dropout_keep_prob: 1.0})
    #    all_predictions = np.concatenate([all_predictions, batch_predictions])
    
    #print (all_predictions)
