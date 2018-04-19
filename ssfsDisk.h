#include <stdio.h>
#include <fstream>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string.h>

using namespace std;

class ssfsDisk {
    public:
	ssfsDisk(int num_blocks, int block_size, string disk_file_name) {
		this->num_blocks = num_blocks;
		this->block_size = block_size;
		this->num_files = 0;
		this->disk_file_name = disk_file_name;
	}

        int num_blocks;
        int block_size;
	int num_files;
	string disk_file_name;

};
