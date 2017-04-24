/**
* @brief Functions that work on the subjects and objects of verbs
* @file wacky_sbj_obj.cc
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#include "wacky_sbj_obj_mkl.hpp"

using namespace std;

/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SBJ_OBJ the vector of vectors of subjects and objects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a vector of verb x verb 
 * @param sum_subject a vector of verb subjects summed
 * @param sum_object a vector of the verb objects summed
 * @param sum_krn a vector of the verb subs objs kroneckered
 */

void read_subjects_objects(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SBJ_OBJ,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    vector<float> & base_vector,
    vector<float> & sum_subject,
    vector<float> & sum_object,
    vector<float> & sum_krn) {

  MKL_INT nsize = BASIS_SIZE;
  MKL_INT ksize = BASIS_SIZE * BASIS_SIZE;

  int vidx = DICTIONARY_FAST[verb];
  vector<int> subs_obs = VERB_SBJ_OBJ[vidx];

  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector[i] = WORD_VECTORS[vidx][i];
  } 
 
  for (int i=0; i < BASIS_SIZE; ++i){
    sum_subject[i] = 0.0f;
    sum_object[i] = 0.0f;
    
    for (int j=0; j < BASIS_SIZE; ++j){
      sum_krn[(BASIS_SIZE * i)+j] = 1.0f;
    }
  }

  for (int i =0; i < subs_obs.size(); i+=2) {
    vector<float> sbj_vector (BASIS_SIZE);
    vector<float> obj_vector (BASIS_SIZE);

    for (int j =0; j < BASIS_SIZE; ++j) {
      sbj_vector[j] = WORD_VECTORS[ subs_obs[i] ][j];
      obj_vector[j] = WORD_VECTORS[ subs_obs[i+1] ][j];
    }
 
    // TODO this var should not be redeclared all the time ><
    vector<float> tk ( BASIS_SIZE * BASIS_SIZE);
    krn_mul(sbj_vector, obj_vector, tk);
  
    float progress = float(i)/float(subs_obs.size()) * 100.0;
    int thread_num = omp_get_thread_num();
    printf("\033[%d;50H%d-Progress:%f",thread_num+1,thread_num, progress); 
    fflush(stdout);
    vsAdd(ksize, &sum_krn[0], &tk[0], &sum_krn[0]);
    vsAdd(nsize, &sum_subject[0], &sbj_vector[0], &sum_subject[0]);
    vsAdd(nsize, &sum_object[0], &obj_vector[0], &sum_object[0]);
  
  }

}

/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SBJ_OBJ the vector of vectors of subjects and objects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a vector of verb x verb 
 * @param sum_subject a vector of verb subjects summed
 * @param sum_krn a vector of the verb subs objs kroneckered
 */

void read_subjects_objects_few(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SBJ_OBJ,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    vector<float> & base_vector,
    vector<float> & sum_subject,
    vector<float> & sum_krn) {

  
  std::stringstream status;
  MKL_INT nsize = BASIS_SIZE;
  MKL_INT ksize = BASIS_SIZE * BASIS_SIZE;

  int vidx = DICTIONARY_FAST[verb];
  vector<int> subs_obs = VERB_SBJ_OBJ[vidx];

  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector[i] = WORD_VECTORS[vidx][i];
  } 
 
  for (int i=0; i < BASIS_SIZE; ++i){
    sum_subject[i] = 0.0f;
    
    for (int j=0; j < BASIS_SIZE; ++j){
      sum_krn[(BASIS_SIZE * i)+j] = 1.0f;
    }
  }
  
  for (int i =0; i < subs_obs.size(); i+=2) {
    vector<float> sbj_vector (BASIS_SIZE);
    vector<float> obj_vector (BASIS_SIZE);

    for (int j =0; j < BASIS_SIZE; ++j) {
      sbj_vector[j] = WORD_VECTORS[ subs_obs[i] ][j];
      obj_vector[j] = WORD_VECTORS[ subs_obs[i+1] ][j];
    }
 
    float progress = float(i)/float(subs_obs.size()) * 100.0;
    int thread_num = omp_get_thread_num();
    
    printf("\033[%d;50H%d-Progress:%f",thread_num+1,thread_num, progress); 
    fflush(stdout);
    // TODO dont keep redeclaring tk
    vector<float> ts (BASIS_SIZE);
    vector<float> tk (BASIS_SIZE * BASIS_SIZE);
    krn_mul(sbj_vector, obj_vector, tk);

    vsAdd(ksize, &sum_krn[0], &tk[0], &sum_krn[0]);
    vsAdd(nsize, &sbj_vector[0], &obj_vector[0], &ts[0]); 
    vsAdd(nsize, &sum_subject[0], &ts[0], &sum_subject[0]);
  }
}



