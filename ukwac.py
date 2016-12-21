'''
An adaptation of the word2vec simple example from the Tensorflow page.
This version uses pre-processed files from the C++ side and uses blocks for the data
saving the checkpoints to file.

https://raw.githubusercontent.com/tensorflow/tensorflow/r0.11/tensorflow/examples/tutorials/word2vec/word2vec_basic.py

Set the various directories correctly and this program should just run

* Notes *

This looks like fun : http://projector.tensorflow.org/

Issues with overshooting could be related to this? https://stackoverflow.com/questions/33641799/why-does-tensorflow-example-fail-when-increasing-batch-size

I've replaced the standard gradient descent with an adapative as things are exploding a bit too much
https://stackoverflow.com/questions/33919948/how-to-set-adaptive-learning-rate-for-gradientdescentoptimizer

'''

import collections
import math
import os
import random
import zipfile
import sys
import numpy as np
import signal
import sys

from six.moves import xrange  # pylint: disable=redefined-builtin

import tensorflow as tf

from data_buffer import read_dictionary, find_integer_files, set_integer_files, read_freq, read_unk_count, generate_batch, read_total_size   

import data_buffer

from visualize import plot_with_labels

# Options and config
# Naughty globals!

DICTIONARY_FILE     = "dictionary.txt"
FREQ_FILE           = "freq.txt"
INTEGER_DIR         = "."
UNKNOWN_FILE        = "unk_count.txt"
TOTAL_FILE          = "total_count.txt"
OUT_DIR             = "."
CHECKPOINT_DIR      = "checkpoints"
LEARNING_RATE       = 1.0
BATCH_SIZE          = 256
EMBEDDING_SIZE      = 256       # Dimension of the embedding vector.
SKIP_WINDOW         = 5         # How many words to consider left and right.
NUM_SKIPS           = 2         # How many times to reuse an input to generate a label.
NUM_STEPS           = 15000000  # Rough guess at the maximum number of steps

NORMALIZED_EMBEDDINGS = []
STANDARD_EMBEDDINGS = []


def save_data():
  final_NORMALIZED_EMBEDDINGS = NORMALIZED_EMBEDDINGS.eval()
  np.save(BASE_DIR + "/final_normalized_embeddings", final_NORMALIZED_EMBEDDINGS)

  final_STANDARD_EMBEDDINGS = STANDARD_EMBEDDINGS.eval()
  np.save(BASE_DIR + "/final_standard_embeddings", final_STANDARD_EMBEDDINGS)


def signal_handler(signal, frame):
  print('You pressed Ctrl+C!')
  save_data()
  sys.exit(0)
 

