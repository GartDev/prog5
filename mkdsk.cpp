#include "ssfsDisk.cpp"

void print_usage_string();

int main(int argc, char * argv[ ]) {
	int num_blocks;
	int block_size;
	std::string disk_file_name;

	if (argc == 3) {
		disk_file_name = "DISK";
	} else if (argc == 4) {
		disk_file_name = argv[3];
	} else {
		std::cout << "Incorrect number of arguments." << std::endl;
		print_usage_string();
		exit(1);
	}

	try {
		num_blocks = stoi(argv[1]);
		block_size = stoi(argv[2]);
	} catch (invalid_argument e) {
		std::cout << "num_blocks and block_size must be valid integers." << std::endl;
		print_usage_string();
		exit(1);
	}

	if (num_blocks < 1024 or num_blocks > 128000) {
		std::cout << "Error: need 1024 <= num_blocks <= 128K." << std::endl;
		exit(1);
	}

	if (block_size < 128 or block_size > 512) {
		std::cout << "Error: need 128 <= block_size <= 512." << std::endl;
	}

	ofstream disk_output(disk_file_name, std::ofstream::binary);

	

	disk.close();
}

void print_usage_string() {
	std::cout << "./ssfs_mkdsk <num blocks> <block size> <disk file name>" << std::endl;
}
