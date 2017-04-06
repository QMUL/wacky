/**
* @brief Main entry point for our wacky program
* @file wacky.cc
* @author Benjamin Blundell <b.blundell@qmul.ac.uk>
* @date 01/02/2017
*
*/

// http://www.boost.org/doc/libs/1_58_0/doc/html/interprocess/sharedmemorybetweenprocesses.html#interprocess.sharedmemorybetweenprocesses.mapped_file

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <dirent.h>
#include <omp.h>
#include <deque>
#include <getopt.h>

#include <boost/filesystem.hpp>

#include "string_utils.hpp"

// Use or nor use the Intel Maths Library
#ifdef _USE_MKL
#include "wacky_sbj_obj_mkl.hpp"
#else
#include "wacky_sbj_obj.hpp"
#endif

#include "wacky_create.hpp"
#include "wacky_read.hpp"
#include "wacky_verb.hpp"

#ifdef _USE_CUDA
#include <cuda_runtime.h>
#include <vector_types.h>
#include "cuda_verb.hpp"
#endif


using namespace std;
using namespace boost::filesystem;

// Our naughty naughty globals! Tsk, tsk!
// Most of these are global options or the big memory holders for all the vocab etc
// I'm holding them on the heap rather than passing references around at least for now

map<string, size_t> FREQ {};
vector< pair<string,size_t> > FREQ_FLIPPED {};
set<string> WORD_IGNORES {",","-",".","@card@", "<text","<s>xt","</s>SENT", "<s>>SENT", "<s>", "</s>", "<text>", "</text>"};
map<string,int> DICTIONARY_FAST {};
vector<string> DICTIONARY {};
set<string> ALLOWED_BASIS_WORDS;
set<string> INSIST_BASIS_WORDS;
vector< vector<int> > VERB_SUBJECTS;
vector< vector<int> > VERB_OBJECTS;
vector< vector<float> > WORD_VECTORS;
vector< vector<int> > VERB_SBJ_OBJ;
vector<int> BASIS_VECTOR;
vector<string> SIMVERBS;
vector<int> SIMVERBS_COUNT;
vector<int> SIMVERBS_OBJECTS;
vector<int> SIMVERBS_ALONE;

set<string> VERB_TRANSITIVE;
set<string> VERB_INTRANSITIVE;
set<int> WORDS_TO_CHECK;
vector<VerbPair> VERBS_TO_CHECK;

// Our list of options
struct WackyOptions {

  string ukdir;
  string simverb_file;
  string combine_file;
  string RESULTS_FILE;

  bool read_in;
  bool verb_subject;
  bool integers;
  bool word_vectors;
  bool sim_verbs;
  bool combine;
  bool count;
  bool intransitive;
	bool transitive;

  size_t UNK_COUNT;
  size_t TOTAL_COUNT;     // TODO - not really an option so needs moving I think
  size_t VOCAB_SIZE;      // Really more of a max
  size_t BASIS_SIZE;      // When creating the WORD_VECTORS how large are these vectors?
  string WORKING_DIR;     // Our current working directory
  bool  LEMMA_TIME;       // Use the lemma or stem of the word
  size_t IGNORE_WINDOW;   // How many most frequent words do we ignore completely in the basis?
  size_t WINDOW_SIZE;		  // Our sliding window size, either side of the chosen word
  bool  UNIQUE_SUBJECTS;
  bool  UNIQUE_OBJECTS;

};

/**
 * Combine all the ukwac files into one file
 * @param filenames the global vector of files
 * @param outpath the path we are saving the file to
 * @return a 1 or 0 for failure or success
 */

int combine_ukwac(vector<string> filenames, string outpath) {

  std::ofstream combine_file (outpath);
  
  // Scan directory for the files
  for( string filepath : filenames) {
    std::ifstream infile (filepath); 
    string line;
    
    while (std::getline(infile, line)){
      vector<string> tokens = s9::SplitStringWhitespace(line);
      if (tokens.size() > 5 ){
        combine_file << tokens[0] << " ";
      }
    }
    infile.close();
  }
  combine_file.close();
  return 0;

}

/**
 * Parse the command line options (of which there are many)
 * @param argc an int from main
 * @param argv an array of char from main
 * @param options a reference to a WackyOptions struct which we will set here
 */

