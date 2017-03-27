// So the default way of including the main func for boost test doesnt work when using boost shared object libraries
// so we do it this way instead. Kept the original way in case we change back.
//#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE wacky basic test suite
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(simple_test) {
  BOOST_CHECK_EQUAL(2+2, 4);
}
