A pre-processor and Tensorflow example for training a neural-net on the ukWaC dataset. In addition, the program Wacky will create a set of words files that can be used with the C version of word2vec.

Wacky
-----

Wacky is a C++ program that can be built with CMake. It uses Boost's memory mapping to map the large ukWaC data files, resulting in a lower memory usage but reduced performance. It creates a set of files that are useful for the TensorFlow version of word to Vec, including

 - freq.txt - The frequency of all the unique words in the dataset
 - dictionary.txt - the most common words, one per line. Usually this is 50,000 words long
 - unk_count.txt - the count of words not in the dictionary
 - total_count.txt - the total number of words in ukWaC that we considered
 - A series of files called integer_<filename>_<thread_number>.txt - The words, converted to numbers for dictionary lookup - one per thread.

By default, wacky ignores commas and any tokens that have non-printable characters, outside the ascii training set. This is largely so that Tensorflow doesn't complain about incorrect data when it runs.

Wacky uses openmp to parallelise reading from the large files in the memory map. Thus, on an 8 core machine you can expect to see 8 smaller integer files for each ukwac file read.

Building Wacky
--------------

Simply create a directory and run cmake as you normally would. You should only need the Boost libraries.

Running Wacky
-------------

To run wacky

    ./wacky /my/path/to/ukwac/files

Wacky will pick up the files and parse them.


Tensorflow
----------

The tensorflow setup is a little more complicated. Assuming you have tensorflow, matplotlib, scikit learn and the rest installed, run

    python3 ukwac.py /path_to_data

Inside this file there are the paths to the files output by Wacky. You'll need to set these to where the Wacky output is, relative to where you are running ukwac.py from.

This program is set to run off the cpu but on my Tensorflow setup, it runs from the GPU. 

Singularity
-----------

Included is a .def file for creating your singularity container for the tensorflow part of the setup. It will create an ubuntu image, install tensorflow and run ukwac.py.

You need to have cuda version 367.57 installed and running on the host. This container def is quite specific and will need tweeking to fit your setup. It's a work in progress but it does work for most purposes.

To build the image first run:

    sudo singularity create --size 8000 wackyvec.img

Then perform the bootstrapping

    sudo singularity bootstrap wackyvec.img wackyvec.def

Finally, run the container with the provided data directory mapped to /data

    singularity run -B /home/oni/Projects/WackyVec/build:/data wackyvec.img