void ParseCommandLine(int argc, char* argv[], WackyOptions &options) {
  int c;
  int digit_optind = 0;

  while ((c = getopt(argc, (char **)argv, "u:o:v:ls:rc:e:j:f:biwnyzpad?")) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'u' :
        options.ukdir = std::string(optarg);
        break;
      case 'y' :
        options.UNIQUE_SUBJECTS = true;
        break;
      case 'z' :
        options.UNIQUE_OBJECTS = true;
        break;
      case 'o' :
        options.WORKING_DIR = string(optarg);
        break;
      case 'f' :
        options.RESULTS_FILE = string(optarg);
        break;
      case 'v' :
        options.VOCAB_SIZE = s9::FromString<int>(optarg);
        break;
      case 'b':
        options.verb_subject = true;
        break;
      case 'a':
        options.intransitive = true;
        break;
			case 'd':
        options.transitive = true;
        break;
      case 'i':
        options.integers = true;
        break;
      case 'l' :
        options.LEMMA_TIME = true;
        cout << "Using Lemma forms of words" << endl;
        break;
      case 'j':
        options.WINDOW_SIZE = s9::FromString<int>(optarg);
        break;
      case 'r':
        options.read_in = true;
        break;
      case 's':
        options.simverb_file = string(optarg);
        break;
      case 'w':
        options.word_vectors = true;
        break;
      case 'e':
        options.BASIS_SIZE = s9::FromString<int>(optarg);
        break;
      case 'g':
        options.IGNORE_WINDOW = s9::FromString<int>(optarg);
        break;
      case 'n' :
        options.sim_verbs = true;
        break;
      case '?':
        std::cout << "wackyvec -u <path to ukwac> -o <output directory>" << std::endl;
        break;
      case 'c':
        options.combine_file = string(optarg);
        options.combine = true;
        break;
      case 'p':
        options.count = true;
        break;
      default:
        std::cout << "?? getopt returned character code" << c << std::endl;
    }
  }
}


