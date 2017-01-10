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

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/filesystem.hpp>

#include "string_utils.hpp"

using namespace std;
using namespace boost::filesystem;
using namespace boost::interprocess;

// Our master map and set
map<string, size_t> FREQ {};
vector< pair<string,size_t> > FREQ_FLIPPED {};
set<string> WORD_IGNORES {",","-",".","@card@", "<text","<s>xt","</s>SENT", "<s>>SENT", "<s>", "</s>", "<text>", "</text>"};
map<string,int> DICTIONARY_FAST {};
vector<string> DICTIONARY {};
vector< vector<int> > VERB_SUBJECTS;
vector< vector<int> > WORD_VECTORS;
vector<int> BASIS_VECTOR;
vector<string> SIMVERBS;
vector<int> SIMVERBS_COUNT;
vector<int> SIMVERBS_OBJECTS;
vector<int> SIMVERBS_ALONE;


size_t VOCAB_SIZE = 500000;  // Really more of a max
size_t BASIS_SIZE = 5000;   // When creating the WORD_VECTORS how large are these vectors?
string OUTPUT_DIR = ".";
bool  LEMMA_TIME = true;   // Use the lemma or stem of the word
size_t IGNORE_WINDOW = 100; // How many most frequent words do we ignore completely in the basis?
size_t WINDOW_SIZE = 5;			// Our sliding window size, either side of the chosen word

vector<string>::iterator find_in_dictionary(string s){

  vector<string>::iterator it;

  for (it = DICTIONARY.begin(); it != DICTIONARY.end(); ++it){
    if (it->compare(s) == 0){
      break;
    }
    // Quit early due to alphabetic order
    if (it->compare(s) > 0){
      return DICTIONARY.end();
    }
  }
  return it;
}


bool sort_freq (pair<string,size_t> i, pair<string, size_t> j) { return (i.second > j.second); }

// For word vectors, we take a subset of the vocab - the basis
void _create_basis() {
  // At this point we can also create the basis vector as the top of the dictionary
  // We write this out too
  // We dont add UNK to the basis
  int idx = 0;

  for (auto it = FREQ_FLIPPED.begin(); it != FREQ_FLIPPED.end(); it++) {
    if (!s9::StringContains(it->first,"UNK")){ 
      if (idx > IGNORE_WINDOW) {
        BASIS_VECTOR.push_back(DICTIONARY_FAST[it->first]);
      }
      idx++;
      if (idx >= BASIS_SIZE + IGNORE_WINDOW){
        break;
      }
    }   
  }
  
  std::ofstream basis_file (OUTPUT_DIR + "/basis.txt");
  for (auto it : BASIS_VECTOR){
    basis_file << it << endl;
  }
  basis_file.flush();
  basis_file.close();

}

// Create a DICTIONARY by flipping the FREQ around, taking the top VOCAB_SIZE 
// and then sorting into alphabetical order
// TODO - should make the global refs more explicit
int create_dictionary(){
  cout << "Creating Dictionary File" << endl;

  size_t idx = 0; 

  for (auto it = FREQ.begin(); it != FREQ.end(); it++){
    FREQ_FLIPPED.push_back(*it);
  }

  std::sort(FREQ_FLIPPED.begin(), FREQ_FLIPPED.end(), sort_freq);

  for (auto it = FREQ_FLIPPED.begin(); it != FREQ_FLIPPED.end(); it++) {
    DICTIONARY.push_back(it->first);
    idx++;
    if (idx >= VOCAB_SIZE){
      break;
    }  
  }
 
  std::sort(DICTIONARY.begin(), DICTIONARY.end());

  // I suspect we dont actually need the total count
  DICTIONARY.push_back(string("UNK"));
  idx = 0;
  std::ofstream dictionary_file (OUTPUT_DIR + "/dictionary.txt");
  dictionary_file << s9::ToString(DICTIONARY.size()) << endl;
  for (auto it : DICTIONARY){
    dictionary_file << it << endl;
    DICTIONARY_FAST[it] = idx;
    idx++;
  }
  dictionary_file.flush();
  dictionary_file.close();

  _create_basis();

  return 0;
}

