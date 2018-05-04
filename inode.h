#include <stdio.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <vector>

class inode {
    public:
	inode(std::string file_name, int file_size) {
		this->file_name = file_name;
		this->file_size = file_size;
	}

        std::string file_name;
        int file_size;
        int direct_blocks [12];
        int indirect_blocks [12];
	int double_indirect_blocks [12];
};
