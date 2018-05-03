#include <stdio.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string.h>

using namespace std;

class inode {
    public:
	inode(string file_name, int file_size) {
		this->file_name = file_name;
		this->file_size = file_size;
	}

        string file_name;
        int file_size;
        int direct_blocks [12];
        int * indirect_block;
	    int ** double_indirect_block;
}
