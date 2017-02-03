/**
 * A small program that pre-processes the ukWaC dataset for reading in python
 * 
 * It creates 5 sets of files from ukWaC
 * 1) A Vocab / Frequency file that lists the FREQuency of all tokens - word, FREQ\n - FREQ.txt
 * 2) A DICTIONARY of VOCAB_SIZE words in alphabetical order - word\n - DICTIONARY.txt
 * 3) A set of files that form the sentences, one word per line
 * 4) A set of files that form the sentences with integer lookups into the DICTIONARY 
 * 5) A relationship file that lists the subjects of every verb
 * We remove any characters outside the ascii range. I believe ukWaC uses an annoying windows codec?
 *
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
#include "wacky_sbj_obj.hpp"
#include "wacky_create.hpp"
#include "wacky_read.hpp"
#include "wacky_verb.hpp"

using namespace std;
using namespace boost::filesystem;

// Our naughty naughty globals! Tsk, tsk!
// Most of these are global options or the big memory holders for all the vocab etc
map<string, size_t> FREQ {};
vector< pair<string,size_t> > FREQ_FLIPPED {};
set<string> WORD_IGNORES {",","-",".","@card@", "<text","<s>xt","</s>SENT", "<s>>SENT", "<s>", "</s>", "<text>", "</text>"};
map<string,int> DICTIONARY_FAST {};
vector<string> DICTIONARY {};
set<string> ALLOWED_BASIS_WORDS;
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
vector<VerbPair> VERBS_TO_CHECK;

size_t UNK_COUNT = 0;
size_t TOTAL_COUNT = 0;
size_t VOCAB_SIZE = 500000;  // Really more of a max
size_t BASIS_SIZE = 5000;   // When creating the WORD_VECTORS how large are these vectors?
string OUTPUT_DIR = ".";
bool  LEMMA_TIME = true;   // Use the lemma or stem of the word
size_t IGNORE_WINDOW = 100; // How many most frequent words do we ignore completely in the basis?
size_t WINDOW_SIZE = 3;			// Our sliding window size, either side of the chosen word
bool  UNIQUE_SUBJECTS = false;
bool  UNIQUE_OBJECTS = false;


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

}


// Main entrypoint
int main(int argc, char* argv[]) {

  int c;
  int digit_optind = 0;

  string ukdir;
  string simverb_file;
  string combine_file;
  vector<string> filenames;
  bool read_in = false;
  bool verb_subject = false;
  bool integers = false;
  bool word_vectors = false;
  bool sim_verbs = false;
  bool combine = false;
  bool count = false;

  while ((c = getopt(argc, (char **)argv, "u:o:v:ls:rc:j:biwnyzp?")) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'u' :
        ukdir = std::string(optarg);
        break;
      case 'y' :
        UNIQUE_SUBJECTS = true;
        break;
      case 'z' :
        UNIQUE_OBJECTS = true;
        break;
      case 'o' :
        OUTPUT_DIR = string(optarg);
        break;
      case 'v' :
        VOCAB_SIZE = s9::FromString<int>(optarg);
        break;
      case 'b':
        verb_subject = true;
        break;
      case 'i':
        integers = true;
        break;
      case 'l' :
        LEMMA_TIME = true;
        cout << "Using Lemma forms of words" << endl;
        break;
      case 'j':
        WINDOW_SIZE = s9::FromString<int>(optarg);
        break;
      case 'r':
        read_in = true;
        break;
      case 's':
        simverb_file = string(optarg);
        break;
      case 'w':
        word_vectors = true;
        break;
      case 'n' :
        sim_verbs = true;
        break;
      case '?':
        std::cout << "wackyvec -u <path to ukwac> -o <output directory>" << std::endl;
        break;
      case 'c':
        combine_file = string(optarg);
        combine = true;
        break;
      case 'p':
        count = true;
        break;
      default:
        std::cout << "?? getopt returned character code" << c << std::endl;
    }
  }

  cout << "Vocab Max Size: " << VOCAB_SIZE << endl; 
  
  if (read_in){
    cout << "Reading in dictionary and frequency data" << endl;
    read_freq(OUTPUT_DIR, FREQ, FREQ_FLIPPED, ALLOWED_BASIS_WORDS);
    read_dictionary(OUTPUT_DIR, DICTIONARY_FAST, DICTIONARY,VOCAB_SIZE);
    create_basis(OUTPUT_DIR, FREQ, FREQ_FLIPPED, DICTIONARY_FAST, BASIS_VECTOR, ALLOWED_BASIS_WORDS, BASIS_SIZE, IGNORE_WINDOW);

  } else {
    cout << "Creating frequency and dictionary" << endl;
    if (create_freq(filenames, OUTPUT_DIR,FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, WORD_IGNORES, ALLOWED_BASIS_WORDS,LEMMA_TIME) != 0)  { return 1; }    
    if (create_dictionary(OUTPUT_DIR, FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, VOCAB_SIZE) != 0) { return 1; }
  }
 
  // Initialise our verb to subject/object maps
  for (int i = 0; i <= VOCAB_SIZE; ++i) {
    VERB_SUBJECTS.push_back( vector<int>() );
  }
  
  for (int i = 0; i <= VOCAB_SIZE; ++i) {
    VERB_OBJECTS.push_back( vector<int>() );
  }

  for (int i = 0; i <= VOCAB_SIZE; ++i) {
    VERB_SBJ_OBJ.push_back( vector<int>() );
  }


  if (count) {
    if (read_in) { 
      cout << "Performing statistics on count vectors" << endl;
      cout << "Reading in dictionary and frequency data" << endl;
      read_total_file(OUTPUT_DIR, TOTAL_COUNT);
      read_unk_file(OUTPUT_DIR, UNK_COUNT);
      read_sim_file(OUTPUT_DIR, VERBS_TO_CHECK);
      read_sim_stats(OUTPUT_DIR, VERB_TRANSITIVE, VERB_INTRANSITIVE);
      read_subject_file(OUTPUT_DIR, VERB_SUBJECTS); 
      read_count(OUTPUT_DIR, FREQ, DICTIONARY, BASIS_VECTOR, WORD_VECTORS, TOTAL_COUNT);
      intrans_count( VERBS_TO_CHECK, VERB_TRANSITIVE, VERB_INTRANSITIVE, BASIS_SIZE, DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS);
    } else {
      cout << "You must pass -r and -s along with -p" << endl;
    }
    return 0;
  }

  // Scan directory for the files
  if (is_directory(ukdir)) {
    
    DIR *dir;
    struct dirent *ent;
    dir = opendir (ukdir.c_str());

    while ((ent = readdir (dir)) != NULL) {
      // Files to ignore
      if (strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0){
        continue;
      }
      string fullpath;
      fullpath = ukdir + "/" + string(ent->d_name);
      filenames.push_back(fullpath);
    }

  } else { // Incorrect directory given
    cout << "Incorrect command line argument for directory" << endl;
    return 1;
  }
 
  if (combine) {
    cout << "Combining ukwac into a large file of text" << endl;
    combine_ukwac(filenames, combine_file);
    return 0;
  }
 
  if (verb_subject) {
    cout << "Create verb subjects and objects" << endl; 
    if (create_verb_subject_object(filenames, OUTPUT_DIR, DICTIONARY_FAST, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, UNIQUE_OBJECTS, UNIQUE_SUBJECTS, LEMMA_TIME ) != 0)  { return 1; }
  }
  
  if (integers) {
    cout << "Create integer files" << endl;
    if (create_integers(filenames, OUTPUT_DIR, DICTIONARY_FAST, VOCAB_SIZE, LEMMA_TIME) != 0) { return 1; }
  }
  
  if (word_vectors){
    cout << "Creating word vectors" << endl;
    if (create_word_vectors(filenames, OUTPUT_DIR, FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, BASIS_VECTOR, WORD_IGNORES, WORD_VECTORS, ALLOWED_BASIS_WORDS, VOCAB_SIZE, BASIS_SIZE, WINDOW_SIZE, LEMMA_TIME) != 0)  { return 1; }
  }
  
  if (sim_verbs) {
    cout << "Creating simverbs" << endl;
    if (create_simverbs(filenames,simverb_file, OUTPUT_DIR, SIMVERBS, SIMVERBS_COUNT, SIMVERBS_OBJECTS, SIMVERBS_ALONE, LEMMA_TIME ) !=0)  { return 1; }
  }

  return 0;
}
