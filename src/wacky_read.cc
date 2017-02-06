/**
* @brief Functions for reading in existing various wacky files
* @file wacky_read.cc
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#include "wacky_read.hpp"

using namespace std;

/**
 * Read in our dictionary from a file
 * @param OUTPUT_DIR the output directory (in this case, where are we reading from?)
 * @param DICTIONARY_FAST the fast dictionary
 * @param DICTIONARY the original dictionary
 * @param VOCAB_SIZE how big is the dictionary
 * @return int a value to say if we succeeded or not
 */

int read_dictionary(string OUTPUT_DIR, map<string,int> & DICTIONARY_FAST, vector<string> & DICTIONARY, size_t & VOCAB_SIZE){

  std::ifstream dictionary_file (OUTPUT_DIR + "/dictionary.txt");
  string line;
  size_t idx = 0;
  while ( getline (dictionary_file,line) ) {
    string word = s9::RemoveChar(line,'\n'); 
    if (idx != 0){
      DICTIONARY.push_back(word);
      DICTIONARY_FAST[word] = idx-1;
    } 
    idx+=1;
  }
	VOCAB_SIZE = DICTIONARY.size();
}



/**
 * Read the count of unknown words
 * @param OUTPUT_DIR the output directory
 * @param UNK_COUNT the number we are filling
 * @return int a value to say if we succeeded or not
 */