/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SUBJECTS the vector of vectors of verb subjects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a vector of verb x verb 
 * @param add_vector a vector of subjects added
 * @param min_vector a vector of minimums
 * @param max_vector a vector of maximums 
 * @param krn_vector a vector of verb (x) verb
 */


void read_subjects(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SUBJECTS,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    vector<float> & base_vector,
    vector<float> & add_vector,
    vector<float> & min_vector,
    vector<float> & max_vector,
    vector<float> & krn_vector) {

  std::stringstream status;
  
  MKL_INT nsize = BASIS_SIZE;
  MKL_INT ksize = BASIS_SIZE * BASIS_SIZE;

  int vidx = DICTIONARY_FAST[verb];
  vector<int> subjects = VERB_SUBJECTS[vidx];

  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector[i] = WORD_VECTORS[vidx][i];
  } 
 
  for (int i=0; i < BASIS_SIZE; ++i){
    add_vector[i] = 0.0f;
    min_vector[i] = 10000000.0f; // TODO - replace with EPSILON
    max_vector[i] = -100000000.0f;
    
    for (int j=0; j < BASIS_SIZE; ++j){
      krn_vector[(BASIS_SIZE * i) + j] = 0.0f;
    }
  }

  int prg = 0;
  for (int i : subjects) {
    vector<float> sbj_vector (BASIS_SIZE);
  
    for (int j =0; j < BASIS_SIZE; ++j) {
      sbj_vector[j] = WORD_VECTORS[i][j];
    }
   
    float progress = float(prg)/float(subjects.size()) * 100.0;
    int thread_num = omp_get_thread_num();
    printf("\033[%d;50H%d-Progress:%f",thread_num+1,thread_num, progress); 
    fflush(stdout);
    vsAdd(nsize, &add_vector[0], &sbj_vector[0], &add_vector[0]);

    vector<float> tk (BASIS_SIZE * BASIS_SIZE);
    krn_mul(sbj_vector, sbj_vector, tk);
    vsAdd(ksize, &krn_vector[0], &tk[0], &krn_vector[0]); 

    // Min and max vectors
    for (int j =0; j < BASIS_SIZE; ++j){
      if (min_vector[j] > sbj_vector[j]){
        min_vector[j] = sbj_vector[j];
      } else if (max_vector[j] < sbj_vector[j]){
        max_vector[j] = sbj_vector[j];
      }
    }

    prg++; 
  }
}

/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SUBJECTS the vector of vectors of verb subjects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a vector of verb x verb 
 * @param add_vector a vector of subjects added
 * @param krn_vector a vector of verb (x) verb
 */

void read_subjects_few(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SUBJECTS,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    vector<float> & base_vector,
    vector<float> & add_vector,
    vector<float> & krn_vector) {
 
  std::stringstream status;
  
  MKL_INT nsize = BASIS_SIZE;
  MKL_INT ksize = BASIS_SIZE * BASIS_SIZE;

  int vidx = DICTIONARY_FAST[verb];
  vector<int> subjects = VERB_SUBJECTS[vidx];

  printf("\033[subjects size:%d",VERB_SUBJECTS.size()); 
  fflush(stdout);
   
  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector[i] = WORD_VECTORS[vidx][i];
  } 
 
  for (int i=0; i < BASIS_SIZE; ++i){
    add_vector[i] = 0.0f;
    
    for (int j=0; j < BASIS_SIZE; ++j){
      krn_vector[(BASIS_SIZE * i) + j] = 0.0f;
    }
  }

  vector<float> tk (BASIS_SIZE * BASIS_SIZE);
  int prg = 0;
  for (int i : subjects) {
    vector<float> sbj_vector (WORD_VECTORS[i]); 
    float progress = float(prg)/float(subjects.size()) * 100.0;
    int thread_num = omp_get_thread_num();
    printf("\033[%d;50H%d-Progress:%f",thread_num+1, thread_num, progress); 
    fflush(stdout);
    vsAdd(nsize, &add_vector[0], &sbj_vector[0], &add_vector[0]);
    
    krn_mul(sbj_vector, sbj_vector, tk);
   
    vsAdd(ksize, &krn_vector[0], &tk[0], &krn_vector[0]); 
    prg++;
  }
}