// If we've already generated a dictionary read it in
int read_dictionary(){

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
  _create_basis();
}

// Create the intial FREQ FREQuency map
int create_freq(vector<string> filenames) {
  // Scan directory for the files
  for (string filepath : filenames){

    cout << filepath << endl;

    int num_blocks =1; 
    
    #pragma omp parallel
    {
      num_blocks = omp_get_num_threads();
    }
  
    char * block_pointer[num_blocks];
    size_t block_size[num_blocks];
    cout << "Num Blocks: " << num_blocks << endl;

    // Problem with memory-mapped files is we need the number of line
    // endings in order to split the file for OpenMP processing on the same
    // file. Using one thread per file is probably easier. 
    //
    // OR we could just move the pointers within the file backwards till we hit
    // a newline and set it there. That would probably be quite easy

    try {

      file_mapping m_file(filepath.c_str(), read_only);
      mapped_region region(m_file, read_only);  

      void * addr = region.get_address();
      size_t size = region.get_size();
  
#ifdef _WRITE_WORDS  
      string filename = OUTPUT_DIR + "/words_" + s9::FilenameFromPath(filepath) + ".txt";
      std::ofstream words_file (filename);
#endif

      size_t step = size / num_blocks;
      cout << "Step Size " << step <<endl;

      // Set the starting positions and the sizes, by finding the nearest end sentence marker
      // that occurs after the guessed block border. This likely means the last block
      // will be the smallest
      
      std::string ssm = "0000";
      
      block_pointer[0] = static_cast<char*>(addr);
      char *mem = static_cast<char*>(addr);
      for (int i=1; i < num_blocks; ++i) {
        mem += step;
        while (ssm.compare("</s>") != 0){
        	ssm[0] = ssm[1];
        	ssm[1] = ssm[2];
        	ssm[2] = ssm[3];
					ssm[3] = *mem;
          mem++;
				}
       	ssm = "0000"; 
        block_pointer[i] = mem;
      }
  
      size_t csize = 0;
      for (int i = 0; i < num_blocks-1; ++i) {
        block_size[i] = block_pointer[i+1] - block_pointer[i];
        csize += block_size[i];
      }

      if (num_blocks > 1){
        block_size[num_blocks-1] = size - csize;
      } else {
        block_size[0] = size;
      }

      cout << "Block Sizes: " << endl;
      for (int i = 0; i < num_blocks; ++i) {
        cout << i << " " <<  block_size[i] << endl;
      }

      // Now begin the threading block

      // Probably a little slow reading char by char right?
      // Also potential unicode errors? Maybe
      #pragma omp parallel
      {   
        int block_id = omp_get_thread_num();
        char *mem = block_pointer[block_id];
        string str;

        for(std::size_t i = 0; i < block_size[block_id]; ++i){
          char data = *mem;
          if (data != '\n' && data != '\r'){
            str += data;
          } else {
            // Can now look at the string and work on our FREQ
            vector<string> tokens = s9::SplitStringWhitespace(str);  
            if (tokens.size() > 1) {
              string val = s9::ToLower(tokens[0]);
              if (LEMMA_TIME) {
                val = s9::ToLower(tokens[1]); // Use the canonical form of the word
              }

              if (s9::IsAsciiPrintableString(val)){
                if (WORD_IGNORES.find(val) == WORD_IGNORES.end()){  

#ifdef _WRITE_WORDS
                  #pragma omp critical
                  words_file << val << " ";
#endif
                  auto result = FREQ.find(val); 
                  if (result == FREQ.end()){
                    #pragma omp critical
                    { 
                      FREQ[val] = 1;
                    }
                  }  else {
                    #pragma omp atomic 
                    FREQ[val] = FREQ[val] + 1;
                  }
                }
              }
            }
            str = "";
          }
          mem++;
        }
      
      }

#ifdef _WRITE_WORDS
      words_file.flush();
      words_file.close();
#endif

    } catch (interprocess_exception &ex) {
      fprintf(stderr, "Exception %s\n", ex.what());
      fflush(stderr);
      return 1;
    }
    // remove memory map ?
  }

  std::ofstream freq_file (OUTPUT_DIR + "/freq.txt");
  if (freq_file.is_open()) {
    freq_file << s9::ToString(FREQ.size()) << endl;
    for (auto it = FREQ.begin(); it != FREQ.end(); ++it) {
      freq_file << it->first << ", " << it->second << endl;
    }
    freq_file.close();
  } else {
    cout << "Unable to open FREQ file for writing" << endl;
    return 1;
  }
  
  cout << "Finished writing FREQ file" << endl;

  return 0;

}

