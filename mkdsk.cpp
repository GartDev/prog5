#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <fstream>
#include <iostream>

#include <sys/time.h>
#include <sys/types.h>

void print_usage_string();

int main(int argc, char * argv[ ]) {
	int num_blocks;
	int block_size;
	std::string disk_file_name;

	// argument parsing
	if (argc == 3) {
		disk_file_name = "DISK";
	} else if (argc == 4) {
		disk_file_name = argv[3];
	} else {
		std::cout << "Incorrect number of arguments." << std::endl;
		print_usage_string();
		exit(1);
	}

	// argument parsing
	try {
		num_blocks = std::stoi(argv[1]);
		block_size = std::stoi(argv[2]);
	} catch (std::invalid_argument e) {
		std::cout << "num_blocks and block_size must be valid integers." << std::endl;
		print_usage_string();
		exit(1);
	}

	// checking argument validity
	if (num_blocks < 1024 or num_blocks > 131072) {
		std::cout << "Error: need 1024 <= num_blocks <= 128K." << std::endl;
		exit(1);
	}
	if (block_size < 128 or block_size > 512) {
		std::cout << "Error: need 128 <= block_size <= 512." << std::endl;
	}

	// writing disk parameters and other cool stuff to top
//	std::ofstream disk_output(disk_file_name, std::ios::out | std::ios::binary);

	FILE * disk_output = fopen(disk_file_name.c_str(), "wb");
	truncate(disk_file_name.c_str(), num_blocks*block_size);

	std::string num_blocks_s = std::to_string(num_blocks);
	std::string block_size_s = std::to_string(block_size);

	const char * num_blocks_c = num_blocks_s.c_str();
	const char * block_size_c = block_size_s.c_str();


	// DISK BLOCK SYNTAX: BLOCKS ARE REPRESENTED BY LINES, SO BLOCK 1 IS THE FIRST
	// LINE, BLOCK 2 IS THE SECOND, ETC.

	// LINE (BLOCK) 1 HAS THE DISK DATA INCLUDING PARAMETERS,
	// INODE MAP, AND FREE BLOCK LIST

	// LINE 1 OF DISK: PARAMETERS
	// format: "num_blocks block_size files_in_system"
	// num blocks, a space, block size, a space, number of files in the system
	fwrite(num_blocks_c, sizeof(char), num_blocks_s.length(), disk_output);
	fwrite(" ", sizeof(char), 1, disk_output);
	fwrite(block_size_c, sizeof(char), block_size_s.length(), disk_output);
	fwrite(" 0\n", sizeof(char), 3, disk_output);

	// LINE 2 OF DISK: INODE MAP
	// format: "file_name:inode_location file_name2:inode_location ..."
	// file name followed by : followed by its location. files are 
	// separated by spaces. still deciding how to represent location
	fwrite("sam:loc\n", sizeof(char), 8, disk_output);

	// LINE 3 OF DISK: FREE BLOCKS
	// format: "<block#>-<block#> <block#>-<block#> ..."
	// list of block numbers that are free, 1-4 means that
	// blocks 1, 2, 3, and 4 are free but not 5, etc..
	fwrite("2-", sizeof(char), 2, disk_output);
	fwrite(num_blocks_c, sizeof(char), num_blocks_s.length(), disk_output);

	fwrite(" \n", sizeof(char), 2, disk_output);

	// close disk
	fclose(disk_output);

	return 0;
}

void print_usage_string() {
	std::cout << "./ssfs_mkdsk <num blocks> <block size> <disk file name>" << std::endl;
}