/**
 * Return all the stats for our intransitive verb pairs
 * @param VERBS_TO_CHECK a vector of VerbPair
 * @param VERB_TRANSITIVE the list of transitive verbs
 * @param VERB_INTRANSITIVE the list of intransitive verbs
 * @param BASIS_SIZE the size of our word vectors
 * @param DICTIONARY_FAST the fast dictionary 
 * @param VERB_SUBJECTS the vector of vectors of verb subjects
 * @param WORD_VECTORS our word count vectors
 */


void intrans_count(  std::string results_file,
  std::vector<VerbPair> & VERBS_TO_CHECK,
  set<string> & VERB_TRANSITIVE,
  set<string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  map<string,int> & DICTIONARY_FAST,
  vector< vector<int> > & VERB_SUBJECTS,
  vector< vector<float> > & WORD_VECTORS) {
  
  /*int num_blocks = 1;

  #pragma omp parallel
  {
    num_blocks = omp_get_num_threads();
  }

  int block_size = VERBS_TO_CHECK.size() / num_blocks;
  */
  int total_verbs = 0;

  // Open the file to write results
  std::ofstream out_file (results_file);
  if (!out_file.is_open()) {
    cout << "Unable to open " << results_file << " for writing" << endl;
    return;
  }
  
  out_file << "verb0,verb1,base_sim,add_sim,min_sim,max_sim,add_add_sim,add_mul_sim,min_add_sim,min_mul_sim,max_add_sim,max_mul_sim,krn_sim,krn_add_sim,krn_mul_sim,human_sim" << endl;

  #pragma omp parallel
  {   
    /*int block_id = omp_get_thread_num();
    int start = block_size * block_id;
    int end = block_size * (block_id + 1);
    if (block_id + 1 == num_blocks){
      end = VERBS_TO_CHECK.size();
    }*/

    std::string status;

    #pragma omp for
    for (int i=0; i < VERBS_TO_CHECK.size(); ++i){

      VerbPair vp = VERBS_TO_CHECK[i];

      status = "Verbs: " + vp.v0 + ", " + vp.v1 + "                             " + s9::ToString(i) + "of" + s9::ToString(VERBS_TO_CHECK.size()) + "  ";
  
      int thread_num = omp_get_thread_num();
      printf("\033[%d;0H%d-%s",thread_num+1, thread_num, status.c_str()); 
      fflush(stdout);
     
      if(VERB_INTRANSITIVE.find(vp.v0) != VERB_INTRANSITIVE.end() &&
          VERB_INTRANSITIVE.find(vp.v1) != VERB_INTRANSITIVE.end()){

        vector<float> base_vector0 (BASIS_SIZE,0);
        vector<float> add_vector0 (BASIS_SIZE,0);
        vector<float> min_vector0 (BASIS_SIZE,0);
        vector<float> max_vector0 (BASIS_SIZE,0);
        vector<float> add_base_add_vector0 (BASIS_SIZE,0);
        vector<float> add_base_mul_vector0 (BASIS_SIZE,0);
        vector<float> min_base_add_vector0 (BASIS_SIZE,0);
        vector<float> min_base_mul_vector0 (BASIS_SIZE,0);
        vector<float> max_base_add_vector0 (BASIS_SIZE,0);
        vector<float> max_base_mul_vector0 (BASIS_SIZE,0);
        vector<float> krn_vector0 (BASIS_SIZE * BASIS_SIZE,0);
        vector<float> krn_base_add_vector0 (BASIS_SIZE * BASIS_SIZE,0);
        vector<float> krn_base_mul_vector0 (BASIS_SIZE * BASIS_SIZE,0);
    
        vector<float> base_vector1 (BASIS_SIZE,0);
        vector<float> add_vector1 (BASIS_SIZE,0);
        vector<float> min_vector1 (BASIS_SIZE,0);
        vector<float> max_vector1 (BASIS_SIZE,0);
        vector<float> add_base_add_vector1 (BASIS_SIZE,0);
        vector<float> add_base_mul_vector1 (BASIS_SIZE,0);
        vector<float> min_base_add_vector1 (BASIS_SIZE,0);
        vector<float> min_base_mul_vector1 (BASIS_SIZE,0);
        vector<float> max_base_add_vector1 (BASIS_SIZE,0);
        vector<float> max_base_mul_vector1 (BASIS_SIZE,0);
        vector<float> krn_vector1 (BASIS_SIZE * BASIS_SIZE,0);
        vector<float> krn_base_add_vector1 (BASIS_SIZE * BASIS_SIZE,0);
        vector<float> krn_base_mul_vector1 (BASIS_SIZE * BASIS_SIZE,0);

        read_subjects(vp.v0,
            DICTIONARY_FAST,
            VERB_SUBJECTS,
            WORD_VECTORS,
            BASIS_SIZE,
            base_vector0,
            add_vector0,
            min_vector0,
            max_vector0,
            krn_vector0);

        read_subjects(vp.v1,
            DICTIONARY_FAST,
            VERB_SUBJECTS,
            WORD_VECTORS,
            BASIS_SIZE, 
            base_vector1,
            add_vector1,
            min_vector1,
            max_vector1,
            krn_vector1);

        // Now we can perform the last step in our equation
  
        vsAdd(BASIS_SIZE, &add_vector0[0], &base_vector0[0], &add_base_add_vector0[0]);
        vsMul(BASIS_SIZE, &add_vector0[0], &base_vector0[0], &add_base_mul_vector0[0]);
        
        vsAdd(BASIS_SIZE, &min_vector0[0], &base_vector0[0], &min_base_add_vector0[0]);
        vsMul(BASIS_SIZE, &min_vector0[0], &base_vector0[0], &min_base_mul_vector0[0]);
        
        vsAdd(BASIS_SIZE, &max_vector0[0], &base_vector0[0], &max_base_add_vector0[0]);
        
        vsMul(BASIS_SIZE, &max_vector0[0], &base_vector0[0], &max_base_mul_vector0[0]);
        vector<float> td (BASIS_SIZE * BASIS_SIZE);
        krn_mul(base_vector0, base_vector0, td);
        
        vsAdd(BASIS_SIZE * BASIS_SIZE, &krn_vector0[0], &td[0], &krn_base_add_vector0[0]);
        
        vsMul(BASIS_SIZE * BASIS_SIZE, &krn_vector0[0], &td[0], &krn_base_mul_vector0[0]);
        vsAdd(BASIS_SIZE, &add_vector1[0], &base_vector1[0], &add_base_add_vector1[0]);
          
        vsAdd(BASIS_SIZE, &min_vector1[0], &base_vector1[0], &min_base_add_vector1[0]);
        
        vsMul(BASIS_SIZE, &min_vector1[0], &base_vector1[0], &min_base_mul_vector1[0]);
        vsAdd(BASIS_SIZE, &max_vector1[0], &base_vector1[0], &max_base_add_vector1[0]);
        
        vsMul(BASIS_SIZE * BASIS_SIZE, &max_vector1[0], &base_vector1[0], &max_base_mul_vector1[0]);
        krn_mul(base_vector1, base_vector1, td);
        vsAdd(BASIS_SIZE * BASIS_SIZE, &krn_vector1[0], &td[0], &krn_base_add_vector1[0]);
        vsMul(BASIS_SIZE * BASIS_SIZE, &krn_vector1[0], &td[0], &krn_base_mul_vector1[0]);

        float c0 = cosine_sim(base_vector0, base_vector1, BASIS_SIZE);
        float c1 = cosine_sim(add_vector0, add_vector1, BASIS_SIZE);
        float c2 = cosine_sim(min_vector0, min_vector1, BASIS_SIZE);
        float c3 = cosine_sim(max_vector0, max_vector1, BASIS_SIZE);
        float c4 = cosine_sim(add_base_add_vector0, add_base_add_vector1, BASIS_SIZE);
        float c5 = cosine_sim(add_base_mul_vector0, add_base_mul_vector1, BASIS_SIZE);
        float c6 = cosine_sim(min_base_add_vector0, min_base_add_vector1, BASIS_SIZE);
        float c7 = cosine_sim(min_base_mul_vector0, min_base_mul_vector1, BASIS_SIZE);
        float c8 = cosine_sim(max_base_add_vector0, max_base_add_vector1, BASIS_SIZE);
        float c9 = cosine_sim(max_base_mul_vector0, max_base_mul_vector1, BASIS_SIZE);
        float c10 = cosine_sim(krn_vector0, krn_vector1, BASIS_SIZE * BASIS_SIZE);
        float c11 = cosine_sim(krn_base_add_vector0, krn_base_add_vector1, BASIS_SIZE * BASIS_SIZE);
        float c12 = cosine_sim(krn_base_mul_vector0, krn_base_mul_vector1, BASIS_SIZE * BASIS_SIZE);

        std::stringstream stream;       
        stream << vp.v0 << "," << vp.v1 << "," << s9::ToString(c0)
          << "," << s9::ToString(c1)
          << "," << s9::ToString(c2)
          << "," << s9::ToString(c3)
          << "," << s9::ToString(c4)
          << "," << s9::ToString(c5)
          << "," << s9::ToString(c6)
          << "," << s9::ToString(c7)
          << "," << s9::ToString(c8)
          << "," << s9::ToString(c9)
          << "," << s9::ToString(c10)
          << "," << s9::ToString(c11)
          << "," << s9::ToString(c12)
          << "," << s9::ToString(vp.s)
          << endl;
        
        #pragma omp critical
        {
          out_file << stream.str();
          out_file.flush();
        }
      }
    }
  }

  out_file.close();
}