// Main entrypoint
int main(int argc, char* argv[]) {

  // Initial settings
  WackyOptions options;

  options.read_in = false;
  options.verb_subject = false;
  options.integers = false;
  options.word_vectors = false;
  options.sim_verbs = false;
  options.combine = false;
  options.count = false;
  options.intransitive = false;
	options.transitive = false;
  options.ukdir = ".";

  options.UNK_COUNT = 0;
  options.TOTAL_COUNT = 0;
  options.VOCAB_SIZE = 500000;
  options.BASIS_SIZE = 5000;
  options.WORKING_DIR = ".";
  options.LEMMA_TIME = true;
  options.IGNORE_WINDOW = 100;
  options.WINDOW_SIZE = 5;
  options.UNIQUE_SUBJECTS = false;
  options.UNIQUE_OBJECTS = false;

  options.RESULTS_FILE = "results.txt";

  ParseCommandLine(argc, argv, options);

  vector<string> filenames;
  
  // Scan directory for the ukwac files
  if (is_directory(options.ukdir)) {
    
    DIR *dir;
    struct dirent *ent;
    dir = opendir (options.ukdir.c_str());

    while ((ent = readdir (dir)) != NULL) {
      // Files to ignore
      if (strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0){
        continue;
      }
      string fullpath;
      fullpath = options.ukdir + "/" + string(ent->d_name);
      filenames.push_back(fullpath);
    }

  } else { // Incorrect directory given
    cout << "Incorrect command line argument for directory" << endl;
    return 1;
  }
 

  // Are we reading in the existing dictionary, frequency and such
  if (options.read_in){
    cout << "Reading in dictionary and frequency data" << endl;
    read_freq(options.WORKING_DIR, FREQ, FREQ_FLIPPED, ALLOWED_BASIS_WORDS);
    read_dictionary(options.WORKING_DIR, DICTIONARY_FAST, DICTIONARY, options.VOCAB_SIZE);
    read_insist_words(options.WORKING_DIR, INSIST_BASIS_WORDS);
    create_basis(options.WORKING_DIR, FREQ, FREQ_FLIPPED, DICTIONARY_FAST, BASIS_VECTOR, ALLOWED_BASIS_WORDS, INSIST_BASIS_WORDS, options.BASIS_SIZE, options.IGNORE_WINDOW);

  } else {
    cout << "Creating frequency and dictionary" << endl;
    if (create_freq(filenames, options.WORKING_DIR,FREQ, FREQ_FLIPPED, WORD_IGNORES, ALLOWED_BASIS_WORDS, options.LEMMA_TIME) != 0)  { return 1; }    
    if (create_dictionary(options.WORKING_DIR, FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, options.VOCAB_SIZE) != 0) { return 1; }
  }
	
  cout << "Vocab Size: " << options.VOCAB_SIZE << endl; 
 
  // Initialise our verb to subject/object maps
  for (int i = 0; i <= options.VOCAB_SIZE; ++i) {
    VERB_SUBJECTS.push_back( vector<int>() );
  }
  
  for (int i = 0; i <= options.VOCAB_SIZE; ++i) {
    VERB_OBJECTS.push_back( vector<int>() );
  }

  for (int i = 0; i <= options.VOCAB_SIZE; ++i) {
    VERB_SBJ_OBJ.push_back( vector<int>() );
  }


  // Are we creating the verb subject/object vectors?
  if (options.count) {
    if (options.read_in) { 
      cout << "Performing statistics on count vectors" << endl;
      cout << "Reading in dictionary and frequency data" << endl;
      if (read_total_file(options.WORKING_DIR, options.TOTAL_COUNT) != 0 ) { cout << "read total file failed" << endl; return 1; }
      if (read_unk_file(options.WORKING_DIR, options.UNK_COUNT)  != 0 ) { cout << "read unk file failed" << endl; return 1; }
      if (read_sim_file(options.simverb_file, VERBS_TO_CHECK) != 0 ) { cout << "read sim file failed" << endl; return 1; }
      if (read_sim_stats(options.WORKING_DIR, VERB_TRANSITIVE, VERB_INTRANSITIVE) != 0 ) { cout << "read sim_stats file failed" << endl; return 1; }

    	if (options.intransitive){
        if (read_subject_file(options.WORKING_DIR, VERB_SUBJECTS) != 0 ) { cout << "read subject file failed" << endl; return 1; }
        generate_words_to_check(WORDS_TO_CHECK, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, VERBS_TO_CHECK,DICTIONARY_FAST );
				if (read_count(options.WORKING_DIR, FREQ, DICTIONARY, BASIS_VECTOR, WORD_VECTORS, options.TOTAL_COUNT, WORDS_TO_CHECK) != 0 ) { cout << "read count file failed" << endl; return 1; }
        intrans_count( options.RESULTS_FILE, VERBS_TO_CHECK, VERB_TRANSITIVE, VERB_INTRANSITIVE, options.BASIS_SIZE, DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS);

      } else if (options.transitive) {
        if (read_subject_file(options.WORKING_DIR, VERB_SUBJECTS) != 0 ) { cout << "read subject file failed" << endl; return 1; }
        if ( read_subject_object_file(options.WORKING_DIR, VERB_SBJ_OBJ) != 0 ) { cout << "read total file failed" << endl; return 1; }

				generate_words_to_check(WORDS_TO_CHECK, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, VERBS_TO_CHECK,DICTIONARY_FAST );

				if (read_count(options.WORKING_DIR, FREQ, DICTIONARY, BASIS_VECTOR, WORD_VECTORS, options.TOTAL_COUNT, WORDS_TO_CHECK) != 0 ) { cout << "read count file failed" << endl; return 1; }
        trans_count( options.RESULTS_FILE, VERBS_TO_CHECK, VERB_TRANSITIVE, VERB_INTRANSITIVE, options.BASIS_SIZE, DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS);
      } else {
     		if(read_subject_object_file(options.WORKING_DIR, VERB_SBJ_OBJ) != 0 ) { cout << "read subject/object file failed" << endl; return 1; }

				generate_words_to_check(WORDS_TO_CHECK, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, VERBS_TO_CHECK,DICTIONARY_FAST );
				if (read_count(options.WORKING_DIR, FREQ, DICTIONARY, BASIS_VECTOR, WORD_VECTORS, options.TOTAL_COUNT, WORDS_TO_CHECK)  != 0 ) { cout << "read count file failed" << endl; return 1; }

#ifdef _USE_CUDA
        all_count_cuda(options.RESULTS_FILE, VERBS_TO_CHECK, VERB_TRANSITIVE, VERB_INTRANSITIVE, options.BASIS_SIZE, DICTIONARY_FAST, VERB_SBJ_OBJ, VERB_SUBJECTS, WORD_VECTORS);
#else
        all_count(options.RESULTS_FILE, VERBS_TO_CHECK, VERB_TRANSITIVE, VERB_INTRANSITIVE, options.BASIS_SIZE, DICTIONARY_FAST, VERB_SBJ_OBJ, VERB_SUBJECTS, WORD_VECTORS);
#endif
      }

    } else {
      cout << "You must pass -r and -s along with -p" << endl;
      return 1;
    }
  }

  // Are we combining files?
  if (options.combine) {
    cout << "Combining ukwac into a large file of text" << endl;
    combine_ukwac(filenames, options.combine_file);
    return 0;
  }
 
  // Are we creating our verb subject and object files
  if (options.verb_subject) {
    cout << "Create verb subjects and objects" << endl; 
    if (create_verb_subject_object(filenames, options.WORKING_DIR, DICTIONARY_FAST, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, options.UNIQUE_OBJECTS, options.UNIQUE_SUBJECTS, options.LEMMA_TIME ) != 0)  { return 1; }
  }
  
  // Are we converting words to numbers for tensorflow?
  if (options.integers) {
    cout << "Create integer files" << endl;
    if (create_integers(filenames, options.WORKING_DIR, DICTIONARY_FAST, options.VOCAB_SIZE, options.LEMMA_TIME) != 0) { return 1; }
  }

  // Are we creating our word vectors?
  if (options.word_vectors){
    cout << "Creating word vectors" << endl;
    if (create_word_vectors(filenames, options.WORKING_DIR, FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, BASIS_VECTOR, WORD_IGNORES, WORD_VECTORS, ALLOWED_BASIS_WORDS, options.VOCAB_SIZE, options.BASIS_SIZE, options.WINDOW_SIZE, options.LEMMA_TIME) != 0)  { return 1; }
  }
  
  // Are we creating the sim verbs file?
  if (options.sim_verbs) {
    cout << "Creating simverbs" << endl;
    if (create_simverbs(filenames, options.simverb_file, options.WORKING_DIR, SIMVERBS, SIMVERBS_COUNT, SIMVERBS_OBJECTS, SIMVERBS_ALONE, options.LEMMA_TIME ) !=0)  { return 1; }
  }

  return 0;
}