// If we've already generated a frequency read it in
int read_freq(){
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

  return 0;
}

// Breakup our files into blocks 

int _breakup ( char ** block_pointer, size_t * block_size, file_mapping &m_file, mapped_region &region) {
  // Scan directory for the files
	int num_blocks =1; 
		
	#pragma omp parallel
	{
		num_blocks = omp_get_num_threads();
	}

	cout << "Num Blocks: " << num_blocks << endl;

	// Problem with memory-mapped files is we need the number of line
	// endings in order to split the file for OpenMP processing on the same
	// file. Using one thread per file is probably easier. 
	//
	// OR we could just move the pointers within the file backwards till we hit
	// a newline and set it there. That would probably be quite easy

	try {

		void * addr = region.get_address();
		size_t size = region.get_size();

		size_t step = size / num_blocks;
		cout << "Step Size " << step <<endl;

		// Set the starting positions and the sizes, by finding the nearest newline
		// that occurs after the guessed block border. This likely means the last block
		// will be the smallest
		std::string ssm = "0000";

		block_pointer[0] = static_cast<char*>(addr);
		char *mem = static_cast<char*>(addr);
		for (int i=1; i < num_blocks; ++i) {
			mem += step;
			while (ssm.compare("</s>") != 0){
				ssm[0] = ssm[1];
				ssm[1] = ssm[2];
				ssm[2] = ssm[3];
				ssm[3] = *mem;
				mem++;
				
			}
			ssm = "0000"; 
			block_pointer[i] = mem;
		}

		size_t csize = 0;
		for (int i = 0; i < num_blocks-1; ++i) {
			block_size[i] = block_pointer[i+1] - block_pointer[i];
			csize += block_size[i];
		}

		if (num_blocks > 1){
			block_size[num_blocks-1] = size - csize;
		} else {
			block_size[0] = size;
		}

		cout << "Block Sizes: " << endl;
		for (int i = 0; i < num_blocks; ++i) {
			cout << i << " " <<  block_size[i] << endl;
		}

	} catch (interprocess_exception &ex) {
		fprintf(stderr, "Exception %s\n", ex.what());
		fflush(stderr);
		return 1;
	}

	return 0;
}


// Create a file of word vectors based on counts

