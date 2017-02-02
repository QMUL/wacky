
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



// Create a DICTIONARY by flipping the FREQ around, taking the top VOCAB_SIZE 
// and then sorting into alphabetical order
// TODO - should make the global refs more explicit

int create_dictionary(string OUTPUT_DIR,
    map<string, size_t> & FREQ, 
    vector< pair<string,size_t> > & FREQ_FLIPPED,
    map<string,int> & DICTIONARY_FAST,
    vector<string> & DICTIONARY) {

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

  return 0;
}


// For word vectors, we take a subset of the vocab - the basis
void create_basis(string OUTPUT_DIR,
    map<string, size_t> & FREQ, 
    vector< pair<string,size_t> > & FREQ_FLIPPED,
    map<string,int> & DICTIONARY_FAST,
    vector<int> & BASIS_VECTOR,
    set<string> & ALLOWED_BASIS_WORDS,
    int BASIS_SIZE,
    int IGNORE_WINDOW ) {

  // At this point we can also create the basis vector as the top of the dictionary
  // We write this out too
  // We dont add UNK to the basis
  int idx = 0;

  for (auto it = FREQ_FLIPPED.begin(); it != FREQ_FLIPPED.end(); it++) {
    if (!s9::StringContains(it->first,"UNK")){ 
      if (idx > IGNORE_WINDOW) {
        if (ALLOWED_BASIS_WORDS.find(it->first) != ALLOWED_BASIS_WORDS.end()) {
          BASIS_VECTOR.push_back(DICTIONARY_FAST[it->first]);
					idx++;
        }
      } else{ 
        idx++;
      }
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

// Create the intial FREQ FREQuency map
// In addition this creates a special file that records the type of the word (VV, NN etc)
int create_freq(vector<string> filenames, 
    string OUTPUT_DIR,
    map<string, size_t> & FREQ, 
    vector< pair<string,size_t> > & FREQ_FLIPPED,
    map<string,int> & DICTIONARY_FAST,
    vector<string> & DICTIONARY
    set<string> & WORD_IGNORES,
    set<string> & ALLOWED_BASIS_WORDS,
    bool LEMMA_TIME) {

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
            if (tokens.size() > 2) {
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
                      if ( s9::StringContains(tokens[2],"NN") ||
                            s9::StringContains(tokens[2],"JJ") ||
                            s9::StringContains(tokens[2],"VV") ||
                            s9::StringContains(tokens[2],"RB")){
                        ALLOWED_BASIS_WORDS.insert(val);  
                      }
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
  

  // Write out the final frequency file
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

  // Write out the allowed basis words file
  std::ofstream allowed_file (OUTPUT_DIR + "/allowed.txt");
  if (allowed_file.is_open()) {
    for (auto it = ALLOWED_BASIS_WORDS.begin(); it != ALLOWED_BASIS_WORDS.end(); ++it) {
      allowed_file << *it << endl;
    }
    allowed_file.close();
  } else {
    cout << "Unable to open allowed.txt file for writing" << endl;
    return 1;
  }
  
  cout << "Finished writing ALLOWED file" << endl;

  return 0;

}


// Create a file of word vectors based on counts

int create_word_vectors(vector<string> filenames,
    string OUTPUT_DIR,
    map<string, size_t> & FREQ, 
    vector< pair<string,size_t> > & FREQ_FLIPPED,
    map<string,int> & DICTIONARY_FAST,
    vector<string> & DICTIONARY 
    vector<int> & BASIS_VECTOR,
    set<string> & WORD_IGNORES,
    vector< vector<float> > & WORD_VECTORS,
    set<string> & ALLOWED_BASIS_WORDS,
    int VOCAB_SIZE,
    int BASIS_SIZE,
    int WINDOW_SIZE,
    bool LEMMA_TIME) {
	
  int num_blocks =1; 
		
	#pragma omp parallel
	{
		num_blocks = omp_get_num_threads();
	}

	// Start by setting the counts - we add an extra 1 for the UNK value (but UNK does not occur in the basis)
	for (int i =0; i < VOCAB_SIZE+1; ++i) {
	  vector<float> ti; 	
		ti.reserve(VOCAB_SIZE);
		for (int j=0; j < BASIS_SIZE; ++j) {
			ti.push_back(0.0);
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
                      WORD_VECTORS[ sentence[idw] ][bv] += 1.0;
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
                      WORD_VECTORS[ sentence[idw] ][bv] +=1.0;
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
		for (vector<float> tv : WORD_VECTORS){
			for (float tf : tv){
        int ti = static_cast<int>(tf);
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
int create_integers(vector<string> filenames,
    string OUTPUT_DIR,
    map<string,int> & DICTIONARY_FAST,
    int VOCAB_SIZE,
    bool LEMMA_TIME) {

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


