#include "ssfsDisk.cpp"
#include <unistd.h>
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
		num_blocks = stoi(argv[1]);
		block_size = stoi(argv[2]);
	} catch (invalid_argument e) {
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
	ofstream disk_output(disk_file_name, ios::out | ios::binary);
	truncate(disk_file_name.c_str(), num_blocks*block_size);

	std::string num_blocks_s = to_string(num_blocks);
	std::string block_size_s = to_string(block_size);

	const char * num_blocks_c = num_blocks_s.c_str();
	const char * block_size_c = block_size_s.c_str();

	// LINE 1 OF DISK: PARAMETERS
	// format: "num_blocks block_size files_in_system"
	// num blocks, a space, block size, a space, number of files in the system
	disk_output.write(num_blocks_c, num_blocks_s.length()*sizeof(char));
	disk_output.write(" ", sizeof(char));
	disk_output.write(block_size_c, block_size_s.length()*sizeof(char));
	disk_output.write(" 0", 2*sizeof(char));

	disk_output.write("\n", sizeof(char));

	// LINE 2 OF DISK: INODE MAP
	// format: "file_name:inode_location file_name2:inode_location ..."
	// file name followed by : followed by its location. files are 
	// separated by spaces. still deciding how to represent location
	disk_output.write("sam:loc", 7*sizeof(char));

	disk_output.write("\n", sizeof(char));

	// LINE 3 OF DISK: FREE BLOCKS
	// format: "<block#>-<block#> <block#>-<block#> ..."
	// list of block numbers that are free, 1-4 means that
	// blocks 1, 2, 3, and 4 are free but not 5, etc..
	disk_output.write("1-", 2*sizeof(char));
	disk_output.write(num_blocks_c, num_blocks_s.length()*sizeof(char));

	disk_output.write("\n", sizeof(char));

	// close disk
	disk_output.close();

	return 0;
}

void print_usage_string() {
	std::cout << "./ssfs_mkdsk <num blocks> <block size> <disk file name>" << std::endl;
}