if __name__ == "__main__" :

  signal.signal(signal.SIGINT, signal_handler)

  BASE_DIR = sys.argv[1] + "/"
  print("Running from base_dir:", BASE_DIR)

  # Read all the config options and perform setup

  print("Reading dictionary")
  dictionary, reverse_dictionary, vocabulary_size = read_dictionary(BASE_DIR + DICTIONARY_FILE) 
  data_files, size_files = find_integer_files(BASE_DIR + INTEGER_DIR)
  count, count_order = read_freq(BASE_DIR + FREQ_FILE, vocabulary_size)

  # Wacky removes the most 100 common so we adjust the count_order accordingly
  count_order = count_order[100:]

  count["UNK"] = read_unk_count(BASE_DIR + UNKNOWN_FILE)

  print("Vocabularly of size", vocabulary_size)

  print("Reading integer data files")
  set_integer_files(data_files, size_files)
  
  print("Reading total data size")
  data_size = read_total_size(BASE_DIR + TOTAL_FILE)

  valid_size = 32       # Random set of words to evaluate similarity on.
  valid_window = 2000   # Only pick dev samples in the head of the distribution.
  valid_examples = []
  for i in np.random.choice(valid_window, valid_size, replace=False):
    valid_examples.append(dictionary[count_order[i]])

  print("Valid examples - first 20. Drawn from top 2000 most frequent words")
  for i in range(20):
    print(reverse_dictionary[valid_examples[i]])

  num_sampled = 128     # Number of negative examples to sample.

  # Begin the Tensorflow Setup
  # Generate a batch and print it
  batch, labels, valid_batch = generate_batch(BATCH_SIZE, NUM_SKIPS, SKIP_WINDOW)  
  for i in range(16):
    try:
      print(batch[i], reverse_dictionary[batch[i]], '->', labels[i, 0], reverse_dictionary[labels[i, 0]])
    except:
      print(labels[i])

  # Begin graph creation
  graph = tf.Graph()

  checkpoint_file = tf.train.latest_checkpoint(BASE_DIR + CHECKPOINT_DIR)
  
  # Setup the Tensorflow Graph itself

  with graph.as_default():
    # Input data.
    train_inputs = tf.placeholder(tf.int32, shape=[BATCH_SIZE], name="inputs")
    train_labels = tf.placeholder(tf.int32, shape=[BATCH_SIZE, 1], name="labels")
    valid_dataset = tf.constant(valid_examples, dtype=tf.int32, name="valid_dataset")

    with tf.device("/cpu:0"):
      # Look up STANDARD_EMBEDDINGS for inputs.
      STANDARD_EMBEDDINGS = tf.Variable(tf.random_uniform([vocabulary_size, EMBEDDING_SIZE], -1.0, 1.0), name="STANDARD_EMBEDDINGS")
      embed = tf.nn.embedding_lookup(STANDARD_EMBEDDINGS, train_inputs)
      
      # Save progress
      checkpoint_dir = os.path.abspath(os.path.join(BASE_DIR + OUT_DIR, "checkpoints"))
      checkpoint_prefix = os.path.join(checkpoint_dir, "model")
      
      if not os.path.exists(checkpoint_dir):
        os.makedirs(checkpoint_dir)

      # Construct the variables for the NCE loss
      nce_weights = tf.Variable(tf.truncated_normal([vocabulary_size, EMBEDDING_SIZE],stddev=1.0 / math.sqrt(EMBEDDING_SIZE)))
      nce_biases = tf.Variable(tf.zeros([vocabulary_size]),name="biases")

      # Compute the average NCE loss for the batch.
      # tf.nce_loss automatically draws a new sample of the negative labels each
      # time we evaluate the loss.

      #loss = tf.reduce_mean(tf.nn.sampled_softmax_loss(nce_weights, nce_biases, embed, train_labels, num_sampled, vocabulary_size), name="loss")

      loss = tf.reduce_mean(tf.nn.nce_loss(nce_weights, nce_biases, embed, train_labels, num_sampled, vocabulary_size))
      
      # Construct the an optimizer that works proper like
      #optimizer = tf.train.GradientDescentOptimizer(LEARNING_RATE, name='optimizer').minimize(loss)

      optimizer = tf.train.AdagradOptimizer(LEARNING_RATE, name='optimizer').minimize(loss)

      # Compute the cosine similarity between minibatch examples and all STANDARD_EMBEDDINGS.
      norm = tf.sqrt(tf.reduce_sum(tf.square(STANDARD_EMBEDDINGS), 1, keep_dims=True))
      NORMALIZED_EMBEDDINGS = STANDARD_EMBEDDINGS / norm
      valid_standard_embeddings = tf.nn.embedding_lookup(NORMALIZED_EMBEDDINGS, valid_dataset)
      similarity = tf.matmul(valid_standard_embeddings, NORMALIZED_EMBEDDINGS, transpose_b=True, name="similarity")

      # Global step variable
      global_step =  tf.Variable(0, name="global_step")

      saver = tf.train.Saver()
      
      # Add variable initializer.
      init = tf.initialize_all_variables()

  # Step 5: Begin training.
  
  with tf.Session(graph=graph) as session:
    # We must initialize all variables before we use them.
    init.run()
    print("Initialized")
    
    step = 0

    ckpt = tf.train.get_checkpoint_state(BASE_DIR + CHECKPOINT_DIR)
    if ckpt and ckpt.model_checkpoint_path:
      print(ckpt.model_checkpoint_path)
      saver.restore(session,ckpt.model_checkpoint_path) 
      print ("reloaded session")
      # this doesnt work which is really annoying
      #step = global_step.eval(session)

    else:
      print ("Could not loaded checkpoint for some reason")

    average_loss = 0
    while step < NUM_STEPS:
      batch_inputs, batch_labels, valid_batch = generate_batch( BATCH_SIZE, NUM_SKIPS, SKIP_WINDOW)
      
      # If its not a valid batch then I suspect we need to stop before the noise contrastive explodes
      if not valid_batch:
        print ("Finished after",step,"steps")
        save_data()
        sys.exit()
      
      feed_dict = {train_inputs : batch_inputs, train_labels : batch_labels}

      # We perform one update step by evaluating the optimizer op (including it
      # in the list of returned values for session.run()
      _, loss_val = session.run([optimizer, loss], feed_dict=feed_dict)
      average_loss += loss_val

      if step % 2000 == 0:
        if step > 0:
          average_loss /= 2000
        if average_loss < 0.4 :
          print("Loss below 0.4. Quitting")
          break

        if average_loss > 50000:
          print("loss is exploding")
          sys.exit()
 
        # The average loss is an estimate of the loss over the last 2000 batches.
        print("Average loss at step ", step, ": ", average_loss, "Block:", data_buffer._current_block)
        average_loss = 0

      # Note that this is expensive (~20% slowdown if computed every 500 steps)
      if step % 100000 == 0:
        sim = similarity.eval()
        for i in xrange(valid_size):
          valid_word = reverse_dictionary[valid_examples[i]]
          top_k = 8 # number of nearest neighbors
          nearest = (-sim[i, :]).argsort()[1:top_k+1]
          log_str = "Nearest to %s:" % valid_word
          for k in xrange(top_k):
            try:
              close_word = reverse_dictionary[nearest[k]]
              log_str = "%s %s," % (log_str, close_word)
            except:
              print("Problem in closelookup", k)
          print(log_str)

      if step % 10000 == 0:
        path = saver.save(session, checkpoint_prefix, global_step=step)
        print("Saved model checkpoint to {}\n".format(path))
      
      step += 1

    save_data() 
