#include "wacky_verb.hpp"

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace std;

void create_verb_objects(string str_buffer, vector<int> & verb_obj_pairs,
    map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_OBJECTS,
    bool UNIQUE_OBJECTS,
    bool LEMMA_TIME ) {

  vector<string> lines = s9::SplitStringNewline(str_buffer);
  
  for (int k = 0; k < lines.size(); ++k){
    vector<string> tokens = s9::SplitStringWhitespace(lines[k]);  
  
    if (tokens.size() > 5) {  
      // Find the Subjects
      if (s9::StringContains(tokens[5],"OBJ") 
           && ( s9::StringContains(tokens[2],"NN") || 
              s9::StringContains(tokens[2],"JJ"))) {
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
                  // We choose between unique or otherwise
                  if (UNIQUE_OBJECTS){
                    if (std::find(VERB_OBJECTS[widx->second].begin(), VERB_OBJECTS[widx->second].end(), vidx->second) == VERB_OBJECTS[widx->second].end()) {
                      #pragma omp critical
                      {
                        VERB_OBJECTS[widx->second].push_back(vidx->second);
                        verb_obj_pairs.push_back(widx->second);
                        verb_obj_pairs.push_back(s9::FromString<int>(tokens2[3]));
                        verb_obj_pairs.push_back(vidx->second);
                      }
                    }
                  } else {
                    #pragma omp critical
                    {
                      VERB_OBJECTS[widx->second].push_back(vidx->second);
                      verb_obj_pairs.push_back(widx->second);
                      verb_obj_pairs.push_back(s9::FromString<int>(tokens2[3]));
                      verb_obj_pairs.push_back(vidx->second);

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

// Given a buffer, setup the subjects for a verb
void create_verb_subjects(string str_buffer, vector<int> & verb_sbj_pairs,
    map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SUBJECTS,
    bool UNIQUE_SUBJECTS,
    bool LEMMA_TIME ) {

  vector<string> lines = s9::SplitStringNewline(str_buffer);
  
  for (int k = 0; k < lines.size(); ++k){
    vector<string> tokens = s9::SplitStringWhitespace(lines[k]);  
  
    if (tokens.size() > 5) {  
      // Find the Subjects
      if (s9::StringContains(tokens[5],"SBJ")  && (
            s9::StringContains(tokens[2],"NN") || 
            s9::StringContains(tokens[2],"JJ"))){
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
                  // We choose between unique or otherwise
                  if (UNIQUE_SUBJECTS){
                    if (std::find(VERB_SUBJECTS[widx->second].begin(), VERB_SUBJECTS[widx->second].end(), vidx->second) == VERB_SUBJECTS[widx->second].end()) {
                      #pragma omp critical
                      {
                        VERB_SUBJECTS[widx->second].push_back(vidx->second);
                        verb_sbj_pairs.push_back(widx->second);
                        verb_sbj_pairs.push_back(s9::FromString<int>(tokens2[3]));
                        verb_sbj_pairs.push_back(vidx->second);

                      }
                    }
                  } else {
                    #pragma omp critical
                    {
                      VERB_SUBJECTS[widx->second].push_back(vidx->second);
                      verb_sbj_pairs.push_back(widx->second);
                      verb_sbj_pairs.push_back(s9::FromString<int>(tokens2[3]));
                      verb_sbj_pairs.push_back(vidx->second);

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

// This function will read everything within the sentence tags, creating a file that links 
// verbs to objects via the DICTIONARY
int create_verb_subject_object(vector<string> filenames,
    string OUTPUT_DIR, 
    map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SBJ_OBJ,
    vector< vector<int> > & VERB_SUBJECTS,
    vector< vector<int> > & VERB_OBJECTS,
    bool UNIQUE_OBJECTS,
    bool UNIQUE_SUBJECTS,
    bool LEMMA_TIME ) {

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

		int result = breakup(block_pointer, block_size, m_file, region );
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

          vector<int> verb_obj_pairs;
          vector<int> verb_sbj_pairs;

          create_verb_subjects(str_buffer, verb_sbj_pairs, DICTIONARY_FAST, VERB_SUBJECTS, UNIQUE_SUBJECTS, LEMMA_TIME);
          create_verb_objects(str_buffer, verb_obj_pairs, DICTIONARY_FAST, VERB_OBJECTS, UNIQUE_OBJECTS,LEMMA_TIME );
      
          // We now need to match the objects and subjects to the same verb if it appears
          // Each vector has verb,id,word, verb,id,word... indices into the dictionary

          for (std::size_t j = 0; j < verb_obj_pairs.size(); j+=3){
          
            for (std::size_t k = 0; k < verb_sbj_pairs.size(); k+=3){
              if (verb_obj_pairs[j+1] == verb_sbj_pairs[k+1]){
                #pragma omp critical
                {
                  VERB_SBJ_OBJ[verb_obj_pairs[j]].push_back(verb_sbj_pairs[k+2]);
                  VERB_SBJ_OBJ[verb_obj_pairs[j]].push_back(verb_obj_pairs[j+2]);
                }
              }
            }  
          } 
            
          str_buffer = "";
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
  string filename = OUTPUT_DIR + "/verb_subjects.txt";
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

  filename = OUTPUT_DIR + "/verb_objects.txt";
  std::ofstream obj_file (filename);
  idv = 0;
  for (vector<int> verbs : VERB_OBJECTS){
    if (verbs.size() > 0 ){
      obj_file << s9::ToString(idv) << " ";
      for (int sb : verbs){
        obj_file << s9::ToString(sb) << " ";
      }
      obj_file << endl;
    }
    idv++;
  }
  
  obj_file.close();

  // Finally, write out the verb subject object pairings
  filename = OUTPUT_DIR + "/verb_sbj_obj.txt";
  std::ofstream obj_sbj_file (filename);
  idv = 0;
  for (vector<int> verbs : VERB_SBJ_OBJ){
    if (verbs.size() > 0 ){
      obj_sbj_file << s9::ToString(idv) << " ";
      for (int sb : verbs){
        obj_sbj_file << s9::ToString(sb) << " ";
      }
      obj_sbj_file << endl;
    }
    idv++;
  }
  
  obj_sbj_file.close();

  return 0;
}

// Create counts of how many times a verb has an object
int create_simverbs(vector<string> filenames, string simverb_path,
    string OUTPUT_DIR,
    vector<string> & SIMVERBS,
    vector<int> & SIMVERBS_COUNT,
    vector<int> & SIMVERBS_OBJECTS,
    vector<int> & SIMVERBS_ALONE,
    bool LEMMA_TIME ) {

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

		int result = breakup(block_pointer, block_size, m_file, region );
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
              for (int j = 0; j < lines.size(); ++j) {
                vector<string> target_tokens = s9::SplitStringWhitespace(lines[j]);
                
                if (target_tokens.size() > 5) {
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
                        // Stop here, we've found the direct verb
                        target = 0;
                        break;
                      }             
                    }
                  }
                }
              }

              if (target == start){
                target = 0;
                break;
              } 
            }
          }

          str_buffer = "";

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