/**
 * Return all the stats for our transitive verb pairs
 * @param VERBS_TO_CHECK a vector of VerbPair
 * @param VERB_TRANSITIVE the list of transitive verbs
 * @param VERB_INTRANSITIVE the list of intransitive verbs
 * @param BASIS_SIZE the size of our word vectors
 * @param DICTIONARY_FAST the fast dictionary 
 * @param VERB_SBJ_OBJ the vector of vectors of verb subject-object pairs
 * @param WORD_VECTORS our word count vectors
 */


void trans_count(  std::string results_file,
  std::vector<VerbPair> & VERBS_TO_CHECK,
  set<string> & VERB_TRANSITIVE,
  set<string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  map<string,int> & DICTIONARY_FAST,
  vector< vector<int> > & VERB_SBJ_OBJ,
  vector< vector<float> > & WORD_VECTORS) {

  int total_verbs = 0;
  // Print out the total number we should expect
  for (int i=0; i < VERBS_TO_CHECK.size(); ++i){

    VerbPair vp = VERBS_TO_CHECK[i];
    if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
      VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){
      total_verbs ++;
    }  
  }

  cout << "Total Trans Count: " << total_verbs << endl;

  // Open the file to write results
  std::ofstream out_file (results_file);
  if (!out_file.is_open()) {
    cout << "Unable to open " << results_file << " for writing" << endl;
    return;
  }
  

  out_file << "verb0,verb1,base_sim,sbj_obj_sim,sbj_obj_add,sbj_obj_mul,sum_sbj_obj,sum_sbj_obj_mul,sum_sbj_obj_add,human_sim" << endl;

  /*int num_blocks = 1;

  #pragma omp parallel
  {
    num_blocks = omp_get_num_threads();
  }

  int block_size = VERBS_TO_CHECK.size() / num_blocks;*/



  #pragma omp parallel
  {   
    /*int block_id = omp_get_thread_num();
    int start = block_size * block_id;
    int end = block_size * (block_id + 1);

    if (block_id + 1 == num_blocks){
      end = VERBS_TO_CHECK.size();
    }*/

    std::string status;
    
    vector<float> base_vector0 (BASIS_SIZE);
    vector<float> sum_subject0 (BASIS_SIZE);
    vector<float> sum_object0 (BASIS_SIZE);
    vector<float> sum_krn0 (BASIS_SIZE * BASIS_SIZE);

    vector<float> base_vector1 (BASIS_SIZE);
    vector<float> sum_subject1 (BASIS_SIZE);
    vector<float> sum_object1 (BASIS_SIZE);
    vector<float> sum_krn1 (BASIS_SIZE * BASIS_SIZE);

    vector<float> krn_base0 (BASIS_SIZE * BASIS_SIZE);
    vector<float> krn_base1 (BASIS_SIZE * BASIS_SIZE);

    #pragma omp for
    for (int i=0; i < VERBS_TO_CHECK.size(); ++i){

      VerbPair vp = VERBS_TO_CHECK[i];

      if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
          VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){

        // Set the vectors to zeros
        sum_subject0.clear();  
        sum_subject1.clear();        
        sum_object0.clear();
        sum_object1.clear();
        sum_krn0.clear();
        sum_krn1.clear();

        status = "Verbs: " + vp.v0 + ", " + vp.v1 + "                             " + s9::ToString(i) + "of" + s9::ToString(VERBS_TO_CHECK.size()) + "  ";
 
        int thread_num = omp_get_thread_num();
        printf("\033[%d;0H%d-%s",thread_num+1, thread_num, status.c_str()); 
        fflush(stdout);
     
        read_subjects_objects(vp.v0,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_object0, sum_krn0);
        read_subjects_objects(vp.v1,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_object1, sum_krn1);
      
        krn_mul(base_vector0, base_vector0, krn_base0);
        krn_mul(base_vector1, base_vector1, krn_base1);
        
        float c0 = cosine_sim(base_vector0, base_vector1, BASIS_SIZE);
        float c1 = cosine_sim(sum_krn0, sum_krn1, BASIS_SIZE * BASIS_SIZE);

        vector<float> tk0 (BASIS_SIZE * BASIS_SIZE);    
        vsAdd(BASIS_SIZE * BASIS_SIZE, &sum_krn0[0], &krn_base0[0], &tk0[0]);

        vector<float> tk1 (BASIS_SIZE * BASIS_SIZE);
        vsAdd(BASIS_SIZE * BASIS_SIZE, &sum_krn1[0], &krn_base1[0], &tk1[0]);
        float c2 = cosine_sim(tk0, tk1, BASIS_SIZE * BASIS_SIZE);

        vsMul(BASIS_SIZE * BASIS_SIZE, &sum_krn0[0], &krn_base0[0], &tk0[0]);
        vsMul(BASIS_SIZE * BASIS_SIZE, &sum_krn1[0], &krn_base1[0], &tk1[0]);
        float c3 = cosine_sim(tk0, tk1, BASIS_SIZE * BASIS_SIZE);

        vector<float> tm0 (BASIS_SIZE);
        vector<float> tm1 (BASIS_SIZE);

        vsAdd(BASIS_SIZE, &sum_subject0[0], &sum_subject0[0], &tm0[0]);
        vsAdd(BASIS_SIZE, &sum_subject1[0], &sum_object1[0], &tm1[0]);

        float c4 = cosine_sim(tm0, tm1, BASIS_SIZE);
        
        vsAdd(BASIS_SIZE, &sum_subject0[0], &sum_object0[0], &tm0[0]);
        vsMul(BASIS_SIZE, &tm0[0], &base_vector0[0], &tm0[0]);
        vsAdd(BASIS_SIZE, &sum_subject1[0], &sum_object1[0], &tm1[0]); 
        vsMul(BASIS_SIZE, &tm1[0], &base_vector1[0], &tm1[0]);

        float c5 = cosine_sim(tm0, tm1, BASIS_SIZE);
        
        vsAdd(BASIS_SIZE, &sum_subject0[0], &sum_object0[0], &tm0[0]);
        vsAdd(BASIS_SIZE, &base_vector0[0], &tm0[0], &tm0[0]);
        vsAdd(BASIS_SIZE, &sum_subject1[0], &sum_object1[0], &tm1[0]);
        vsAdd(BASIS_SIZE, &base_vector1[0], &tm1[0], &tm1[0]);
        float c6 = cosine_sim(tm0, tm1, BASIS_SIZE);
      
        std::stringstream stream;

        stream << vp.v0 << "," << vp.v1 << "," << s9::ToString(c0)
          << "," << s9::ToString(c1)
          << "," << s9::ToString(c2)
          << "," << s9::ToString(c3)
          << "," << s9::ToString(c4)
          << "," << s9::ToString(c5)
          << "," << s9::ToString(c6)
          << "," << s9::ToString(vp.s)
          << endl;

        #pragma omp critical
        {
          out_file << stream.str();
          out_file.flush();
        }
      }
    }
  }

  out_file.close();
}

