#include "wacky_sbj_obj.hpp"

using namespace boost::numeric;
using namespace std;


// Create the vectors we want from a verb's subjects
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
      krn_vector[i*j] = 1.0f;
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


void intrans_count( std::vector<VerbPair> & VERBS_TO_CHECK,
  set<string> & VERB_TRANSITIVE,
  set<string> & VERB_INTRANSITIVE,
  int BASIS_SIZE,
  map<string,int> & DICTIONARY_FAST,
  vector< vector<int> > & VERB_SUBJECTS,
  vector< vector<float> > & WORD_VECTORS) {
  cout << "verb0,verb1,base_sim,add_sim,min_sim,max_sim,add_add_sim,add_mul_sim,min_add_sim,min_mul_sim,     max_add_sim,max_mul_sim,krn_sim,krn_add_sim,krn_mul_sim,human_sim" << endl;

  for (VerbPair vp : VERBS_TO_CHECK){
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

      cout << vp.v0 << "," << vp.v1 << "," << s9::ToString(c0)
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

    }
  }

}


