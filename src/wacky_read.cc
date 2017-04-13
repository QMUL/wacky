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
  
  if (dictionary_file.is_open()) {
    while ( getline (dictionary_file,line) ) {
      string word = s9::RemoveChar(line,'\n'); 
      DICTIONARY.push_back(word);
      DICTIONARY_FAST[word] = idx;
      idx+=1;
    }
	  VOCAB_SIZE = DICTIONARY.size();
    return 0;
  }
  return 1;
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

  if (unk_file.is_open()) {
    while ( getline (unk_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      int uk = s9::FromString<int>(line);
      unk_file.close();
      UNK_COUNT = uk;
      return 0;
    }
  }
  return 1;
}

/**
 * Read the optional list of words that have to be in basis
 * @param OUTPUT_DIR the output directory
 * @param UNK_COUNT the number we are filling
 * @return int a value to say if we succeeded or not
 */

int read_insist_words(std::string OUTPUT_DIR, std::set<std::string> & INSIST_BASIS_WORDS) {
  std::ifstream insist_file (OUTPUT_DIR + "/insist.txt");
  string line;

  if (insist_file.is_open()) {
    while ( getline (insist_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      INSIST_BASIS_WORDS.insert(line); 
    }    
    insist_file.close();
    return 0;
  }
  return 1;
}


/**
 * Read in the file that contains verb subjects
 * @param OUTPUT_DIR the output directory
 * @param VERB_SUBJECTS a vector of vector of ints we shall file
 * @return int value to say if we succeeded or not
 */

int read_subject_file(string OUTPUT_DIR, vector< vector<int> > & VERB_SUBJECTS) {
  std::ifstream total_file (OUTPUT_DIR + "/verb_subjects.txt");
  string line;

  cout << "Reading Subject File" << endl;

  if (total_file.is_open()) {
    while ( getline (total_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      vector<string> tokens =  s9::SplitStringWhitespace(line);

      if (tokens.size() > 1) {
        int idx = s9::FromString<int>(tokens[0]);
        for (int i= 1; i < tokens.size(); ++i) {
          int sbj_idx = s9::FromString<int>(tokens[i]);
          VERB_SUBJECTS[idx].push_back(sbj_idx);
        }
      }
    }
    return 0;
  }
  return 1;
}

/**
 * Read the subject object pairing file
 * @param OUTPUT_DIR the output directory
 * @param VERB_SBJ_OBJ a vector of vector of ints we shall file
 * @return int value to say if we succeeded or not
 */

int read_subject_object_file(string OUTPUT_DIR, vector< vector<int> > & VERB_SBJ_OBJ) {
  std::ifstream total_file (OUTPUT_DIR + "/verb_sbj_obj.txt");
  string line;

  cout << "Reading Subject Object File" << endl;

  if (total_file.is_open()) {
    while ( getline (total_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      vector<string> tokens =  s9::SplitStringWhitespace(line);

      if (tokens.size() > 1) {
        for (int i= 1; i < tokens.size(); ++i) {
          int idx = s9::FromString<int>(tokens[i]);
          VERB_SBJ_OBJ[s9::FromString<int>(tokens[0])].push_back(idx);
        }
      }
    }
    return 0;
  }
  return 1;
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

  if (total_file.is_open()) {
    while ( getline (total_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      int uk = s9::FromString<int>(line);
      total_file.close();
      TOTAL_COUNT = uk;
      return 0;
    }
  }
  return 1;
}


/**
 * Read in the file that contains the statistics of our verb subjects / objects
 * @param OUTPUT_DIR the output directory
 * @param VERB_TRANSITIVE a set of transitive verbs we shall fill
 * @param VERB_INTRANSITIVE a set of intransitive verbs we shall fill
 * @return int to show if we succeeded or not
 */

int read_sim_stats(string OUTPUT_DIR, set<string> & VERB_TRANSITIVE, set<string> & VERB_INTRANSITIVE ) {

  std::ifstream total_file (OUTPUT_DIR + "/sim_stats.txt");
  string line;

  if (total_file.is_open()) {
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
    return 0;
  }
  return 1;
}



int read_basis(string OUTPUT_DIR,
  vector<int> & BASIS_VECTOR,
  size_t & BASIS_SIZE) {

  std::ifstream total_file (OUTPUT_DIR + "/basis.txt");
  string line;

  if (total_file.is_open()) {
    while ( getline (total_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      int bidx = s9::FromString<int>(line);
      BASIS_VECTOR.push_back(bidx);
    }
  
    BASIS_SIZE = BASIS_VECTOR.size();
    total_file.close();
    return 0;
  }

  return 1;
}



/**
* Read in the verb pair comparisons file
* @param OUTPUT_DIR the output directory
* @param VERBS_TO_CHECK a vector of VerbPairs that we shall fill
* @return int to say if we succeeded or not
*/

int read_sim_file(string simverb_file, vector<VerbPair> & VERBS_TO_CHECK) {
   
  std::ifstream total_file (simverb_file);
  string line;

  if (total_file.is_open()) {
    while ( getline (total_file,line) ) {
      line = s9::RemoveChar(line,'\n');
      
      vector<string> tokens = s9::SplitStringWhitespace(line);
      
      VerbPair s;
      s.v0 = tokens[0];
      s.v1 = tokens[1];
      s.s = s9::FromString<float>(tokens[3]);

      VERBS_TO_CHECK.push_back(s);
    }

    return 0;
  }
  return 1;

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

  if (freq_file.is_open()) {
    while ( getline (freq_file,line) ) {
      line = s9::RemoveChar(line,'\n');

      vector<string> tokens = s9::SplitStringString(line, ", ");
      if (tokens.size() > 1) {
        FREQ[tokens[0]] = s9::FromString<size_t>(tokens[1]); 
        idx+=1;
      }
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

  return 1;
}

void generate_words_to_check(set<int> & WORDS_TO_CHECK, vector< vector<int> > & VERB_SBJ_OBJ, vector< vector<int> > & VERB_SUBJECTS, vector< vector<int> > & VERB_OBJECTS, vector<VerbPair> & VERBS_TO_CHECK, map<string,int> DICTIONARY_FAST ) {

	for (VerbPair vp : VERBS_TO_CHECK){
		int idx0 = DICTIONARY_FAST[vp.v0];
		int idx1 = DICTIONARY_FAST[vp.v1];

		WORDS_TO_CHECK.insert(idx0);
		WORDS_TO_CHECK.insert(idx1);

		for (int j =0; j < VERB_SBJ_OBJ[idx0].size(); ++j){
			WORDS_TO_CHECK.insert(VERB_SBJ_OBJ[idx0][j]);			
		}

		for (int j =0; j < VERB_SBJ_OBJ[idx1].size(); ++j){
			WORDS_TO_CHECK.insert(VERB_SBJ_OBJ[idx1][j]);			
		}
		
		for (int j =0; j < VERB_SUBJECTS[idx0].size(); ++j){
			WORDS_TO_CHECK.insert(VERB_SUBJECTS[idx0][j]);			
		}

		for (int j =0; j < VERB_SUBJECTS[idx1].size(); ++j){
			WORDS_TO_CHECK.insert(VERB_SUBJECTS[idx1][j]);			
		}
		
		for (int j =0; j < VERB_OBJECTS[idx0].size(); ++j){
			WORDS_TO_CHECK.insert(VERB_OBJECTS[idx0][j]);			
		}

		for (int j =0; j < VERB_OBJECTS[idx1].size(); ++j){
			WORDS_TO_CHECK.insert(VERB_OBJECTS[idx1][j]);			
		}	
	}

}


/**
 * Read in the word vector counts for analysis. It converts the vectors to probabilities
 * @param OUTPUT_DIR the output directory
 * @param FREQ the map of frequency
 * @param DICTIONARY the dictionary
 * @param BASIS_VECTOR the vector of ints that represents the basis
 * @param WORD_VECTORS the vector of vectors we shall fill
 * @param TOTAL_COUNT the total count of all the words in ukwac
 * @return int whether we succeeded or not
 */

int read_count(string OUTPUT_DIR, map<string, size_t> & FREQ, vector<string> & DICTIONARY, vector<int>  & BASIS_VECTOR, vector< vector<float> > & WORD_VECTORS, size_t TOTAL_COUNT, set<int> & WORDS_TO_CHECK) {
  cout << "Reading the word_vectors count" << endl;
  std::ifstream count_file (OUTPUT_DIR + "/word_vectors.txt");
  string line;
  size_t idx = 0;
 
  if (!count_file.is_open()) {
    return 1;
  }

  while ( getline (count_file,line) && idx < DICTIONARY.size()) {
    line = s9::RemoveChar(line,'\n');
    vector<string> tokens = s9::SplitStringWhitespace(line);
   
    vector<float> tv; 

		if (WORDS_TO_CHECK.find(idx) != WORDS_TO_CHECK.end())	{

			for (int i =0; i < tokens.size(); ++i) {
				float ct = static_cast<float>(FREQ[DICTIONARY[BASIS_VECTOR[i]]]);
				float pmi = 0;

				if (ct != 0.0){					
					float cc = static_cast<float>(FREQ[DICTIONARY[idx]]);
					if (cc != 0.0) {
						float cct = s9::FromString<float>(tokens[i]);
						if (cct != 0.0) {
							pmi = log( (cct/ ct) / (cc / static_cast<float>(TOTAL_COUNT)));
						}
					}
				}
				tv.push_back(pmi);

			}
		}

		WORD_VECTORS.push_back(tv);
    idx++;
  }
  return 0;
}


/**
 * Read in the word vector counts for analysis. We keep the RAW values
 * @param OUTPUT_DIR the output directory
 * @param DICTIONARY the dictionary
 * @param BASIS_VECTOR the vector of ints that represents the basis
 * @param WORD_VECTORS the vector of vectors we shall fill
 * @return int whether we succeeded or not
 */

int read_count_raw(string OUTPUT_DIR, vector<string> & DICTIONARY, vector<int>  & BASIS_VECTOR, vector< vector<float> > & WORD_VECTORS, set<int> & WORDS_TO_CHECK) {
  cout << "Reading the word_vectors count" << endl;
  std::ifstream count_file (OUTPUT_DIR + "/word_vectors.txt");
  string line;
  size_t idx = 0;
 
  if (!count_file.is_open()) {
    return 1;
  }

  while ( getline (count_file,line) && idx < DICTIONARY.size()) {
    line = s9::RemoveChar(line,'\n');
    vector<string> tokens = s9::SplitStringWhitespace(line);
   
    vector<float> tv; 

		if (WORDS_TO_CHECK.find(idx) != WORDS_TO_CHECK.end())	{
      for (int i=0; i < tokens.size(); ++i) {
		    float cct = s9::FromString<float>(tokens[i]);
			  tv.push_back(cct);
      }
		}

		WORD_VECTORS.push_back(tv);
    idx++;
  }
  return 0;
}


