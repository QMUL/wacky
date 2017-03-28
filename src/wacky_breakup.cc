#include "wacky_misc.hpp"

using namespace boost::interprocess;
using namespace std;



/**
 * Breakup our files into blocks 
 * @param block_pointer pointer to pointer to our block of memory to break
 * @param block_size the size of our blocks
 * @param m_file the boost mapped files
 * @param region the boost mapped region
 * @return int a value to say if we succeeded or not
 */

int breakup ( char ** & block_pointer, size_t * & block_size, file_mapping &m_file, mapped_region &region, int & num_blocks) {
  // Scan directory for the files
	num_blocks = 1; 
	
  size_t size = region.get_size();

  if (size > 1024) {
	  #pragma omp parallel
	  {
		  num_blocks = omp_get_num_threads();
	  }
  }

  block_pointer = new char*[num_blocks];
  block_size = new size_t[num_blocks];

  // we don't breakup if the region size < 1024 bytes - theres no point really

  cout << "Num Blocks: " << num_blocks << endl;
  
  if (size > 1024) {

    // Problem with memory-mapped files is we need the number of line
    // endings in order to split the file for OpenMP processing on the same
    // file. Using one thread per file is probably easier. 
    //
    // OR we could just move the pointers within the file backwards till we hit
    // a newline and set it there. That would probably be quite easy

    try {

      void * addr = region.get_address();
      size_t step = size / num_blocks;
      cout << "Step Size " << step << "," << size << endl;

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
  
    } catch (interprocess_exception &ex) {
		  fprintf(stderr, "Exception %s\n", ex.what());
		  fflush(stderr);
		  return 1;
	  }
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

  return 0;
}