/**
 * Return all the stats for all verb pairs
 * @param VERBS_TO_CHECK a vector of VerbPair
 * @param VERB_TRANSITIVE the list of transitive verbs
 * @param VERB_INTRANSITIVE the list of intransitive verbs
 * @param BASIS_SIZE the size of our word vectors
 * @param DICTIONARY_FAST the fast dictionary 
 * @param VERB_SBJ_OBJ the vector of vectors of verb subject-object pairs
 * @param VERB_SUBJECTS the vector of verb subjects
 * @param WORD_VECTORS our word count vectors
 */

void all_count( std::string results_file,
  std::vector<VerbPair> & VERBS_TO_CHECK,
  set<string> & VERB_TRANSITIVE,
  set<string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  map<string,int> & DICTIONARY_FAST,
  vector< vector<int> > & VERB_SBJ_OBJ,
  vector< vector<int> > & VERB_SUBJECTS,
  vector< vector<float> > & WORD_VECTORS) {

  int total_verbs = 0;
  // Print out the total number we should expect
  for (int i=0; i < VERBS_TO_CHECK.size(); ++i){

    VerbPair vp = VERBS_TO_CHECK[i];
    if(!(VERB_TRANSITIVE.find(vp.v0) == VERB_TRANSITIVE.end() ||
      VERB_TRANSITIVE.find(vp.v1) == VERB_TRANSITIVE.end() || 
      VERB_INTRANSITIVE.find(vp.v0) == VERB_INTRANSITIVE.end() ||
      VERB_TRANSITIVE.find(vp.v1) == VERB_INTRANSITIVE.end())) {
      //cout << vp.v0 << "," << vp.v1 << endl;
        total_verbs ++;
    }  
  }

  cout << "Total verb pairs: " << total_verbs << endl;

  MKL_INT nsize = BASIS_SIZE;
  MKL_INT ksize = BASIS_SIZE * BASIS_SIZE;

  
  // Open the file to write results
  std::ofstream out_file (results_file);
  if (!out_file.is_open()) {
    cout << "Unable to open " << results_file << " for writing" << endl;
    return;
  }
  
  out_file << "verb0,verb1,base_sim,cs1,cs2,cs3,cs4,cs5,cs6,human_sim" << endl;
  
  // TODO - Better to use a for loop so that fast threads can do work and not sit still
  #pragma omp parallel
  {   
    std::string status;

    vector<float> base_vector0 (BASIS_SIZE,0);
    vector<float> sum_subject0 (BASIS_SIZE,0);
    vector<float> sum_krn0 (BASIS_SIZE * BASIS_SIZE,0);

    vector<float> base_vector1 (BASIS_SIZE,0);
    vector<float> sum_subject1 (BASIS_SIZE,0);
    vector<float> sum_krn1 (BASIS_SIZE * BASIS_SIZE,0);

    vector<float> krn_base0 (BASIS_SIZE * BASIS_SIZE,0);
    vector<float> krn_base1 (BASIS_SIZE * BASIS_SIZE,0);

    vector<float> tv0 (BASIS_SIZE);
    vector<float> tv1 (BASIS_SIZE);
    vector<float> tk0 (BASIS_SIZE * BASIS_SIZE);
    vector<float> tk1 (BASIS_SIZE * BASIS_SIZE);
    
    #pragma omp for
    for (int i=0; i < VERBS_TO_CHECK.size(); ++i){

      VerbPair vp = VERBS_TO_CHECK[i];

      // Set the vectors to zeros
      std::fill(sum_subject0.begin(), sum_subject0.end(), 0);
      std::fill(sum_subject1.begin(), sum_subject1.end(), 0);
      std::fill(sum_krn0.begin(), sum_krn0.end(), 0);
      std::fill(sum_krn1.begin(), sum_krn1.end(), 0);
      std::fill(tv0.begin(), tv0.end(), 0);
      std::fill(tv1.begin(), tv1.end(), 0);     
      std::fill(tk0.begin(), tk0.end(), 0);
      std::fill(tk1.begin(), tk1.end(), 0);     
      std::fill(krn_base0.begin(), krn_base0.end(), 0);
      std::fill(krn_base1.begin(), krn_base1.end(), 0);     
 
     status = "Verbs: " + vp.v0 + ", " + vp.v1 + "                             " + s9::ToString(i) + "of" + s9::ToString(VERBS_TO_CHECK.size()) + "  ";
  
      int thread_num = omp_get_thread_num();
      printf("\033[%d;0H%d-%s",thread_num+1, thread_num, status.c_str()); 
      fflush(stdout);

    
      if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
          VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){

      	read_subjects_objects_few(vp.v0,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

        read_subjects_objects_few(vp.v1,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);

      } else if(VERB_TRANSITIVE.find(vp.v0) == VERB_TRANSITIVE.end() &&
          VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){
  
        read_subjects_few(vp.v0,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

        read_subjects_objects_few(vp.v1,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);
      } else if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
          VERB_TRANSITIVE.find(vp.v1) == VERB_TRANSITIVE.end()){
      
        read_subjects_objects_few(vp.v0,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

        read_subjects_few(vp.v1,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);
      } else {
      
        read_subjects_few(vp.v0,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);
        read_subjects_few(vp.v1,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);
      }
 
      krn_mul(base_vector0, base_vector0, krn_base0);
      krn_mul(base_vector1, base_vector1, krn_base1);
      
      float c0 = cosine_sim(base_vector0, base_vector1, BASIS_SIZE);
      float c1 = cosine_sim(sum_subject0, sum_subject1, BASIS_SIZE);
      
      vsAdd(nsize, &sum_subject0[0], &base_vector0[0], &tv0[0]);
      vsAdd(nsize, &sum_subject1[0], &base_vector1[0], &tv1[0]);

      float c2 = cosine_sim(tv0,tv1, BASIS_SIZE);
      vsMul(nsize, &sum_subject0[0], &base_vector0[0], &tv0[0]);
      vsMul(nsize, &sum_subject1[0], &base_vector1[0], &tv1[0]);
 
      float c3 = cosine_sim(tv0,tv1, BASIS_SIZE);
      float c4 = cosine_sim(sum_krn0, sum_krn1, BASIS_SIZE * BASIS_SIZE);
  
      vsAdd(ksize, &sum_krn0[0], &krn_base0[0], &tk0[0]);
      vsAdd(ksize, &sum_krn1[0], &krn_base1[0], &tk1[0]);
      float c5 = cosine_sim(tk0, tk1, BASIS_SIZE * BASIS_SIZE);

      vsMul(ksize, &sum_krn0[0], &krn_base0[0], &tk0[0]);
      vsMul(ksize, &sum_krn1[0], &krn_base1[0], &tk1[0]);
      float c6 = cosine_sim(tk0, tk1, BASIS_SIZE * BASIS_SIZE);

      std::stringstream stream;
    
      stream << vp.v0 << "," << vp.v1 << "," << s9::ToString(c0)
        << "," << s9::ToString(c1)
        << "," << s9::ToString(c2)
        << "," << s9::ToString(c3)
        << "," << s9::ToString(c4)
        << "," << s9::ToString(c5)
        << "," << s9::ToString(c6)
        << "," << s9::ToString(vp.s)
        << endl;

      #pragma omp critical
      {
        out_file << stream.str();
        out_file.flush();
      }
    }    
  }

  out_file.close();
}

/**
 * Return the variance of the verbs to be checked
 * @param VERBS_TO_CHECK a vector of VerbPair
 * @param BASIS_SIZE the size of our word vectors
 * @param DICTIONARY_FAST the fast dictionary 
 * @param VERB_SBJ_OBJ the vector of vectors of verb subject-object pairs
 * @param WORD_VECTORS our word count vectors
 */

void variance_count( std::string results_file,
  std::vector<VerbPair> & VERBS_TO_CHECK,
  int BASIS_SIZE,
  map<string,int> & DICTIONARY_FAST,
  vector< vector<int> > & VERB_SBJ_OBJ,
  vector< vector<float> > & WORD_VECTORS) {

  // Get all the unique verbs in the set to check
  set<string> verbs_to_check_set;
  vector<string> verbs_to_check;
  
  for (VerbPair vp : VERBS_TO_CHECK){
    verbs_to_check_set.insert(vp.v0);
    verbs_to_check_set.insert(vp.v1);
  }

  std::copy(verbs_to_check_set.begin(), verbs_to_check_set.end(), std::back_inserter(verbs_to_check));

  // Open the file to write results
  std::ofstream out_file (results_file);
  if (!out_file.is_open()) {
    cout << "Unable to open " << results_file << " for writing" << endl;
    return;
  }
  
  out_file << "verb,variance" << endl;
  
  // TODO - Better to use a for loop so that fast threads can do work and not sit still
  #pragma omp parallel
  {   
    std::string status;

    #pragma omp for
    for (int i=0; i < verbs_to_check.size(); ++i){
    
      string verb = verbs_to_check[i];

      status = "Verb: " + verb + "                                 " + s9::ToString(i) + "of" + s9::ToString(VERBS_TO_CHECK.size()) + "  ";
  
      int thread_num = omp_get_thread_num();
      printf("\033[%d;0H%d-%s",thread_num+1, thread_num, status.c_str()); 
      fflush(stdout);
    
      int vidx = DICTIONARY_FAST[verb];
      vector<int> subobs = VERB_SBJ_OBJ[vidx];
      vector<float> distances;

      for (int j=0; j < subobs.size()-1; ++j){
          
        int sidx = subobs[j];
        vector<float> wvj = WORD_VECTORS[sidx];

        for (int k=j+1; k < subobs.size(); ++k){

          int tidx = subobs[k];
          vector<float> wvk = WORD_VECTORS[tidx];

          // Now compute the distance

          float dd = 0;
          for (int m = 0; m < BASIS_SIZE; ++m){
            float tf = static_cast<float>(wvj[m] - wvk[m]);
            dd += (tf*tf);
          }

          float distance = sqrt(dd);
          
          distances.push_back(distance);
        }
      }


      float variance = 0;
      float mean = 0;

     for (float tf : distances){
        mean += tf;
      }

      mean = mean / static_cast<float>(distances.size());

      for (float tf : distances){
        float tt = tf - mean;
        variance += (tt * tt); 
      }
      
      variance *= 1.0/static_cast<float>(distances.size());

      std::stringstream stream;
    
      stream << verb << "," << s9::ToString(variance) << endl;

      #pragma omp critical
      {
        out_file << stream.str();
        out_file.flush();
      }
    }    

  }

  out_file.close();
}
