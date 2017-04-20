// So the default way of including the main func for boost test doesnt work when using boost shared object libraries
// so we do it this way instead. Kept the original way in case we change back.
//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE wacky basic test suite
#include <boost/test/included/unit_test.hpp>


#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <dirent.h>
#include <omp.h>
#include <deque>

#include "string_utils.hpp"
#include "wacky_math.hpp"

using namespace std;

// Lets test the mkl

BOOST_AUTO_TEST_CASE(math_test) {

#ifdef _USE_MKL_

  vector<float> tv0 = {1,2,3,4,5,6,7,8,9,0};
  vector<float> tv1 = {1,2,3,4,5,6,7,8,9,0};
 
  vsAdd(10, &tv0[0], &tv1[0], &tv1[0]);

  BOOST_CHECK_EQUAL(tv1[2], 6);

  //vsMul(nsize, &sum_krn0[0], &krn_base0[0], &tk0[0]);

#endif


}
