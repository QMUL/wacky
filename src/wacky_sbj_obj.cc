/**
* @brief Functions that work on the subjects and objects of verbs
* @file wacky_sbj_obj.cc
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 01/02/2017
*
*/

#include "wacky_sbj_obj.hpp"

using namespace boost::numeric;
using namespace std;

/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SBJ_OBJ the vector of vectors of subjects and objects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a ublas vector of verb x verb 
 * @param sum_subject a ublas vector of verb subjects summed
 * @param sum_object a ublas vector of the verb objects summed
 * @param sum_krn a ublas vector of the verb subs objs kroneckered
 */

void read_subjects_objects(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SBJ_OBJ,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    ublas::vector<float> & base_vector,
    ublas::vector<float> & sum_subject,
    ublas::vector<float> & sum_object,
    ublas::vector<float> & sum_krn) {

  int vidx = DICTIONARY_FAST[verb];
  vector<int> subs_obs = VERB_SBJ_OBJ[vidx];

  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector(i) = WORD_VECTORS[vidx][i];
  } 
 
  for (int i=0; i < BASIS_SIZE; ++i){
    sum_subject(i) = 0.0f;
    sum_object(i) = 0.0f;
    
    for (int j=0; j < BASIS_SIZE; ++j){
      sum_krn((BASIS_SIZE * i)+j) = 1.0f;
    }
  }

  for (int i =0; i < subs_obs.size(); i+=2) {
    ublas::vector<float> sbj_vector (BASIS_SIZE);
    ublas::vector<float> obj_vector (BASIS_SIZE);

    for (int j =0; j < BASIS_SIZE; ++j) {
      sbj_vector(j) = WORD_VECTORS[ subs_obs[i] ][j];
      obj_vector(j) = WORD_VECTORS[ subs_obs[i+1] ][j];
    }
 
    ublas::vector<float> tk = krn_mul(sbj_vector, obj_vector);
      
    sum_krn = sum_krn + tk;
    
    sum_subject = sum_subject + sbj_vector;
    sum_object = sum_object + obj_vector;
  
  }

}


/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SUBJECTS the vector of vectors of verb subjects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a ublas vector of verb x verb 
 * @param add_vector a ublas vector of subjects added
 * @param min_vector a ublas vector of minimums
 * @param max_vector a ublas vector of maximums 
 * @param add_base_add_vector a ublas vector of summed vector + base
 * @param add_base_mul_vector a ublas vector of summed vector * base
 * @param min_base_add_vector a ublas vector of minimums + base
 * @param min_base_mul_vector a ublas vector of minimums * base
 * @param max_base_add_vector a ublas vector of maximums + base
 * @param max_base_mul_vector a ublas vector of maximums * base
 * @param krn_vector a ublas vector of verb (x) verb
 * @param krn_base_add_vector a ublas vector of summed subject (x) subject + base
 * @param krn_base_mul_vector a ublas vector of summed subject (x) subject * base
 */