int create_word_vectors(vector<string> filenames) {
	
  int num_blocks =1; 
		
	#pragma omp parallel
	{
		num_blocks = omp_get_num_threads();
	}

	// Start by setting the counts - we add an extra 1 for the UNK value (but UNK does not occur in the basis)
	for (int i =0; i < VOCAB_SIZE+1; ++i) {
	  vector<int> ti; 	
		ti.reserve(VOCAB_SIZE);
		for (int j=0; j < BASIS_SIZE; ++j) {
			ti.push_back(0);
		}	
		WORD_VECTORS.push_back(ti);
	}

  for( string filepath : filenames) {

    char * block_pointer[num_blocks];
    size_t block_size[num_blocks];

    // Cant pass this into _breakup sadly
		file_mapping m_file(filepath.c_str(), read_only);
		mapped_region region(m_file, read_only);  

		int result = _breakup(block_pointer, block_size, m_file, region );
		if (result == -1){
			return -1;
		}	

    cout << "Reading file " << filepath << endl;
		
		#pragma omp parallel
		{   
			int block_id = omp_get_thread_num();
			char *mem = block_pointer[block_id];
			size_t file_count = 0;
			std::string str;
			std::vector<int> sentence;
      bool recording = false;

			for(std::size_t i = 0; i < block_size[block_id]; ++i){
				char data = *mem;
				if (data != '\n' && data != '\r'){
					str += data;
				} else {  
          //  Can now look at the string and work on our FREQ
          vector<string> tokens = s9::SplitStringWhitespace(str);  
          
          // Essentially, we capture a sentence and then look at each word
          // and the WINDOW_SIZE of words before and after it, and update a 
          // count if that word occurs in the BASIS_VECTOR 

          if (tokens.size() > 0){
            string val = s9::ToLower(tokens[0]);

            if (s9::StringContains(val,"</s>")){
              // Stop sentence
              recording = false;
              // Now update the counts
              for (int idw = 0; idw < sentence.size(); ++idw){
                // look below
                for (int jdw = idw-1; jdw > idw - WINDOW_SIZE && jdw >= 0; --jdw){
                  int ji = sentence[jdw];

                  for (int bv = 0; bv < BASIS_SIZE; ++bv){
                    if (BASIS_VECTOR[bv] == ji){
                      // Probably could be faster here
                      #pragma omp atomic
                      WORD_VECTORS[ sentence[idw] ][bv] +=1;
                      break;
                    }
                  }    
                }

                // look above
                for (int jdw = idw+1; jdw < idw + WINDOW_SIZE && jdw < sentence.size(); ++jdw){

                  int ji = sentence[jdw];
                  for (int bv = 0; bv < BASIS_SIZE; ++bv){

                    if (BASIS_VECTOR[bv] == ji){
                      #pragma omp atomic
                      WORD_VECTORS[ sentence[idw] ][bv] +=1;
                      break;
                    }
                  }    
                } 
              }
              sentence.clear(); 

            } else if (s9::StringContains(val,"<s>")){
              // start sentence
              recording = true;
            } else if (recording) {

              if (tokens.size() > 1) {
                string lemma = val;
              
                if (LEMMA_TIME) {
                  lemma = s9::ToLower(tokens[1]);
                }

                if (DICTIONARY_FAST.find(lemma) == DICTIONARY_FAST.end()){
                  sentence.push_back(VOCAB_SIZE);
                } else {
                  sentence.push_back(DICTIONARY_FAST[lemma] );
                }
              }
            }
          } 
          str = "";
				}
				mem++;
			}    
		} // end parallel bit
	}

	// Finished all the files, now quit
  std::ofstream wv_file (OUTPUT_DIR + "/word_vectors.txt");
  if (wv_file.is_open()) {
		size_t idx = 0;
		for (vector<int> tv : WORD_VECTORS){
			for (int ti : tv){
    		wv_file << s9::ToString(ti) << " ";
			}
			wv_file << endl;
		}
  	wv_file.close();
  } else {
    cout << "Unable to open word_vec file for writing" << endl;
    return 1;
  }

	return 0;
}


