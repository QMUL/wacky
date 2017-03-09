/**
* @brief A little bit of benchmarking
* @file wacky_bench.cc
* @author Benjamin Blundell <oni@section9.co.uk>
* @date 14/02/2017
*
*/


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
#include <time.h>


#ifdef _USE_MKL
#include "wacky_sbj_obj_mkl.hpp"
#else
#include "wacky_sbj_obj.hpp"
#endif

#include "string_utils.hpp"

using namespace std;

#ifdef _USE_MKL

int main(int argc, char* argv[]) {

  time_t start,end;
  size_t ss = 5000;
  double dif;
  
  time(&start);
 
  vector<float> tk (ss * ss);
  
  for (int i = 0; i < 1000; ++i) {
    vector<float> krn_vector0 (ss);
    vector<float> krn_vector1 (ss);
  
    for (int j=0; j < ss; ++j){
      krn_vector0[j] = 2.0f;
      krn_vector1[j] = 2.0f;
    }

    krn_mul(krn_vector0, krn_vector1, tk);
  }
  time(&end);

  dif = difftime (end,start);
  printf("%f",dif);
 
  cout << endl;

  MKL_INT n = ss * ss;
  vsAdd(n,&tk[0],&tk[0],&tk[0]); 

  cout << tk[0] << endl;

}

#else

using namespace boost::numeric;

int main(int argc, char* argv[]) {

  time_t start,end;
  size_t ss = 5000;
  double dif;
  
  time(&start);
  for (int i = 0; i < 1000; ++i) {
    ublas::vector<float> krn_vector0 (ss);
    ublas::vector<float> krn_vector1 (ss);
  
    for (int j=0; j < ss; ++j){
      krn_vector0 (j) = krn_vector1(j) = 1.0f;
    }

    ublas::vector<float> tk = krn_mul(krn_vector0, krn_vector1);
  }
  time(&end);

  dif = difftime (end,start);
  printf("%f",dif);

}
#endif
