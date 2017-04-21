  # Wacky

The Wacky project is a set of tools for working with the ukwac dataset. It consists of 

* A C++ program that does a lot of the heavy lifting 
* A set of python scripts for analysis and further processing
* A set of scripts for submission to QMUL Apocrita and other HPC services
* Singularity definitions for creating containers to perform Tensorflow training
* A Tensorflow word2vec example using wacky data.

# wacky program

Wacky is a C++ program that can be built with CMake. It uses Boost's memory mapping to map the large ukWaC data files, resulting in a lower memory usage. There is an early draft of a CUDA version of the program but this is incomplete and should be ignored.

Wacky creates a lot of files in its *working directory*. These files are summaries and conversions for use with later stages of wacky, or in other programs such as [word2vec](https://code.google.com/archive/p/word2vec) and [Tensorflow](https://www.tensorflow.org/tutorials/word2vec). 

## Building Wacky

Simply create a directory and run cmake as you normally would. Example:

    mkdir build
    cd build
    cmake ..

### requirements

* Boost Libraries - needed for memory mapped I/O.
* OpenMP - splits processing across local cores. Increases speed dramatically.
* Intel Math Library - this gives a performance boost. The default routines are much slower
* GCC or Intel Compiler. Clang has not been tested but I don't see a reason why it shouldn't.
* Python3 if you want to generate stats and further models.
* A copy of the ukwac corpus.

### Intel MKL

The [Intel's Math Library](https://software.intel.com/en-us/intel-mkl/) should be used for extra speed when working out Kronecker products and matrix multiplication. I haven't yet tested the claims that MKL is much faster but it seems to be. 

## Running Wacky

Depending on the command line flags, wacky can perform several operations on the ukwac dataset. For example...

    ./wacky -u ~/ukwac -l -v 50000 -o ~/output

... will scan the directory ~/ukwac for the ukwac files themselves. It will then proceed to create a set of summary files, such as the dictionary, the frequency and the total counts into the directory ~/output. The dictionary will contain 50000 entries + the unknown entry. Finally, wacky will use the *lemmatized* versions of the word. For example, the word *working* will be converted / counted as *work*.

### Adapting to other Corpus

Wacky is designed to run with the ukwac corpus formatted in a particular way. ukWac entries look something like this:

    <text>
    <s>
    Last  last  JJ  1 4 NMOD
    updated update  VVN 2 4 NMOD
    December  December  NP  3 4 NMOD
    1999  @card@  CD  4 0 ROOT
    </s>
    </text>

This is the output created by the *MaltParser* apparently ([http://wacky.sslmit.unibo.it/doku.php?id=corpora](http://wacky.sslmit.unibo.it/doku.php?id=corpora)).

### Command line options

* u - the directory to the ukwac files
* y - use unique subjects when creating the subject-object files
* z - use unique objects when creating the subject-object files
* o - the output / working directory. Specify this directory when creating AND reading-in files.
* f - the name of the results file when running the subject-object models
* v - how big should the dictionary / vocab be?
* b - create the verb subject-object files
* a - do we want to run intransitive only models?
* d - do we want to run transitive only models?
* i - should we convert ukwac to integer lookups? This is what tensorflow requires.
* l - use the *lemmatized* form of all the words we lookup.
* j - how big is the window when we create our word vectors?
* r - Read in the dictionary, frequency and other summary files from the directory specified with -o
* s - the full path to the simverb file
* w - create word vectors
* e - How large is the basis / word-vector?
* g - How many of the most popular words should we ignore in our word vector?
* n - create sim files
* c - combine the ukwac files into one file
* p - run the count vector models

### wacky basic workflows

If starting from scratch, the first part of any workflow with wacky is to create the basic summary files. These are used by the various scripts as well as wacky itself.

    ./wacky -u ~/ukwac -l -v 500000 -o ~/output


An example for the next step - what if you want to create classic word vector counts for use with your models? To do that you would need to run the following:

    ./wacky -u ~/ukwac -l -o ~/output -r -w -j 5 -e 1000 -g 100

This creates a set of word vectors that are 1000 items long using a window of 5 and ignoring the top 100 most popular words.

The next command creates a set of integer files that represent the words as indices into the dictionary. 

    ./wacky -u ~/ukwac -r -l -o ~/output -i

This creates a whole load of lookup files (depending on how many cores your computer has). At this point, you can start to train your tensorflow model on the ukwac data, as well as the other models.
Alternatively, lets say you wish to use the original word2vec program. You would have to combine all the files together into one.

    ./wacky -u ~/ukwac -o ~/output -c


### wacky subject-object workflows

Wacky can also work on the verbs in the ukwac dataset and create summaries and vectors based on these verbs' subjects and objects. Wacky considers these words whose tag contains *VV* to be verbs. Words tagged with *NN* or *JJ* are accepted as subjects or objects of these verbs.

The first thing to do is create a list of all the verbs, their subjects and their objects.

    ./wacky -u ~/ukwac -o ~/output -r -l -b -n -s ~/simverb.txt

This creates a large file - *verb_sbj_obj.txt* that contains the index of the verb and the indicies of it's subjects and/or objects. We also get a sim_stats.txt file that contains the counts of subjects and objects for each verb that is fiven in the simverb.txt file.

Note that we *have* to provide a list of verbs for which we are generating the statistics on whether or not a verb is transitive or intransitive. The -n and -s flags are used for this purpose. 

The -s parameter is a link to one of the SimVerb files you can find at [Daniela Gerz's page at Cambridge University](http://people.ds.cam.ac.uk/dsg40/simverb.html) - it is a list of verb pairs and ratings that we use to train and test our models.

Note that this incantation allows for duplicates in the subject/object list. If we only want to include a subject or object once we would call:

    ./wacky -u ~/ukwac -o ~/output -r -l -b -y -z -n -s ~/simverb.txt

With our subject and object file created we can begin to work on the various models we might have. If you want to work with the tensorflow or word2vec models, you need to use the included python scripts. If you are working with the wacky-created word vectors, you can run the models like so:

    ./wacky -o ~/output -r -l -p -s ~/simverb.txt

## Summary Files

Depending on the workflow, Wacky can be used to create summary files that can then be used in programs written in R, Python, or any other language. Wacky creates files such as (but not limited to):

* freq.txt - the frequency of all the words.
* dictionary.txt - the words we accept into our dictionary of a certain size
* unk_count.txt - the total number of words not in the dictionary
* total_count.txt - how many words does ukwac contain?
* word_vectors.txt - the file of count vectors for all the words in ukwac

These are perhaps the most useful. In order to create a complete set of summary files and word count vectors, one needs to run twice as follows:
 
    ./wacky -u ~/ukwac -l -v 500000 -o ~/output
    ./wacky -u ~/ukwac -l -o ~/output -r -w -j 5 -e 1000 -g 100

As mentioned previously, these lines will create a set of initial summary files, a set of integer lookups and a set of word vectors with the specified sizes and windows. Inside the *output* directory you should see a set of files with formats given below. On a reasonable desktop machine, this is likely to take at least 1 hour (ukwac is quite large).

### File Formats

The following files are created by Wacky with the following formats:

#### basis.txt

A set of indices into the dictionary, 1 per line, showing which words form the basis.

#### dictionary.txt

A list of words, one per line, that form the dictionary. The line number is the index, starting at 0.

#### freq.txt

The frequency of all the *unique* tokens in ukwac. This is affected by the *lemmatize* flag. Each line starts with a word, followed by a comma then a space, then a number. The order is alphabetical

#### integers_xxxxxx.txt

A whole set of files starting with integers, and a series of numbers. The number depends on how many cores your machine has. Each file contains part of the corpus with all words replaced by their index into the dictionary. One number occurs per line. These files are mainly used with Tensorflow.

#### sim_stats.txt

Created when we look at the verb subjects and objects. Each line has a word and three numbers, each one separated by a space. The first number is the number of subjects, the second is the number of objects and the last number is the total.

#### size_xxxxxx.txt

These files list the number of numbers in each of the integers_xxxxx.txt files. One number on one line.

#### total_count.txt

One number that lists the total number of words in ukWaC.

#### unk_count.txt

How many words did not fall in the dictionary and were therefore classed as unknown. One number on one line.

#### verb_objects.txt

Each line represents a verb. The first number is the index of this verb in the dictionary. The following numbers are the indices of all the objects of that verb. The numbers are separated by a space.

#### verb_subjects.txt

Same as the above but for subjects.

#### verb_sbj_obj.txt

Same as the above but for both subjects and objects.

#### word_vectors.txt

The word vectors for every word in the dictionary. The first number on each line is that word's index into the dictionary. All following numbers are the raw counts, separated by a space. One can take the order of these numbers and check against basis.txt and dictionary.txt to find out which word is represented.

# Tensorflow training

The tensorflow setup is a little more complicated. Assuming you have tensorflow, matplotlib, scikit learn and the rest installed, run

    python3 ukwac.py /path_to_data

Inside this file there are the paths to the files output by Wacky. You'll need to set these to where the Wacky output is, relative to where you are running ukwac.py from.

This program is set to run off the cpu but you can alter the script to run off the GPU instead which is a little faster.

Depending on the size of your dictionary, the [Tensorflow word2vec example](https://www.tensorflow.org/tutorials/word2vec) will take between 1 and 15 days to complete (roughly). When it completes it will create two files called *final_embeddings.npy* and *final_normalized_embeddings.npy*. These represent the final vectors and the normalised final vectors respectively. These are held in numpy format and can be read straight back into a numpy array using the numpy functions.

## Requirements

This script assumes you have Tensorflow installed and working correctly on your system. You can easily modify the script to use GPU or CPU.

# word2vec original training

It is possible to train word2vec on ukwac using the intermidiate files created above. Just point word2vec at the combined file of ukwac text and it will do the rest for you.

# Python Scripts for models

If you are running models based on either tensorflow or word2vec (as opposed to the wacky created count vectors), you will need to use one or more of the python based scripts.

## Requirements

* scikit-learn
* numpy
* scipy
* python 3 and above


## Scripts

* verbs_binary.py - run either the intransitive or transitive models on either the tensorflow or word2vec data.
* stats.py - generate some statistics on our verb subject-object models.
* verbs_both.py - run the final models on either tensorflow or word2vec data.
* probs.py - given count vectors, what is the probabilty of one word, given another word.
* closest.py - which words are closest to other words?

### Workflows

Say you have generated a Tensorflow model and you want to run the set of intransitive verb models against this dataset. You would run...

    python3 verbs_binary.py ~/tensorflow_results

You can change to the transitive models by passing the *-t* flag. It is expected that other files such as the dictionary, frequency and so forth, are also in this same directory.

The final models that deal with both kinds of verbs can be excuted thusly:

    python3 verbs_both.py ~/tensorflow_results

If you want to process the word2vec results you can pass the *-b* flag. 


# Singularity

Included is a .def file for creating your singularity container for the tensorflow part of the setup. It will create an ubuntu image, install tensorflow and run ukwac.py.

You need to have cuda version 367.57 installed and running on the host. This container def is quite specific and will need tweeking to fit your setup. It's a work in progress but it does work for most purposes.

To build the image first run:

    sudo singularity create --size 8000 wackyvec.img

Then perform the bootstrapping

    sudo singularity bootstrap wackyvec.img wackyvec.def

Finally, run the container with the provided data directory mapped to /data

    singularity run -B /home/oni/Projects/WackyVec/build:/data wackyvec.img

# Acknowledgements

The simverb.txt test file is taken from the SimVerb 3500 dataset:

[http://people.ds.cam.ac.uk/dsg40/simverb.html](http://people.ds.cam.ac.uk/dsg40/simverb.html)

    SimVerb-3500: A Large-Scale Evaluation Set of Verb Similarity
    Daniela Gerz, Ivan Vulić, Felix Hill, Roi Reichart and Anna Korhonen. EMNLP 2016.

The test ukwac files are taken from ukWaC:

[http://wacky.sslmit.unibo.it/doku.php](http://wacky.sslmit.unibo.it/doku.php)

    M. Baroni, S. Bernardini, A. Ferraresi and E. Zanchetta. 2009. The WaCky Wide Web: A Collection of 
    Very Large Linguistically Processed Web-Crawled Corpora. Language Resources and Evaluation 43 (3): 209-226.


