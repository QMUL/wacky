'''
An adaptation of the word2vec simple example from the Tensorflow page.
This version uses pre-processed files from the C++ side and uses blocks for the data
saving the checkpoints to file.

https://raw.githubusercontent.com/tensorflow/tensorflow/r0.11/tensorflow/examples/tutorials/word2vec/word2vec_basic.py

Set the various directories correctly and this program should just run

'''

import collections
import math
import os
import random
import zipfile

import numpy as np

from six.moves import xrange  # pylint: disable=redefined-builtin
from sklearn.manifold import TSNE

import tensorflow as tf

from data_buffer import read_dictionary, find_integer_files, set_integer_files, read_freq, read_unk_count, generate_batch, read_total_size   

from visualize import plot_with_labels

# Options and config
DICTIONARY_FILE     = "./build/dictionary.txt"
FREQ_FILE           = "./build/freq.txt"
INTEGER_DIR         = "./build"
UNKNOWN_FILE        = "./build/unk_count.txt"
TOTAL_FILE          = "./build/total_count.txt"
OUT_DIR             = "."

if __name__ == "__main__" :

  # Read all the config options and perform setup

  print("Reading dictionary")
  dictionary, reverse_dictionary, vocabulary_size = read_dictionary(DICTIONARY_FILE) 
  data_files, size_files = find_integer_files(INTEGER_DIR)
  count = read_freq(FREQ_FILE, vocabulary_size)
  count[0][1] = read_unk_count(UNKNOWN_FILE)

  print("Reading integer data files")
  set_integer_files(data_files, size_files)
  
  print("Reading total data size")
  data_size = read_total_size(TOTAL_FILE)

  print("Top 100 words")
  print(count[:100]) 

  batch_size = 256
  embedding_size = 256  # Dimension of the embedding vector.
  skip_window = 5       # How many words to consider left and right.
  num_skips = 2         # How many times to reuse an input to generate a label.
  num_steps = 1000001

  # We pick a random validation set to sample nearest neighbors. Here we limit the
  # validation samples to the words that have a low numeric ID, which by
  # construction are also the most frequent.
  valid_size = 16     # Random set of words to evaluate similarity on.
  valid_window = 100  # Only pick dev samples in the head of the distribution.
  valid_examples = np.random.choice(valid_window, valid_size, replace=False)
  num_sampled = 128    # Number of negative examples to sample.

  # Begin the Tensorflow Setup

  batch, labels = generate_batch(batch_size=8, num_skips=2, skip_window=1)
  
  for i in range(8):
    try:
      print(batch[i], reverse_dictionary[batch[i]], '->', labels[i, 0], reverse_dictionary[labels[i, 0]])
    except:
      print(labels[i])

  graph = tf.Graph()

  # Setup the Tensorflow Graph itself

  with graph.as_default():
    # Input data.
    train_inputs = tf.placeholder(tf.int32, shape=[batch_size])
    train_labels = tf.placeholder(tf.int32, shape=[batch_size, 1])
    valid_dataset = tf.constant(valid_examples, dtype=tf.int32)

    with tf.device("/cpu:0"):
      # Look up embeddings for inputs.
      embeddings = tf.Variable(tf.random_uniform([vocabulary_size, embedding_size], -1.0, 1.0))
      embed = tf.nn.embedding_lookup(embeddings, train_inputs)
      
      # Save progress
      checkpoint_dir = os.path.abspath(os.path.join(OUT_DIR, "checkpoints"))
      checkpoint_prefix = os.path.join(checkpoint_dir, "model")
      
      if not os.path.exists(checkpoint_dir):
        os.makedirs(checkpoint_dir)

      saver = tf.train.Saver(tf.all_variables())
      
      # Construct the variables for the NCE loss
      nce_weights = tf.Variable(tf.truncated_normal([vocabulary_size, embedding_size],stddev=1.0 / math.sqrt(embedding_size)))
      nce_biases = tf.Variable(tf.zeros([vocabulary_size]))

      # Compute the average NCE loss for the batch.
      # tf.nce_loss automatically draws a new sample of the negative labels each
      # time we evaluate the loss.
      loss = tf.reduce_mean(tf.nn.nce_loss(nce_weights, nce_biases, embed, train_labels, num_sampled, vocabulary_size))
      
      # Construct the SGD optimizer using a learning rate of 1.0.
      optimizer = tf.train.GradientDescentOptimizer(1.0).minimize(loss)

      # Compute the cosine similarity between minibatch examples and all embeddings.
      norm = tf.sqrt(tf.reduce_sum(tf.square(embeddings), 1, keep_dims=True))
      normalized_embeddings = embeddings / norm
      valid_embeddings = tf.nn.embedding_lookup(normalized_embeddings, valid_dataset)
      similarity = tf.matmul(valid_embeddings, normalized_embeddings, transpose_b=True)

      # Add variable initializer.
      init = tf.initialize_all_variables()

  # Step 5: Begin training.

  with tf.Session(graph=graph) as session:
    # We must initialize all variables before we use them.
    init.run()
    print("Initialized")

    average_loss = 0
    for step in xrange(num_steps):
      batch_inputs, batch_labels = generate_batch( batch_size, num_skips, skip_window)
      feed_dict = {train_inputs : batch_inputs, train_labels : batch_labels}

      # We perform one update step by evaluating the optimizer op (including it
      # in the list of returned values for session.run()
      _, loss_val = session.run([optimizer, loss], feed_dict=feed_dict)
      average_loss += loss_val

      if step % 2000 == 0:
        if step > 0:
          average_loss /= 2000
        
        # The average loss is an estimate of the loss over the last 2000 batches.
        print("Average loss at step ", step, ": ", average_loss)
        average_loss = 0

      # Note that this is expensive (~20% slowdown if computed every 500 steps)
      if step % 10000 == 0:
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

        path = saver.save(session, checkpoint_prefix, global_step=step)
        print("Saved model checkpoint to {}\n".format(path))

    final_embeddings = normalized_embeddings.eval()

    tsne = TSNE(perplexity=30, n_components=2, init='pca', n_iter=5000)
    plot_only = 500
  
    # Find interesting words and their embeddings and use these
    offset = 0
    useful_embeddings = []
    labels = []
    for word in count[offset:offset+plot_only]:  
      useful_embeddings.append( final_embeddings[ dictionary[ word[0]] ])
      labels.append(word[0])

    low_dim_embs = tsne.fit_transform(useful_embeddings)
    plot_with_labels(low_dim_embs, labels)

