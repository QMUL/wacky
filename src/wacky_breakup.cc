#include "wacky_misc.hpp"

using namespace boost::interprocess;
using namespace std;

// Breakup our files into blocks 

int breakup ( char ** block_pointer, size_t * block_size, file_mapping &m_file, mapped_region &region) {
  // Scan directory for the files
	int num_blocks =1; 
		
	#pragma omp parallel
	{
		num_blocks = omp_get_num_threads();
	}

	cout << "Num Blocks: " << num_blocks << endl;

	// Problem with memory-mapped files is we need the number of line
	// endings in order to split the file for OpenMP processing on the same
	// file. Using one thread per file is probably easier. 
	//
	// OR we could just move the pointers within the file backwards till we hit
	// a newline and set it there. That would probably be quite easy

	try {

		void * addr = region.get_address();
		size_t size = region.get_size();

		size_t step = size / num_blocks;
		cout << "Step Size " << step <<endl;

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

	} catch (interprocess_exception &ex) {
		fprintf(stderr, "Exception %s\n", ex.what());
		fflush(stderr);
		return 1;
	}

	return 0;
}