// Create integer versions of all the strings
// I suspect this is the one that takes the time as the DICTIONARY lookup will be slow :/
int create_integers(vector<string> filenames) {

  cout << "Creating Integer Files" << endl;

  size_t unk_count = 0;
  size_t total_count = 0;

  // Scan directory for the files
  for( string filepath : filenames) {
    int num_blocks =1; 
      
    #pragma omp parallel
    {
      num_blocks = omp_get_num_threads();
    }
    
    char * block_pointer[num_blocks];
    size_t block_size[num_blocks];

    // Cant pass this into _breakup sadly
		file_mapping m_file(filepath.c_str(), read_only);
		mapped_region region(m_file, read_only);  

		int result = _breakup(block_pointer, block_size, m_file, region );
		if (result == -1){
			return -1;
		}	
 
    #pragma omp parallel
    {   
      int block_id = omp_get_thread_num();
      char *mem = block_pointer[block_id];
      size_t file_count = 0;
      std::string str;

      string filename = OUTPUT_DIR + "/integers_" + s9::FilenameFromPath(filepath) + "_" + s9::ToString(block_id) + ".txt";
      
      std::ofstream int_file (filename);

      if (!int_file.is_open()) {
        cout << "ERROR: Unable to up " << filename << " for writing." << endl;
      }

      for(std::size_t i = 0; i < block_size[block_id]; ++i){
        char data = *mem;
        if (data != '\n' && data != '\r'){
          str += data;
        } else {       
          // Can now look at the string and work on our FREQ
          vector<string> tokens = s9::SplitStringWhitespace(str);  
          if (tokens.size() > 1) {
            string val = s9::ToLower(tokens[0]);
            
            if (LEMMA_TIME) {
              val = s9::ToLower(tokens[1]);
            }

            if (DICTIONARY_FAST.find(val) == DICTIONARY_FAST.end()){
              int_file << s9::ToString(VOCAB_SIZE) << endl;
              unk_count++; 
            } else {
              int_file << s9::ToString( DICTIONARY_FAST[val] ) << endl;
              #pragma omp critical
              total_count++;
              file_count++;
            }
          } 
          str = "";
        }
        mem++;
      }    
      int_file.flush();
      int_file.close();

      filename = OUTPUT_DIR + "/size_" + s9::FilenameFromPath(filepath) + "_" + s9::ToString(block_id) + ".txt";
      std::ofstream size_file (filename);
      size_file << s9::ToString(file_count) << endl;
      size_file.close();

    }

  }

  std::ofstream unk_file (OUTPUT_DIR + "/unk_count.txt");
  if (unk_file.is_open()) {
    unk_file << s9::ToString(unk_count) << endl;
    unk_file.close();
  } else {
    cout << "Unable to open unk file for writing" << endl;
    return 1;
  }

  std::ofstream total_file (OUTPUT_DIR + "/total_count.txt");
  if (total_file.is_open()) {
    total_file << s9::ToString(total_count) << endl;
    total_file.close();
  } else {
    cout << "Unable to open total file for writing" << endl;
    return 1;
  }

  return 0;
}

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


