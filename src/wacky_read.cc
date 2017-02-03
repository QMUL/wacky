#include "wacky_read.hpp"

using namespace std;

// If we've already generated a dictionary read it in
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


// Read the unkown count if we havent already

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

// Read the subjects file
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


// Read the total count if we havent already
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

// read the sim stats file we have generated
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

// read the sim file
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

// If we've already generated a frequency read it in
// along with the allowed words
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

// Read vector count
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


