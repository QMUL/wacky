// So the default way of including the main func for boost test doesnt work when using boost shared object libraries
// so we do it this way instead. Kept the original way in case we change back.
//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE wacky verb test suite
#include <boost/test/unit_test.hpp>


#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <dirent.h>
#include <omp.h>
#include <deque>


#include "string_utils.hpp"

#include "wacky_sbj_obj.hpp"
#include "wacky_create.hpp"
#include "wacky_read.hpp"
#include "wacky_verb.hpp"

using namespace std;

// Now lets test to make sure we can read everything back in correctly and also that the numbers we
// read in are correct
// TODO - this test is too long and also, there are not any representitive sentences to make for a proper
// test so we need to rethink our test data

BOOST_AUTO_TEST_CASE(verb_create_test) {

  map<string, size_t> FREQ {};
  vector< pair<string,size_t> > FREQ_FLIPPED {};
  set<string> WORD_IGNORES {",","-",".","@card@", "<text","<s>xt","</s>SENT", "<s>>SENT", "<s>", "</s>", "<text>", "</text>"};
  map<string,int> DICTIONARY_FAST {};
  vector<string> DICTIONARY {};
  set<string> ALLOWED_BASIS_WORDS;
  set<string> INSIST_BASIS_WORDS;
  vector<int> BASIS_VECTOR;

  size_t TOTAL_COUNT;
  size_t UNK_COUNT;
  size_t VOCAB_SIZE;

  set<string> VERB_TRANSITIVE;
  set<string> VERB_INTRANSITIVE;
  set<int> WORDS_TO_CHECK;
  vector<VerbPair> VERBS_TO_CHECK;
  vector< vector<int> > VERB_SUBJECTS;
  vector< vector<int> > VERB_OBJECTS;
  vector< vector<int> > VERB_SBJ_OBJ;
  vector< vector<float> > WORD_VECTORS;

  vector<string> SIMVERBS;
  vector<int> SIMVERBS_COUNT;
  vector<int> SIMVERBS_OBJECTS;
  vector<int> SIMVERBS_ALONE;

  // read the ukwac files
  vector<string> filenames;

  DIR *dir;
  struct dirent *ent;
  dir = opendir ("./ukwac");

  while ((ent = readdir (dir)) != NULL) {
    // Files to ignore
    if (strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0){
      continue;
    }
    string fullpath;
    fullpath = "./ukwac/" + string(ent->d_name);
    filenames.push_back(fullpath);
  }


  int r0 = read_total_file("./output", TOTAL_COUNT);
  int r1 = read_unk_file("./output", UNK_COUNT);
  int r2 = read_freq("./output", FREQ, FREQ_FLIPPED, ALLOWED_BASIS_WORDS); 
  int r3 = read_dictionary("./output", DICTIONARY_FAST, DICTIONARY, VOCAB_SIZE);

  BOOST_CHECK_EQUAL(VOCAB_SIZE, 820);

  create_basis("./output", FREQ, FREQ_FLIPPED, DICTIONARY_FAST, BASIS_VECTOR, ALLOWED_BASIS_WORDS, INSIST_BASIS_WORDS, 250, 100);

  int r4 = read_sim_file("simverb.txt", VERBS_TO_CHECK);
  BOOST_CHECK_EQUAL(r4, 0);

  BOOST_CHECK_EQUAL(BASIS_VECTOR.size(), 250);
 
  // Create then read the sim verbs
  int r5 = create_simverbs(filenames, "simverbs.txt", "./output", SIMVERBS, SIMVERBS_COUNT, SIMVERBS_OBJECTS, SIMVERBS_ALONE, true );

  int r6 = read_sim_stats("./output", VERB_TRANSITIVE, VERB_INTRANSITIVE);
  BOOST_CHECK_EQUAL(r6, 0);

  // Create the word Vectors
  int r7 = create_word_vectors(filenames, "./output", FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, BASIS_VECTOR, WORD_IGNORES, WORD_VECTORS, ALLOWED_BASIS_WORDS, VOCAB_SIZE, 250, 5, true);
  BOOST_CHECK_EQUAL(r7, 0);

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

  // Now create the verb subjects
  int r8 = create_verb_subject_object(filenames, "./output", DICTIONARY_FAST, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, false, false, true );
  BOOST_CHECK_EQUAL(r8, 0);
  
  // Intrans 
  int r9 = read_subject_file("./output", VERB_SUBJECTS);
  BOOST_CHECK_EQUAL(r9, 0);

  generate_words_to_check(WORDS_TO_CHECK, VERB_SBJ_OBJ, VERB_SUBJECTS, VERB_OBJECTS, VERBS_TO_CHECK,DICTIONARY_FAST);
	read_count("./output", FREQ, DICTIONARY, BASIS_VECTOR, WORD_VECTORS, TOTAL_COUNT, WORDS_TO_CHECK);
  intrans_count( VERBS_TO_CHECK, VERB_TRANSITIVE, VERB_INTRANSITIVE, 250, DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS);

  // Trans
  
}
