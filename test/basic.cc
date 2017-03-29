// So the default way of including the main func for boost test doesnt work when using boost shared object libraries
// so we do it this way instead. Kept the original way in case we change back.
//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE wacky basic test suite
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


// Create the basic files from the ukwac test case and check the numbers

BOOST_AUTO_TEST_CASE(dictionary_freq_create_test) {

  map<string, size_t> FREQ {};
  vector< pair<string,size_t> > FREQ_FLIPPED {};
  set<string> WORD_IGNORES {",","-",".","@card@", "<text","<s>xt","</s>SENT", "<s>>SENT", "<s>", "</s>", "<text>", "</text>"};
  map<string,int> DICTIONARY_FAST {};
  vector<string> DICTIONARY {};
  set<string> ALLOWED_BASIS_WORDS;
  size_t VOCAB_SIZE = 5000;
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

  // First, test that we can create correct dictionaries and frequencies 
  int r0 = create_freq(filenames, "./output", FREQ, FREQ_FLIPPED, WORD_IGNORES, ALLOWED_BASIS_WORDS, true); 
  int r1 = create_dictionary("./output", FREQ, FREQ_FLIPPED, DICTIONARY_FAST, DICTIONARY, VOCAB_SIZE);
  int r2 = create_integers(filenames, "./output", DICTIONARY_FAST, VOCAB_SIZE, true);
  
  BOOST_CHECK_EQUAL(r0, 0);
  BOOST_CHECK_EQUAL(r1, 0);
  BOOST_CHECK_EQUAL(r2, 0);

  // Now read in the files and check the scores

  std::ifstream freq_file ("./output/freq.txt");
  string line;
  size_t idx = 0;
  
  if (freq_file.is_open()) {
    while ( getline (freq_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      vector<string> tokens = s9::SplitStringString(line, ", ");
      if (tokens.size() > 1) {
        size_t freq = s9::FromString<size_t>(tokens[1]); 
        if (tokens[0].compare("also") == 0){
          BOOST_CHECK_EQUAL(freq, 4);
        }
        else if (tokens[0].compare("box") == 0){
          BOOST_CHECK_EQUAL(freq, 3);
        }
        else if (tokens[0].compare("aloe") == 0){
          BOOST_CHECK_EQUAL(freq, 7);
        }
      } 
    }
  }    
}


// Now lets test to make sure we can read everything back in correctly and also that the numbers we
// read in are correct
BOOST_AUTO_TEST_CASE(dictionary_freq_read_test) {

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

  int r0 = read_total_file("./output", TOTAL_COUNT);
  BOOST_CHECK_EQUAL(r0, 0);
  BOOST_CHECK_EQUAL(TOTAL_COUNT, 1993);
  
  int r1 = read_unk_file("./output", UNK_COUNT);
  BOOST_CHECK_EQUAL(r1, 0);
  BOOST_CHECK_EQUAL(UNK_COUNT, 224);

  int r2 = read_freq("./output", FREQ, FREQ_FLIPPED, ALLOWED_BASIS_WORDS);
  BOOST_CHECK_EQUAL(r2, 0);
  
  int r3 = read_dictionary("./output", DICTIONARY_FAST, DICTIONARY, VOCAB_SIZE);
  BOOST_CHECK_EQUAL(r3, 0);
  
  create_basis("./output", FREQ, FREQ_FLIPPED, DICTIONARY_FAST, BASIS_VECTOR, ALLOWED_BASIS_WORDS, INSIST_BASIS_WORDS, 250, 100);

  BOOST_CHECK_EQUAL(BASIS_VECTOR.size(), 250);
  
}
