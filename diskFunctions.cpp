#include "diskFunctions.h"

bool write(std::string fname, char to_write, int start_byte, int num_bytes){
	//i need some way to search the disk for a file by name, that would go here
	//for now I'll treat this pointer boy as if its a pointer to a block object indexed by integers
	Block * the_block;
	//check block size, return error if start_byte is out of bounds

	//check if file needs to be extended
		
	
}

void read(std::string fname, int start_byte, int num_bytes){
	//gotta find the block pointer
}
