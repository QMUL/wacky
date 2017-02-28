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

#include "wacky_sbj_obj.hpp"
#include "string_utils.hpp"

using namespace std;
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