void read_subjects(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SUBJECTS,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    ublas::vector<float> & base_vector,
    ublas::vector<float> & add_vector,
    ublas::vector<float> & min_vector,
    ublas::vector<float> & max_vector,
    ublas::vector<float> & add_base_add_vector,
    ublas::vector<float> & add_base_mul_vector,
    ublas::vector<float> & min_base_add_vector,
    ublas::vector<float> & min_base_mul_vector,
    ublas::vector<float> & max_base_add_vector,
    ublas::vector<float> & max_base_mul_vector,
    ublas::vector<float> & krn_vector,
    ublas::vector<float> & krn_base_add_vector,
    ublas::vector<float> & krn_base_mul_vector )  {
  
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

  for (int i : subjects) {
    ublas::vector<float> sbj_vector (BASIS_SIZE);
  
    for (int j =0; j < BASIS_SIZE; ++j) {
      sbj_vector[j] = WORD_VECTORS[i][j];
    }

    add_vector = add_vector + sbj_vector;
    ublas::vector<float> tk  = krn_mul(sbj_vector, sbj_vector);
      
    krn_vector = krn_vector + tk;

    // Min and max vectors
    for (int j =0; j < BASIS_SIZE; ++j){
      if (min_vector[j] > sbj_vector[j]){
        min_vector[j] = sbj_vector[j];
      } else if (max_vector[j] < sbj_vector[j]){
        max_vector[j] = sbj_vector[j];
      }
    } 
  }

  // Now we can perform the last step in our equation
  
  add_base_add_vector = add_vector + base_vector;
  add_base_mul_vector = mul_vec(add_vector,base_vector);
  min_base_add_vector = min_vector + base_vector;
  min_base_mul_vector = mul_vec(min_vector, base_vector);
  max_base_add_vector = max_vector + base_vector;
  max_base_mul_vector = mul_vec(max_vector, base_vector);
  ublas::vector<float> td = krn_mul(base_vector, base_vector);
  krn_base_add_vector = krn_vector + td;
  krn_base_mul_vector = mul_vec(krn_vector,td);

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

void intrans_count( std::vector<VerbPair> & VERBS_TO_CHECK,
  set<string> & VERB_TRANSITIVE,
  set<string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  map<string,int> & DICTIONARY_FAST,
  vector< vector<int> > & VERB_SUBJECTS,
  vector< vector<float> > & WORD_VECTORS) {
  
	int num_blocks = 1;

	#pragma omp parallel
	{
		num_blocks = omp_get_num_threads();
	}

	int block_size = VERBS_TO_CHECK.size() / num_blocks;

	int total_verbs = 0;
	// Print out the total number we should expect
	for (int i=0; i < VERBS_TO_CHECK.size(); ++i){

			VerbPair vp = VERBS_TO_CHECK[i];
			if(VERB_INTRANSITIVE.find(vp.v0) != VERB_INTRANSITIVE.end() &&
					VERB_INTRANSITIVE.find(vp.v1) != VERB_INTRANSITIVE.end()){
				cout << vp.v0 << "," << vp.v1 << endl;
				total_verbs ++;
			}
	
	}
	cout << "Total verbs: " << s9::ToString(total_verbs) << endl;
	cout << "Block Size: " << block_size << ", num_blocks: " << num_blocks << endl;


  cout << "verb0,verb1,base_sim,add_sim,min_sim,max_sim,add_add_sim,add_mul_sim,min_add_sim,min_mul_sim,     max_add_sim,max_mul_sim,krn_sim,krn_add_sim,krn_mul_sim,human_sim" << endl;

	#pragma omp parallel
	{   
		int block_id = omp_get_thread_num();
		int start = block_size * block_id;
		int end = (block_size + 1) * block_id;
		if (block_id + 1 == num_blocks){
			end = VERBS_TO_CHECK.size();
		}

		for (int i=start; i < end; ++i){

			VerbPair vp = VERBS_TO_CHECK[i];

			if(VERB_INTRANSITIVE.find(vp.v0) != VERB_INTRANSITIVE.end() &&
					VERB_INTRANSITIVE.find(vp.v1) != VERB_INTRANSITIVE.end()){

				ublas::vector<float> base_vector0 (BASIS_SIZE);
				ublas::vector<float> add_vector0 (BASIS_SIZE);
				ublas::vector<float> min_vector0 (BASIS_SIZE);
				ublas::vector<float> max_vector0 (BASIS_SIZE);
				ublas::vector<float> add_base_add_vector0 (BASIS_SIZE);
				ublas::vector<float> add_base_mul_vector0 (BASIS_SIZE);
				ublas::vector<float> min_base_add_vector0 (BASIS_SIZE);
				ublas::vector<float> min_base_mul_vector0 (BASIS_SIZE);
				ublas::vector<float> max_base_add_vector0 (BASIS_SIZE);
				ublas::vector<float> max_base_mul_vector0 (BASIS_SIZE);
				ublas::vector<float> krn_vector0 (BASIS_SIZE * BASIS_SIZE);
				ublas::vector<float> krn_base_add_vector0 (BASIS_SIZE * BASIS_SIZE);
				ublas::vector<float> krn_base_mul_vector0 (BASIS_SIZE * BASIS_SIZE);
		
				ublas::vector<float> base_vector1 (BASIS_SIZE);
				ublas::vector<float> add_vector1 (BASIS_SIZE);
				ublas::vector<float> min_vector1 (BASIS_SIZE);
				ublas::vector<float> max_vector1 (BASIS_SIZE);
				ublas::vector<float> add_base_add_vector1 (BASIS_SIZE);
				ublas::vector<float> add_base_mul_vector1 (BASIS_SIZE);
				ublas::vector<float> min_base_add_vector1 (BASIS_SIZE);
				ublas::vector<float> min_base_mul_vector1 (BASIS_SIZE);
				ublas::vector<float> max_base_add_vector1 (BASIS_SIZE);
				ublas::vector<float> max_base_mul_vector1 (BASIS_SIZE);
				ublas::vector<float> krn_vector1 (BASIS_SIZE * BASIS_SIZE);
				ublas::vector<float> krn_base_add_vector1 (BASIS_SIZE * BASIS_SIZE);
				ublas::vector<float> krn_base_mul_vector1 (BASIS_SIZE * BASIS_SIZE);

				read_subjects(vp.v0,
						DICTIONARY_FAST,
						VERB_SUBJECTS,
						WORD_VECTORS,
						BASIS_SIZE,
						base_vector0,
						add_vector0,
						min_vector0,
						max_vector0,
						add_base_add_vector0,
						add_base_mul_vector0,
						min_base_add_vector0,
						min_base_mul_vector0,
						max_base_add_vector0,
						max_base_mul_vector0,
						krn_vector0,
						krn_base_add_vector0,
						krn_base_mul_vector0);

				read_subjects(vp.v1,
						DICTIONARY_FAST,
						VERB_SUBJECTS,
						WORD_VECTORS,
						BASIS_SIZE, 
						base_vector1,
						add_vector1,
						min_vector1,
						max_vector1,
						add_base_add_vector1,
						add_base_mul_vector1,
						min_base_add_vector1,
						min_base_mul_vector1,
						max_base_add_vector1,
						max_base_mul_vector1,
						krn_vector1,
						krn_base_add_vector1,
						krn_base_mul_vector1);

				float c0 = cosine_sim(base_vector0, base_vector1);
				float c1 = cosine_sim(add_vector0, add_vector1);
				float c2 = cosine_sim(min_vector0, min_vector1);
				float c3 = cosine_sim(max_vector0, max_vector1);
				float c4 = cosine_sim(add_base_add_vector0, add_base_add_vector1);
				float c5 = cosine_sim(add_base_mul_vector0, add_base_mul_vector1);
				float c6 = cosine_sim(min_base_add_vector0, min_base_add_vector1);
				float c7 = cosine_sim(min_base_mul_vector0, min_base_mul_vector1);
				float c8 = cosine_sim(max_base_add_vector0, max_base_add_vector1);
				float c9 = cosine_sim(max_base_mul_vector0, max_base_mul_vector1);
				float c10 = cosine_sim(krn_vector0, krn_vector1);
				float c11 = cosine_sim(krn_base_add_vector0, krn_base_add_vector1);
				float c12 = cosine_sim(krn_base_mul_vector0, krn_base_mul_vector1);


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
				std::cout << stream.str();
					
			}
		}
	}
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

void trans_count( std::vector<VerbPair> & VERBS_TO_CHECK,
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
			//cout << vp.v0 << "," << vp.v1 << endl;
			total_verbs ++;
		}	
	}

	cout << "Total verbs: " << s9::ToString(total_verbs) << endl; 
  cout << "verb0,verb1,base_sim,sbj_obj_sim,sbj_obj_add,sbj_obj_mul,sum_sbj_obj,sum_sbj_obj_mul,sum_sbj_obj_add,human_sim" << endl;

	int num_blocks = 1;

	#pragma omp parallel
	{
		num_blocks = omp_get_num_threads();
	}

	int block_size = VERBS_TO_CHECK.size() / num_blocks;

	#pragma omp parallel
	{   
		int block_id = omp_get_thread_num();
		int start = block_size * block_id;
		int end = block_size * (block_id + 1);

		if (block_id + 1 == num_blocks){
			end = VERBS_TO_CHECK.size();
		}

		ublas::vector<float> base_vector0 (BASIS_SIZE);
		ublas::vector<float> sum_subject0 (BASIS_SIZE);
		ublas::vector<float> sum_object0 (BASIS_SIZE);
		ublas::vector<float> sum_krn0 (BASIS_SIZE * BASIS_SIZE);

		ublas::vector<float> base_vector1 (BASIS_SIZE);
		ublas::vector<float> sum_subject1 (BASIS_SIZE);
		ublas::vector<float> sum_object1 (BASIS_SIZE);
		ublas::vector<float> sum_krn1 (BASIS_SIZE * BASIS_SIZE);

		ublas::vector<float> krn_base0 (BASIS_SIZE * BASIS_SIZE);
		ublas::vector<float> krn_base1 (BASIS_SIZE * BASIS_SIZE);

		for (int i=start; i < end; ++i){

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


				read_subjects_objects(vp.v0,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_object0, sum_krn0);

				read_subjects_objects(vp.v1,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_object1, sum_krn1);

			
				krn_base0 = krn_mul(base_vector0, base_vector0);
				krn_base1 = krn_mul(base_vector1, base_vector1);
				
				float c0 = cosine_sim(base_vector0, base_vector1);
				float c1 = cosine_sim(sum_krn0, sum_krn1);

				ublas::vector<float> tk0 (BASIS_SIZE * BASIS_SIZE);
				tk0 = sum_krn0 + krn_base0;

				ublas::vector<float> tk1 (BASIS_SIZE * BASIS_SIZE);
				tk1 = sum_krn1 + krn_base1;
				float c2 = cosine_sim(tk0, tk1);

				tk0 = mul_vec(sum_krn0, krn_base0);
				tk1 = mul_vec(sum_krn1,krn_base1);
				float c3 = cosine_sim(tk0, tk1);

				ublas::vector<float> tm0 (BASIS_SIZE);
				ublas::vector<float> tm1 (BASIS_SIZE);

				tm0 = sum_subject0 + sum_object0;
				tm1 = sum_subject1 + sum_object1; 

				float c4 = cosine_sim(tm0, tm1);

				tm0 = sum_subject0 + sum_object0;
				tm0 = mul_vec(tm0, base_vector0);
				tm1 = sum_subject1 + sum_object1;
				tm1 = mul_vec(tm1, base_vector1);

				float c5 = cosine_sim(tm0, tm1);
				
				tm0 = sum_subject0 + sum_object0 + base_vector0;
				tm1 = sum_subject1 + sum_object1 + base_vector1;
				
				float c6 = cosine_sim(tm0, tm1);
			
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

				std::cout << stream.str();

			}
		}
	}
}