int read_unk_file(string OUTPUT_DIR, size_t & UNK_COUNT) {

  std::ifstream unk_file (OUTPUT_DIR + "/unk_count.txt");
  string line;

  while ( getline (unk_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    int uk = s9::FromString<int>(line);
    unk_file.close();
    UNK_COUNT = uk;
    return uk;
  }

  return 0;
}


/**
 * Read in the file that contains verb subjects
 * @param OUTPUT_DIR the output directory
 * @param VERB_SUBJECTS a vector of vector of ints we shall file
 */

void read_subject_file(string OUTPUT_DIR, vector< vector<int> > & VERB_SUBJECTS) {
  std::ifstream total_file (OUTPUT_DIR + "/verb_subjects.txt");
  string line;

  cout << "Reading Subject File" << endl;

  while ( getline (total_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    vector<string> tokens =  s9::SplitStringWhitespace(line);

    if (tokens.size() > 1) {
      for (string sbj : tokens) {
        int sbj_idx = s9::FromString<int>(sbj);
        VERB_SUBJECTS[s9::FromString<int>(tokens[0])].push_back(sbj_idx);
      }
    }
  }
}

/**
 * Read the subject object pairing file
 * @param OUTPUT_DIR the output directory
 * @param VERB_SBJ_OBJ a vector of vector of ints we shall file
 */

void read_subject_object_file(string OUTPUT_DIR, vector< vector<int> > & VERB_SBJ_OBJ) {
  std::ifstream total_file (OUTPUT_DIR + "/verb_sbj_obj.txt");
  string line;

  cout << "Reading Subject Object File" << endl;

  while ( getline (total_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    vector<string> tokens =  s9::SplitStringWhitespace(line);

    if (tokens.size() > 1) {
      for (string sobj : tokens) {
        int idx = s9::FromString<int>(sobj);
        VERB_SBJ_OBJ[s9::FromString<int>(tokens[0])].push_back(idx);
      }
    }
  }
}


/**
 * Read how many words in total file
 * @param OUTPUT_DIR the output directory
 * @param TOTAL_COUNT the number we are reading into
 * @return int to show if we succeeded
 */

int read_total_file(string OUTPUT_DIR, size_t & TOTAL_COUNT ) {

  std::ifstream total_file (OUTPUT_DIR + "/total_count.txt");
  string line;

  while ( getline (total_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    int uk = s9::FromString<int>(line);
    total_file.close();
    TOTAL_COUNT = uk;
    return uk;
  }

  return 0;
}


/**
 * Read in the file that contains the statistics of our verb subjects / objects
 * @param OUTPUT_DIR the output directory
 * @param VERB_TRANSITIVE a set of transitive verbs we shall fill
 * @param VERB_INTRANSITIVE a set of intransitive verbs we shall fill
 */

void read_sim_stats(string OUTPUT_DIR, set<string> & VERB_TRANSITIVE, set<string> & VERB_INTRANSITIVE ) {

  std::ifstream total_file (OUTPUT_DIR + "/sim_stats.txt");
  string line;

  while ( getline (total_file,line) ) {
    line = s9::RemoveChar(line,'\n');

    vector<string> tokens = s9::SplitStringWhitespace(line);
    string verb = tokens[0];
    int obs = s9::FromString<int>(tokens[1]);
    int alone = s9::FromString<int>(tokens[2]);
    int total = s9::FromString<int>(tokens[3]);

    if (obs > alone){
      VERB_TRANSITIVE.insert(verb);
    } else {
      VERB_INTRANSITIVE.insert(verb);
    }    
  }
}

/**
 * Read in the verb pair comparisons file
 * @param OUTPUT_DIR the output directory
 * @param VERBS_TO_CHECK a vector of VerbPairs that we shall fill
 */

void read_sim_file(string OUTPUT_DIR, vector<VerbPair> & VERBS_TO_CHECK) {
   
  std::ifstream total_file (OUTPUT_DIR + "/SimVerb-500-dev.txt");
  string line;

  while ( getline (total_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    
    vector<string> tokens = s9::SplitStringWhitespace(line);
    
    VerbPair s;
    s.v0 = tokens[0];
    s.v1 = tokens[1];
    s.s = s9::FromString<float>(tokens[3]);

    VERBS_TO_CHECK.push_back(s);

  }

}


/**
 * Read in the frequency file for all the words
 * @param OUTPUT_DIR the output directory
 * @param FREQ the map of frequency we shall fill
 * @param FREQ_FLIPPED the flipped frequency we shall fill
 * @param ALLOWED_BASIS_WORDS a set of allowed basis words we shall also read in
 * @return an int to say if we succeeded.
 */

int read_freq(string OUTPUT_DIR, map<string, size_t> & FREQ, vector< pair<string,size_t> > & FREQ_FLIPPED, set<string> & ALLOWED_BASIS_WORDS) {
  std::ifstream freq_file (OUTPUT_DIR + "/freq.txt");
  string line;
  size_t idx = 0;
  while ( getline (freq_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    vector<string> tokens = s9::SplitStringString(line, ", ");
    if (idx != 0){
      FREQ[tokens[0]] = s9::FromString<size_t>(tokens[1]); 
    } 
    idx+=1;
  }

  for (auto it = FREQ.begin(); it != FREQ.end(); it++){
    FREQ_FLIPPED.push_back(*it);
  }

  std::sort(FREQ_FLIPPED.begin(), FREQ_FLIPPED.end(), sort_freq);
  freq_file.close();

  // Read the allowed file too
  std::ifstream allowed_file (OUTPUT_DIR + "/allowed.txt");
  while ( getline (allowed_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    ALLOWED_BASIS_WORDS.insert(line);
  }
  allowed_file.close();

  return 0;
}


/**
 * Read in the word vector counts for analysis. It converts the vectors to probabilities
 * @param OUTPUT_DIR the output directory
 * @param FREQ the map of frequency
 * @param DICTIONARY the dictionary
 * @param BASIS_VECTOR the vector of ints that represents the basis
 * @param WORD_VECTORS the vector of vectors we shall fill
 * @param TOTAL_COUNT the total count of all the words in ukwac
 */

void read_count(string OUTPUT_DIR, map<string, size_t> & FREQ, vector<string> & DICTIONARY, vector<int>  & BASIS_VECTOR, vector< vector<float> > & WORD_VECTORS, size_t TOTAL_COUNT) {
  cout << "Reading the word_vectors count" << endl;
  std::ifstream count_file (OUTPUT_DIR + "/word_vectors.txt");
  string line;
  size_t idx = 0;
  while ( getline (count_file,line) ) {
    line = s9::RemoveChar(line,'\n');
    vector<string> tokens = s9::SplitStringWhitespace(line);
   
    vector<float> tv; 

    for (int i =0; i < tokens.size(); ++i) {
      float ct = static_cast<float>(FREQ[DICTIONARY[BASIS_VECTOR[i]]]);
      float cc = static_cast<float>(FREQ[DICTIONARY[idx]]);
      float cct = s9::FromString<float>(tokens[i]);

      float pmi = 0;
      if (cct != 0.0 && ct != 0.0 && cc != 0.0){
        pmi = log( (cct/ ct) / (cc / static_cast<float>(TOTAL_COUNT)));
      }
      tv.push_back(pmi);

    }

    WORD_VECTORS.push_back(tv);
    idx++;
  }
}


