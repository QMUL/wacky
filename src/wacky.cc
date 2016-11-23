/**
 * A small program that pre-processes the ukWaC dataset for reading in python
 * 
 * It creates 5 sets of files from ukWaC
 * 1) A Vocab / Frequency file that lists the frequency of all tokens - word, freq\n - freq.txt
 * 2) A dictionary of VOCAB_SIZE words in alphabetical order - word\n - dictionary.txt
 * 3) A set of files that form the sentences, one word per line
 * 4) A set of files that form the sentences with integer lookups into the dictionary 
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

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/filesystem.hpp>

#include "string_utils.hpp"

using namespace std;
using namespace boost::filesystem;
using namespace boost::interprocess;

// Our master map and set
map<string, size_t> freq {};
vector< pair<string,size_t> > freq_flipped {};
set<string> word_ignores {",","-",".","<text","<s>xt","</s>SENT", "<s>>SENT", "<s>", "</s>", "<text>", "</text>"};
map<string,int> dictionary_fast {};
vector<string> dictionary {};
vector< vector<int> > verb_subjects;

const size_t VOCAB_SIZE = 50000; // Really more of a max

vector<string>::iterator find_in_dictionary(string s){

  vector<string>::iterator it;

  for (it = dictionary.begin(); it != dictionary.end(); ++it){
    if (it->compare(s) == 0){
      break;
    }
    // Quit early due to alphabetic order
    if (it->compare(s) > 0){
      return dictionary.end();
    }
  }
  return it;

}


bool sort_freq (pair<string,size_t> i, pair<string, size_t> j) { return (i.second > j.second); }

// Create a dictionary by flipping the freq around, taking the top VOCAB_SIZE 
// and then sorting into alphabetical order

int create_dictionary(){
  cout << "Creating Dictionary File" << endl;

  size_t idx = 0; 

  for (auto it = freq.begin(); it != freq.end(); it++){
    freq_flipped.push_back(*it);
  }

  std::sort(freq_flipped.begin(), freq_flipped.end(), sort_freq);

  for (auto it = freq_flipped.begin(); it != freq_flipped.end(); it++) {
    dictionary.push_back(it->first);
    idx++;
    if (idx >= VOCAB_SIZE){
      break;
    }  
  }
 
  std::sort(dictionary.begin(), dictionary.end());

  dictionary.push_back(string("UNK"));
  idx = 0;
  std::ofstream dictionary_file ("dictionary.txt");
  dictionary_file << s9::ToString(dictionary.size()) << endl;
  for (auto it : dictionary){
    dictionary_file << it << endl;
    dictionary_fast[it] = idx;
    idx++;
  }
  dictionary_file.flush();
  dictionary_file.close();

  return 0;
}


// Create the intial freq frequency map
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
      string filename = "words_" + s9::FilenameFromPath(filepath) + ".txt";
      std::ofstream words_file (filename);
#endif

      size_t step = size / num_blocks;
      cout << "Step Size " << step <<endl;

      // Set the starting positions and the sizes, by finding the nearest end sentence marker
      // that occurs after the guessed block border. This likely means the last block
      // will be the smallest
      
      std::string ssm = "0000";
      int sc = 0;

      block_pointer[0] = static_cast<char*>(addr);
      char *mem = static_cast<char*>(addr);
      for (int i=1; i < num_blocks; ++i) {
        mem += step;
        while (ssm.compare("</s>") != 0){
          ssm[sc] = *mem;
          mem++;
          sc++;
          if (sc > 3){
            sc = 0;
          }
        }
        
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
            // Can now look at the string and work on our freq
            vector<string> tokens = s9::SplitStringWhitespace(str);  
            if (tokens.size() > 0) {
              string val = s9::ToLower(tokens[0]);
              if (s9::IsAsciiPrintableString(val)){
                if (word_ignores.find(val) == word_ignores.end()){  

#ifdef _WRITE_WORDS
                  #pragma omp critical
                  words_file << val << " ";
#endif
                  auto result = freq.find(val); 
                  if (result == freq.end()){
                    #pragma omp critical
                    { 
                      freq[val] = 1;
                    }
                  }  else {
                    #pragma omp atomic 
                    freq[val] = freq[val] + 1;
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

  std::ofstream freq_file ("freq.txt");
  if (freq_file.is_open()) {
    freq_file << s9::ToString(freq.size()) << endl;
    for (auto it = freq.begin(); it != freq.end(); ++it) {
      freq_file << it->first << ", " << it->second << endl;
    }
    freq_file.close();
  } else {
    cout << "Unable to open freq file for writing" << endl;
    return 1;
  }
  
  cout << "Finished writing freq file" << endl;

  return 0;

}

// Create integer versions of all the strings
// I suspect this is the one that takes the time as the dictionary lookup will be slow :/
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
  
      size_t step = size / num_blocks;
      cout << "Step Size " << step <<endl;

      // Set the starting positions and the sizes, by finding the nearest newline
      // that occurs after the guessed block border. This likely means the last block
      // will be the smallest
      std::string ssm = "0000";
      int sc = 0;

      block_pointer[0] = static_cast<char*>(addr);
      char *mem = static_cast<char*>(addr);
      for (int i=1; i < num_blocks; ++i) {
        mem += step;
        while (ssm.compare("</s>") != 0){
          ssm[sc] = *mem;
          mem++;
          sc++;
          if (sc > 3){
            sc = 0;
          }
        }  
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

      #pragma omp parallel
      {   
        int block_id = omp_get_thread_num();
        char *mem = block_pointer[block_id];
        size_t file_count = 0;
        std::string str;

        string filename = "integers_" + s9::FilenameFromPath(filepath) + "_" + s9::ToString(block_id) + ".txt";
        
        std::ofstream int_file (filename);

        if (!int_file.is_open()) {
          cout << "ERROR: Unable to up " << filename << " for writing." << endl;
        }

        for(std::size_t i = 0; i < block_size[block_id]; ++i){
          char data = *mem;
          if (data != '\n' && data != '\r'){
            str += data;
          } else {       
            // Can now look at the string and work on our freq
            vector<string> tokens = s9::SplitStringWhitespace(str);  
            if (tokens.size() > 0) {
              string val = s9::ToLower(tokens[0]);
              
              if (dictionary_fast.find(val) == dictionary_fast.end()){
                int_file << s9::ToString(VOCAB_SIZE) << endl;
                unk_count++; 
              } else {
                int_file << s9::ToString( dictionary_fast[val] ) << endl;
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

        filename = "size_" + s9::FilenameFromPath(filepath) + "_" + s9::ToString(block_id) + ".txt";
        std::ofstream size_file (filename);
        size_file << s9::ToString(file_count) << endl;
        size_file.close();

      }

    } catch (interprocess_exception &ex) {
      fprintf(stderr, "Exception %s\n", ex.what());
      fflush(stderr);
      return 1;
    }
  }

  std::ofstream unk_file ("unk_count.txt");
  if (unk_file.is_open()) {
    unk_file << s9::ToString(unk_count) << endl;
    unk_file.close();
  } else {
    cout << "Unable to open unk file for writing" << endl;
    return 1;
  }

  std::ofstream total_file ("total_count.txt");
  if (total_file.is_open()) {
    total_file << s9::ToString(total_count) << endl;
    total_file.close();
  } else {
    cout << "Unable to open total file for writing" << endl;
    return 1;
  }



  return 0;
}

// This function will read everything within the sentence tags, creating a file that links 
// verbs to objects via the dictionary
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
  
      size_t step = size / num_blocks;
      cout << "Step Size " << step <<endl;

      // Set the starting positions and the sizes, by finding the nearest newline
      // that occurs after the guessed block border. This likely means the last block
      // will be the smallest
      std::string ssm = "0000";
      int sc = 0;

      block_pointer[0] = static_cast<char*>(addr);
      char *mem = static_cast<char*>(addr);
      for (int i=1; i < num_blocks; ++i) {
        mem += step;
        while (ssm.compare("</s>") != 0){
          ssm[sc] = *mem;
          mem++;
          sc++;
          if (sc > 3){
            sc = 0;
          }
        }  
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

      // Progress basically
      size_t progress = 0;

      #pragma omp parallel
      {   
        int block_id = omp_get_thread_num();
        char *mem = block_pointer[block_id];
        size_t file_count = 0;
        std::string str;
        std::string ssmt = "0000";
        int sct = 0;

        //string filename = "subjects_" + s9::FilenameFromPath(filepath) + "_" + s9::ToString(block_id) + ".txt";
        
        //std::ofstream sub_file (filename);

        //if (!sub_file.is_open()) {
        //  cout << "ERROR: Unable to up " << filename << " for writing." << endl;
        //}

        for(std::size_t i = 0; i < block_size[block_id]; ++i){
          char data = *mem;
          
          if (ssmt.compare("</s>") != 0){
            str += data;
            ssmt[sct] = data;
            sct++;
          
            if (sct>3){
              sct = 0;
            }
          } else {       
            // Can now look at the sentence, derive its structure and find the bits we need
          

            vector<string> lines = s9::SplitStringNewline(str);

            for (int i = 0; i < lines.size(); ++i){
              vector<string> tokens = s9::SplitStringWhitespace(lines[i]);  
              if (tokens.size() > 5) {
              
                // Find the Subjects
                if (s9::StringContains(tokens[5],"SBJ")  && s9::StringContains(tokens[2],"NN")){
                  // Follow the chain to the root, stopping when we hit a verb
                 
                  int target = s9::FromString<int>(tokens[4]);
                  string sbj = s9::ToLower(tokens[0]);
                  auto vidx = dictionary_fast.find(sbj); 

                  if (vidx != dictionary_fast.end()){
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
                        if (s9::StringContains(tokens2[5],"V")){
                         
                          string verb = s9::ToLower(tokens2[0]);
                          
                          auto widx = dictionary_fast.find(verb);
                          if (widx != dictionary_fast.end()){
                            verb_subjects[widx->second].push_back(vidx->second);
                            target = 0; // Just record the one direct verb 
                          }
                        }
                      } else {
                        // We got a duff line so quit
                        target = 0;
                      }
                    }
                  } 
                }
              }
            }         
            str = "";
          }
          mem++;

          // Progress output to console
          //#pragma omp critical
          //{
          //  progress +=1;
          //  cout << "Progress " << static_cast<float>(progress) / static_cast<float>(size) << '\r';
          //}
        }
        // sub_file.close();
      }

    } catch (interprocess_exception &ex) {
      fprintf(stderr, "Exception %s\n", ex.what());
      fflush(stderr);
      return 1;
    }
  }

  // Write out the subject file as lines of numbers.
  // First number is the verb. All following numbers are the subjects
  string filename = "subjects.txt";
  std::ofstream sub_file (filename);
  int idv = 0;
  for (vector<int> verbs : verb_subjects){
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


int main(int argc, char* argv[]) {

  string p(argc <= 1 ? "." : argv[1]);
  
  vector<string> filenames;

  // Scan directory for the files
  if (is_directory(p)) {
    
    DIR *dir;
    struct dirent *ent;

    dir = opendir (p.c_str());
    while ((ent = readdir (dir)) != NULL) {

      // Files to ignore
      if (strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0){
        continue;
      }
      string fullpath;
      fullpath = p + "/" + string(ent->d_name);
      filenames.push_back(fullpath);
    }

  } else { // Incorrect directory given
    cout << "Incorrect command line argument for directory" << endl;
    return 1;
  }

  // Initialise our verb to subject map
  for (int i = 0; i <= VOCAB_SIZE; ++i) {
    verb_subjects.push_back( vector<int>() );
  }


  if (create_freq(filenames) != 0) { return 1; }
  if (create_dictionary() != 0) { return 1; }
  if (create_verb_subject(filenames) != 0) { return 1; }
  //if (create_integers(filenames) != 0) { return 1; }
  return 0;
}