// This function will read everything within the sentence tags, creating a file that links 
// verbs to objects via the DICTIONARY
int create_verb_subject(vector<string> filenames) {
  cout << "Creating Verb Subject" << endl;

  size_t unk_count = 0;
  size_t total_count = 0;

  // Scan directory for the files
  for( string filepath : filenames) {
    int num_blocks =1; 
      
    #pragma omp parallel
    {
      num_blocks = omp_get_num_threads();
    }
  
    cout << "Num Blocks: " << num_blocks << endl;

    char * block_pointer[num_blocks];
    size_t block_size[num_blocks];

    // Cant pass this into _breakup sadly
		file_mapping m_file(filepath.c_str(), read_only);
		mapped_region region(m_file, read_only);  

		int result = _breakup(block_pointer, block_size, m_file, region );
		if (result == -1){
			return -1;
		}	
 
   
    // Progress basically
    size_t progress = 0;

    #pragma omp parallel
    {   
      int block_id = omp_get_thread_num();
      char *mem = block_pointer[block_id];
      size_t file_count = 0;
      std::string str_buffer;
      std::string ssmt = "0000";

      //string filename = "subjects_" + s9::FilenameFromPath(filepath) + "_" + s9::ToString(block_id) + ".txt";
      
      //std::ofstream sub_file (filename);

      //if (!sub_file.is_open()) {
      //  cout << "ERROR: Unable to up " << filename << " for writing." << endl;
      //}

      for(std::size_t i = 0; i < block_size[block_id]; ++i){
        char data = *mem;
        if (ssmt.compare("</s>") != 0){
          str_buffer += data;
          
          ssmt[0] = ssmt[1];
          ssmt[1] = ssmt[2];
          ssmt[2] = ssmt[3];
          ssmt[3] = data;
          
        } else {       
          // Can now look at the sentence, derive its structure and find the bits we need
          ssmt = "0000";

          vector<string> lines = s9::SplitStringNewline(str_buffer);
          
          str_buffer = "";

          for (int k = 0; k < lines.size(); ++k){
            vector<string> tokens = s9::SplitStringWhitespace(lines[k]);  
            if (tokens.size() > 5) {
            
              // Find the Subjects
              if (s9::StringContains(tokens[5],"SBJ")  && s9::StringContains(tokens[2],"NN")){
              //if (s9::StringContains(tokens[5],"SBJ")){
                // Follow the chain to the root, stopping when we hit a verb
               
                int target = s9::FromString<int>(tokens[4]);
                string sbj = s9::ToLower(tokens[0]);
                // Not sure if we want lemma time here?
                if (LEMMA_TIME){
                  sbj = s9::ToLower(tokens[1]);
                }
                
                auto vidx = DICTIONARY_FAST.find(sbj); 
                          
                if (vidx != DICTIONARY_FAST.end()){
                  // Walk up the tree adding verbs till we get to root

                  while (target != 0){
                   
                    // find the next line (we cant assume the indexing is perfect)
                    int tt = -1;
                    for (int j = 0; j < lines.size(); ++j){
                      vector<string> ttokens = s9::SplitStringWhitespace(lines[j]);
                      if (ttokens.size() > 5){
                        if (s9::FromString<int>(ttokens[3]) == target){
                          tt = j;
                          break;
                        }
                      }
                    }
                    
                    if (tt == -1){
                      break;
                    }

                    vector<string> tokens2 = s9::SplitStringWhitespace(lines[tt]); 
                    
                    if (tokens2.size() > 5) {
                      target = s9::FromString<int>(tokens2[4]);
                      
                      if (s9::StringContains(tokens2[2],"VV")){
                        // Do we use the actual root verb or the conjugated
                        string verb = s9::ToLower(tokens2[0]);
                        if (LEMMA_TIME){
                          verb = s9::ToLower(tokens2[1]);
                        }

                        auto widx = DICTIONARY_FAST.find(verb);
                        
                        if (widx != DICTIONARY_FAST.end()){
                          
                          // We record unique instances only
                          if (std::find(VERB_SUBJECTS[widx->second].begin(), VERB_SUBJECTS[widx->second].end(), vidx->second) == VERB_SUBJECTS[widx->second].end()) {
                            #pragma omp critical
                            {
                              VERB_SUBJECTS[widx->second].push_back(vidx->second);
                            }
                          }
                          target = 0; // Just record the one direct verb 
                        }
                      }
                      
                    } else {
                      // We got a duff line so quit
                      break;
                    }
                  }
                } 
              }
            }
          }         
        }
    
        mem++;
        progress +=1;

        /*#pragma omp master
        {
          if ( progress % 1000 == 0){
            // Progress output to console
            int pp = static_cast<int>((static_cast<float>(progress * omp_get_num_threads()) / static_cast<float>(size)) * 10.0);
            std::cout << std::setw(20);
            cout << "Progress: ";
            for (int l=0; l < pp; l++){
              cout << "#"; 
            }

            for (int l=pp; l < 10; l++){
              cout << "_";
            }
            cout << '\r';
          }
        }*/
      }
      // sub_file.close();
    }

  }

  cout << endl;

  // Write out the subject file as lines of numbers.
  // First number is the verb. All following numbers are the subjects
  string filename = OUTPUT_DIR + "/subjects.txt";
  std::ofstream sub_file (filename);
  int idv = 0;
  for (vector<int> verbs : VERB_SUBJECTS){
    if (verbs.size() > 0 ){
      sub_file << s9::ToString(idv) << " ";
      for (int sb : verbs){
        sub_file << s9::ToString(sb) << " ";
      }
      sub_file << endl;
    }
    idv++;
  }
  
  sub_file.close();

  return 0;
}

