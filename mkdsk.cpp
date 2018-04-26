#include "ssfsDisk.cpp"

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
		num_blocks = stoi(argv[1]);
		block_size = stoi(argv[2]);
	} catch (invalid_argument e) {
		std::cout << "num_blocks and block_size must be valid integers." << std::endl;
		print_usage_string();
		exit(1);
	}

	// checking argument validity
	if (num_blocks < 1024 or num_blocks > 128000) {
		std::cout << "Error: need 1024 <= num_blocks <= 128K." << std::endl;
		exit(1);
	}
	if (block_size < 128 or block_size > 512) {
		std::cout << "Error: need 128 <= block_size <= 512." << std::endl;
	}

	// writing disk parameters to top
	ofstream disk_output(disk_file_name, ios::out | ios::binary);

	std::string num_blocks_s = to_string(num_blocks);
	std::string block_size_s = to_string(block_size);

	const char * num_blocks_c = num_blocks_s.c_str();
	const char * block_size_c = block_size_s.c_str();
	const char * disk_file_name_c = disk_file_name.c_str();

	disk_output.write(num_blocks_c, num_blocks_s.length()*sizeof(char));
	disk_output.write(block_size_c, block_size_s.length()*sizeof(char));
	disk_output.write(disk_file_name_c, disk_file_name.length()*sizeof(char));

	/* TESTING BINARY WRITE SHENANIGANS
	ssfsDisk * disk = new ssfsDisk(num_blocks, block_size, disk_file_name);

	const char * disk_c = (const char *) disk;
	disk_output.write(disk_c, sizeof(ssfsDisk));

	ifstream disk_input(disk_file_name, ios::in | ios::binary);

	char new_disk_c[sizeof(ssfsDisk)];
	disk_input.read(new_disk_c, sizeof(ssfsDisk));

	ssfsDisk * disk2 = (ssfsDisk *) new_disk_c;

	std::cout << disk2->num_blocks << std::endl;
	std::cout << disk2->block_size << std::endl;
	std::cout << disk2->num_files << std::endl;
	std::cout << disk2->disk_file_name << std::endl;
	
	disk_input.close();
	*/

	// close disk
	disk_output.close();

	return 0;
}

void print_usage_string() {
	std::cout << "./ssfs_mkdsk <num blocks> <block size> <disk file name>" << std::endl;
}
