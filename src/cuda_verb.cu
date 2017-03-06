/**
* @brief The CUDA version of the verb
* @file cuda_verb.cu
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 14/07/2016
*
*/


#include "cuda_verb.hpp"

using namespace std;
 using namespace boost::numeric;


// Our actual kernel that basically performs vector addition
__global__ void VerbSubjectZero(float *output) {

  unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;   
  output[x]  = 0.0f;
}




// Our actual kernel that basically performs vector addition
__global__ void VerbSubjectAdd(int width, int height, float *input, float *output) {

  unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;   
  unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

  unsigned int i = y * width + x; // index of current pixel (calculated using thread index)

  if (i < height * width){
    output[x] += input[i];   
  }
}


// Our actual kernel that runs over the screen and spits out a colour
__global__ void VerbSubjectKrn(int width, int height, float *input, float *input2, float *output) {

  unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;   
  unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

  unsigned int i = y * width + x; // index of current pixel (calculated using thread index)
  unsigned int j= 0;

  if (i < width * height){
    output[i] = input[i] * input2[i];
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
 * @param krn_vector a ublas vector of verb (x) verb
 */

void read_subjects_cuda(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SUBJECTS,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    ublas::vector<float> & base_vector,
    ublas::vector<float> & add_vector,
    ublas::vector<float> & krn_vector) {
 
  int vidx = DICTIONARY_FAST[verb];
  vector<int> subjects = VERB_SUBJECTS[vidx];

  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector[i] = WORD_VECTORS[vidx][i];
  } 
 
  for (int i=0; i < BASIS_SIZE; ++i){
    add_vector[i] = 0.0f;
    
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
  }
}


/**
 * Given a verb, return the sums of the subjects and objects
 * @param verb a string we are looking at
 * @param DICTIONARY_FAST the fast lookup dictionary
 * @param VERB_SBJ_OBJ the vector of vectors of subjects and objects
 * @param WORD_VECTORS the word vectors converted to probabilities
 * @param BASIS_SIZE the size of our word vectors
 * @param base_vector a ublas vector of verb x verb 
 * @param sum_subject a ublas vector of verb subjects summed
 * @param sum_krn a ublas vector of the verb subs objs kroneckered
 */

void read_subjects_objects_cuda(string verb, map<string,int> & DICTIONARY_FAST,
    vector< vector<int> > & VERB_SBJ_OBJ,
    vector< vector<float> > & WORD_VECTORS,
    int BASIS_SIZE,
    ublas::vector<float> & base_vector,
    ublas::vector<float> & sum_subject,
    ublas::vector<float> & sum_krn) {


  // Copy all the subject vectors into our memory block for transfer
  // into CUDA - this is likely SLOOOOW
  int vidx = DICTIONARY_FAST[verb];
  vector<int> subs_obs = VERB_SBJ_OBJ[vidx];
  vector<float> subs_obs_conv;

  // We modify the Basis size to fit it nicely into memory (ish)
  size_t real_width = BASIS_SIZE + (BASIS_SIZE % 8);

  for (int i : subs_obs) {
    for (int j = 0; j < BASIS_SIZE; ++j) {
      subs_obs_conv.push_back( WORD_VECTORS[i][j]);
    }
    for (int j = 0; j < real_width - BASIS_SIZE; ++j){
      subs_obs_conv.push_back(0);
    }
  }

  // Copy the base vector
  for (int i=0; i < BASIS_SIZE; ++i){
    base_vector[i] = WORD_VECTORS[vidx][i];
  } 
 
  cout << "Verb: " << verb << endl; 

  float* input_s;
  float* input_k;

  // Results on the device
  float* output_s;

  // results on the host
  float* result_s = new float[real_width];
  float* result_k = new float[real_width];


	// allocate memory on the CUDA device (GPU VRAM) for input, sum and kronecker
	cudaMalloc(&output_s, real_width * sizeof(float));

  cout << "Cuda Error M0? " << cudaGetErrorString(cudaGetLastError()) << endl;
  cout << "Mem required in bytes " << subs_obs_conv.size() * sizeof(float) << endl;

  // Break up and loop

  size_t mem_size = 536870912; // Should check this against the card

  size_t ts = subs_obs_conv.size() * sizeof(float); 
  size_t num_blocks = ts / mem_size;
  size_t chunk_size = ts / num_blocks;

  if (num_blocks < 1) {
    num_blocks = 1;
    chunk_size = ts;
  }

  //size_t vecs_per_block = chunk_size / (real_width * sizeof(float));
  size_t vecs_per_block = 1;

  cout << "Sizes: " << endl;
  cout << num_blocks << ", " << vecs_per_block << ", " << chunk_size << ", " << subs_obs_conv.size() <<  endl;

  // Allocate pinned memory
  cudaMallocHost(&input_s, real_width * vecs_per_block * sizeof(float));
  cudaMallocHost(&input_k, real_width * vecs_per_block * sizeof(float));
  cout << "Cuda Error M2 " << cudaGetErrorString(cudaGetLastError()) << endl;

  // Zero everything first
  
  size_t idx = 0;

  dim3 threadsPerBlock(8, 1);
  dim3 numBlocks(real_width / threadsPerBlock.x, vecs_per_block);
  
  VerbSubjectZero <<< threadsPerBlock, numBlocks >>> (output_s);  

  while (idx < subs_obs_conv.size()) {
  
    size_t tsize = 1;
    //size_t tsize = vecs_per_block;
    //if (idx + vecs_per_block > subs_obs_conv.size()){
    //  tsize = subs_obs_conv.size() - idx;
    //}

    cudaMemcpy( input_s, static_cast<void*>(&subs_obs_conv[idx]), tsize * real_width * sizeof(float), cudaMemcpyHostToDevice);
    VerbSubjectAdd  <<< numBlocks, threadsPerBlock >>>(real_width, tsize, input_s, output_s);  

    idx +=  (tsize * real_width);
  }

  cudaDeviceSynchronize(); // Probably dont need this now
  // copy results of computation from device back to host
  cudaMemcpy(result_s, output_s, BASIS_SIZE * sizeof(float), cudaMemcpyDeviceToHost);  
  
  for (int i =0; i < BASIS_SIZE; ++i){
    sum_subject[i] = result_s[i];  
  }


  // Now perform the kronecker which is basically an N^2 / 2 number of ops

  for (int i=0; i < BASIS_SIZE; ++i) { 

    idx = 0;

    // TODO maybe eventually use tsize here - fewer mem copies
    cudaMemcpy( input_s, static_cast<void*>(&subs_obs_conv[i]), 1 * real_width * sizeof(float), cudaMemcpyHostToDevice);
    
    while (idx < subs_obs_conv.size()) {
  
      size_t tsize = 1;
      //size_t tsize = vecs_per_block;
      //if (idx + vecs_per_block > subs_obs_conv.size()){
      //  tsize = subs_obs_conv.size() - idx;
      //}

      cudaMemcpy( input_k, static_cast<void*>(&subs_obs_conv[idx]), tsize * real_width * sizeof(float), cudaMemcpyHostToDevice);
      VerbSubjectKrn  <<< numBlocks, threadsPerBlock >>>(real_width, tsize, input_s, input_k, output_s);  

      // copy results of computation from device back to host
      cudaMemcpy(result_s, output_s, real_width * sizeof(float), cudaMemcpyDeviceToHost);  
  
      for (int j =0; j < BASIS_SIZE; ++j){
        sum_krn[i*BASIS_SIZE + j] += result_s[j];  
      }

      idx += (tsize * real_width);
    }

    cudaDeviceSynchronize(); // Probably dont need this now
  }


  // free CUDA memory
	cudaFree(output_s);  
	cudaFree(input_s);
  cudaFree(input_k);

  delete[] result_s;

}

/**
 * Return all the stats for all verb pairs CUDA version
 * @param VERBS_TO_CHECK a vector of VerbPair
 * @param VERB_TRANSITIVE the list of transitive verbs
 * @param VERB_INTRANSITIVE the list of intransitive verbs
 * @param BASIS_SIZE the size of our word vectors
 * @param DICTIONARY_FAST the fast dictionary 
 * @param VERB_SBJ_OBJ the vector of vectors of verb subject-object pairs
 * @param VERB_SUBJECTS the vector of verb subjects
 * @param WORD_VECTORS our word count vectors
 */


void all_count_cuda( std::vector<VerbPair> & VERBS_TO_CHECK,
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
		if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
			VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){
			//cout << vp.v0 << "," << vp.v1 << endl;
			total_verbs ++;
		}	
	}

	cout << "Total verbs: " << s9::ToString(total_verbs) << endl; 
  cout << "verb0,verb1,base_sim,cs1,cs2,cs3,cs4,cs5,cs6,human_sim" << endl;

	int num_blocks = 1;

	//#pragma omp parallel
	//{
	//	num_blocks = omp_get_num_threads();
	//}

	int block_size = VERBS_TO_CHECK.size() / num_blocks;

  int block_id = omp_get_thread_num();
  int start = block_size * block_id;
  int end = block_size * (block_id + 1);

  if (block_id + 1 == num_blocks){
    end = VERBS_TO_CHECK.size();
  }

  ublas::vector<float> base_vector0 (BASIS_SIZE);
  ublas::vector<float> sum_subject0 (BASIS_SIZE);
  ublas::vector<float> sum_krn0 (BASIS_SIZE * BASIS_SIZE);

  ublas::vector<float> base_vector1 (BASIS_SIZE);
  ublas::vector<float> sum_subject1 (BASIS_SIZE);
  ublas::vector<float> sum_krn1 (BASIS_SIZE * BASIS_SIZE);

  ublas::vector<float> krn_base0 (BASIS_SIZE * BASIS_SIZE);
  ublas::vector<float> krn_base1 (BASIS_SIZE * BASIS_SIZE);

  for (int i=start; i < end; ++i){

    VerbPair vp = VERBS_TO_CHECK[i];

    // Set the vectors to zeros
    sum_subject0.clear();	
    sum_subject1.clear();				
    sum_krn0.clear();
    sum_krn1.clear();

    if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
        VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){

      read_subjects_objects_cuda(vp.v0,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

      read_subjects_objects_cuda(vp.v1,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);
 
      ublas::vector<float> tv0 (BASIS_SIZE);
      ublas::vector<float> tv1 (BASIS_SIZE);

      tv0 = sum_subject0 + base_vector0;
      tv1 = sum_subject1 + base_vector1;

      float c2 = cosine_sim(tv0,tv1);
      cout << "CUDA cosine sim " << c2 << endl;

      krn_base0 = krn_mul(base_vector0, base_vector0);
      krn_base1 = krn_mul(base_vector1, base_vector1);
   
      float c4 = cosine_sim(sum_krn0, sum_krn1);

      cout << "CUDA cosine sim krn " << c4 << endl;
      //break;

    } else if(VERB_TRANSITIVE.find(vp.v0) == VERB_TRANSITIVE.end() &&
        VERB_TRANSITIVE.find(vp.v1) != VERB_TRANSITIVE.end()){

      //read_subjects_cuda(vp.v0,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

      //read_subjects_objects_cuda(vp.v1,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);

      continue;

    } else if(VERB_TRANSITIVE.find(vp.v0) != VERB_TRANSITIVE.end() &&
        VERB_TRANSITIVE.find(vp.v1) == VERB_TRANSITIVE.end()){
    
      //read_subjects_objects_cuda(vp.v0,DICTIONARY_FAST, VERB_SBJ_OBJ, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

      //read_subjects_cuda(vp.v1,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);
      continue;

    } else {
    
      //read_subjects_cuda(vp.v0,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector0, sum_subject0, sum_krn0);

      //read_subjects_cuda(vp.v1,DICTIONARY_FAST, VERB_SUBJECTS, WORD_VECTORS, BASIS_SIZE, base_vector1, sum_subject1, sum_krn1);
      continue;
    }

    /*krn_base0 = krn_mul(base_vector0, base_vector0);
    krn_base1 = krn_mul(base_vector1, base_vector1);
    
    float c0 = cosine_sim(base_vector0, base_vector1);
    float c1 = cosine_sim(sum_subject0, sum_subject1);
    
    ublas::vector<float> tv0 (BASIS_SIZE);
    ublas::vector<float> tv1 (BASIS_SIZE);

    tv0 = sum_subject0 + base_vector0;
    tv1 = sum_subject1 + base_vector1;

    float c2 = cosine_sim(tv0,tv1);
    tv0 = mul_vec(sum_subject0,base_vector0);
    tv1 = mul_vec(sum_subject1,base_vector1);
    float c3 = cosine_sim(tv0,tv1);
    float c4 = cosine_sim(sum_krn0, sum_krn1);


    ublas::vector<float> tk0 (BASIS_SIZE * BASIS_SIZE);
    tk0 = sum_krn0 + krn_base0;

    ublas::vector<float> tk1 (BASIS_SIZE * BASIS_SIZE);
    tk1 = sum_krn1 + krn_base1;
    float c5 = cosine_sim(tk0, tk1);

    tk0 = mul_vec(sum_krn0, krn_base0);
    tk1 = mul_vec(sum_krn1,krn_base1);
    float c6 = cosine_sim(tk0, tk1);

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
*/
  }
}