// Create integer versions of all the strings
// I suspect this is the one that takes the time as the DICTIONARY lookup will be slow :/
int create_simverbs(vector<string> filenames, string simverb_path) {

  size_t unk_count = 0;
  size_t total_count = 0;

  // Setup the basics from the simverb file

  std::ifstream simverb_file(simverb_path);
  string line;

  while ( getline (simverb_file,line) ) {
    line = s9::RemoveChar(line,'\n'); 
    vector<string> tokens = s9::SplitStringWhitespace(line);
    
    string val = tokens[0]; 
    if (std::find(SIMVERBS.begin(), SIMVERBS.end(), val) == SIMVERBS.end()) {
      SIMVERBS.push_back(val);  
    }

    val = tokens[1];
    if (std::find(SIMVERBS.begin(), SIMVERBS.end(), val) == SIMVERBS.end()) {
      SIMVERBS.push_back(val);  
    }
  }

  // Now setup all the counts

  for (int idw = 0; idw < SIMVERBS.size(); ++idw){
    SIMVERBS_COUNT.push_back(0);
    SIMVERBS_OBJECTS.push_back(0);
    SIMVERBS_ALONE.push_back(0);
  }

  cout << "simverbs size : " << SIMVERBS.size() << endl;

  // Scan directory for the files
  for( string filepath : filenames) {
    int num_blocks =1; 
      
    #pragma omp parallel
    {
      num_blocks = omp_get_num_threads();
    }
    
    char * block_pointer[num_blocks];
    size_t block_size[num_blocks];

    // Cant pass this into _breakup sadly
		file_mapping m_file(filepath.c_str(), read_only);
		mapped_region region(m_file, read_only);  

		int result = _breakup(block_pointer, block_size, m_file, region );
		if (result == -1){
			return -1;
		}	

    // Now do the search
    #pragma omp parallel
    {   
      int block_id = omp_get_thread_num();
      char *mem = block_pointer[block_id];
      size_t file_count = 0;
      std::string str_buffer;
      std::string ssmt = "0000";

      for(std::size_t i = 0; i < block_size[block_id]; ++i){
        char data = *mem;
        if (ssmt.compare("</s>") != 0){
          str_buffer += data;
          
          ssmt[0] = ssmt[1];
          ssmt[1] = ssmt[2];
          ssmt[2] = ssmt[3];
          ssmt[3] = data;
          
        } else {       
          // Can now look at the sentence, derive its structure and find the bits we need
          ssmt = "0000";
          vector<string> lines = s9::SplitStringNewline(str_buffer);
          
          str_buffer = "";

          vector<bool> verb_hit;
          vector<string> verbs;
          vector<int> sim_indices;
          vector<int> object_indices;

          for (int k = 0; k < lines.size(); ++k){

            // First find all the verbs in the sentence
            vector<string> tokens = s9::SplitStringWhitespace(lines[k]);  
            if (tokens.size() > 5) {
    
              if (s9::StringContains(tokens[2],"VV")){
                // Do we use the actual root verb or the conjugated
                string verb = s9::ToLower(tokens[0]);
                if (LEMMA_TIME){
                  verb = s9::ToLower(tokens[1]);
                }

                // Find the index of our verb
                int isw = 0;
                for (isw = 0; isw < SIMVERBS.size(); ++isw) {
                  if (SIMVERBS[isw].compare(verb) == 0){
                    verbs.push_back(verb);
                    verb_hit.push_back(false);
                    sim_indices.push_back(isw);
                  }
                }
      
              } else if (s9::StringContains(tokens[5],"OBJ")){
                object_indices.push_back(k);
              }
            }
          }

          // Now we have all the objects and verbs collected. Trace the objects backward

          for (int k : object_indices){
            vector<string> tokens = s9::SplitStringWhitespace(lines[k]);
            
            int target = s9::FromString<int>(tokens[4]);
            int start = target;

            while (target > 0){ 
              // find the next line (we cant assume the indexing is perfect)
              for (int j = 0; j < lines.size(); ++j){
                vector<string> target_tokens = s9::SplitStringWhitespace(lines[j]);
                if (target_tokens.size() > 5){
                  int st = s9::FromString<int>(target_tokens[3]); 
                  if (st == target){
                    target = st;

                    if (target_tokens.size() > 5) {
                
                      if (s9::StringContains(target_tokens[2],"VV")){
                        string verb = s9::ToLower(target_tokens[0]);
                        if (LEMMA_TIME){
                          verb = s9::ToLower(target_tokens[1]);
                        }
                        
                        for (int iv=0; iv < verbs.size(); ++iv){
                          if (verbs[iv].compare(verb) == 0){
                            verb_hit[iv] = true;
                          }
                        }
                      }
                    }
                  }
                }
              }
              if (target == start){
                break;
              }
                    
            }
          }

          // should be done with our sentence now so do some counting
          // TODO - as these are just increments we could go a little better with OpenMP
          for (int iv=0; iv < verbs.size(); ++iv){
            int ss = sim_indices[iv];
            if (verb_hit[iv]){
              #pragma omp critical
              SIMVERBS_OBJECTS[ss] +=1;               
            } else {
              #pragma omp critical
              SIMVERBS_ALONE[ss] +=1;
            }
            #pragma omp critical
            SIMVERBS_COUNT[ss] +=1;
          }
        }
        mem++;
      }
    }
  }

  string filename = OUTPUT_DIR + "/sim_stats.txt";
  std::ofstream sim_file (filename);
  int idv = 0;
  for (string verb : SIMVERBS){
    sim_file << verb << " " << SIMVERBS_OBJECTS[idv] << " " << SIMVERBS_ALONE[idv] << " " << SIMVERBS_COUNT[idv] << endl;
    idv++;
  }
  
  sim_file.close();
 
  return 0;
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

  while ((c = getopt(argc, (char **)argv, "u:o:v:ls:rc:biwn?")) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'u' :
        ukdir = std::string(optarg);
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
      default:
        std::cout << "?? getopt returned character code" << c << std::endl;
    }
  }

  cout << "Vocab Max Size: " << VOCAB_SIZE << endl; 
  
  //omp_set_dynamic(0);

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

  // Initialise our verb to subject map
  for (int i = 0; i <= VOCAB_SIZE; ++i) {
    VERB_SUBJECTS.push_back( vector<int>() );
  }
  
  if (combine) {
    cout << "Combining ukwac into a large file of text" << endl;
    combine_ukwac(filenames, combine_file);
    return 0;
  }

  if (read_in){
    cout << "Reading in dictionary and frequency data" << endl;
    read_freq();
    read_dictionary();
  } else {
    cout << "Creating frequency and dictionary" << endl;
    if (create_freq(filenames) != 0)  { return 1; }
    if (create_dictionary() != 0)     { return 1; }
  }
  
  if (verb_subject) {
    cout << "Create verb subject" << endl; 
    if (create_verb_subject(filenames) != 0)  { return 1; }
  }
  
  if (integers){
    cout << "Create integer files" << endl;
    if (create_integers(filenames) != 0)      { return 1; }
  }
  
  if (word_vectors){
    cout << "Creating word vectors" << endl;
    if (create_word_vectors(filenames) != 0)  { return 1; }
  }
  
  if (sim_verbs) {
    cout << "Creating simverbs" << endl;
    if (create_simverbs(filenames,simverb_file) !=0)  { return 1; }
  }

  return 0;
}
