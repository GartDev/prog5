#include <stdio.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <vector>

class inode {
    public:
	inode() {
		this->file_name = "";
		this->file_size = -1;
	}

	inode(std::string file_name, int file_size) {
		this->file_name = file_name;
		this->file_size = file_size;
	}

        std::string file_name;
		int location;
        int file_size;
        std::vector<int> direct_blocks;
        int indirect_block;
		int double_indirect_block;
};
