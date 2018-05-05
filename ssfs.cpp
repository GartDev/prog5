/* https://is2-ssl.mzstatic.com/image/thumb/Video/v4/ed/79/b0/ed79b0c0-7617-a714-15be-2378cdb58221/source/1200x630bb.jpg */

#include "inode.h"
#include <fstream>
#include <limits>
#include <stdlib.h>
#include <sstream>
#include <pthread.h>
#include <map>
#include <vector>

pthread_cond_t fill, empty;
pthread_mutex_t mutex;

std::string disk_file_name;
int num_blocks;
int block_size;
int files_in_system;
std::string free_block_list;
std::map<std::string, inode> inode_map;

void get_system_parameters();
void build_inode_map();
void build_free_block_list();

std::fstream& go_to_line(std::fstream& file, unsigned int num);

int createFile(std::string fileName);
void deleteFile(std::string fileName);
bool write(std::string fname, char to_write, int start_byte, int num_bytes);
void read(std::string fname, int start_byte, int num_bytes);
void ssfsCat(std::string fileName);
void list();
//bool insert(*inode lilwayne);

void shutdown_globals();

void *read_file(void *arg){
	std::ifstream opfile;
	char* thread_name = (char*)arg;
	opfile.open(thread_name);
	if(!opfile){
		perror(thread_name);
		pthread_exit(NULL);
	}
	//print lines of the file, don't delete yet
	std::string line;
	while(std::getline(opfile,line)){
        std::cout << line << std::endl;
		std::istringstream line_stream(line);
		std::string command;
		line_stream >> command;
		std::string ssfs_file;
		if(command == "CREATE"){
			line_stream >> ssfs_file;
			std::cout << "Creating " << ssfs_file << std::endl;
			//create(ssfs_file);
		}else if(command == "IMPORT"){
			line_stream >> ssfs_file;
			std::string unix_file;
			line_stream >> unix_file;
			std::cout << "Importing unix file " << unix_file << " as \'" << ssfs_file << "\'" << std::endl;
			//import(ssfs_file,unix_file);
		}else if(command == "CAT"){
			line_stream >> ssfs_file;
			std::cout << "Contents of " << ssfs_file << std::endl;
			//cat(ssfs_file);
		}else if(command == "DELETE"){
			line_stream >> ssfs_file;
			std::cout << "Deleting " << ssfs_file << std::endl;
			//deleteFile(ssfs_file);
		}else if(command == "WRITE"){
			line_stream >> ssfs_file;
			char c;
			int start_byte, num_bytes;
			line_stream >> c;
			line_stream >> start_byte;
			line_stream >> num_bytes;
			std::cout << "Writing character '" << c << "' into "<< ssfs_file << " from byte " << start_byte << " to byte " << (start_byte + num_bytes) << std::endl;
			//write(ssfs_file,character,start_byte,num_bytes);
		}else if(command == "READ"){
			line_stream >> ssfs_file;
			int start_byte, num_bytes;
			line_stream >> start_byte;
			line_stream >> num_bytes;
			std::cout << "Reading file " << ssfs_file << " from byte " << start_byte << " to byte " << (start_byte + num_bytes) << std::endl;
			//read(ssfs_file,start_byte,num_bytes);
		}else if(command == "LIST"){
			std::cout << "Listing" << std::endl;
			//list();
		}else if(command == "SHUTDOWN"){
			std::cout << "Saving and shutting down " << thread_name << "..." << std::endl;
			//shutdown();
			pthread_exit(NULL);
		}else {
			std::cout << line << ": command not found" << std::endl;
		}
	}
	opfile.close();
	pthread_exit(NULL);
}

int main(int argc, char **argv){
	if(argc < 2){
		std::cout << "Error: too few arguments." << std::endl;
		exit(1);
	}

	disk_file_name = std::string(argv[1]);
	get_system_parameters();
//	free_block_list = new int[num_blocks+1];
	build_free_block_list();
	//build_inode_map();

	shutdown_globals();
/*
	inode * s = new inode("sample.txt", 128);

	int j;
	for (j = 0 ; j < 12 ; j++) {
		s->direct_blocks[j] = 2*j;
	}

	FILE * disk = fopen(disk_file_name.c_str(), "rb+");
	fwrite(s, sizeof(char), sizeof(inode), disk);
	fclose(disk);

	inode * s2;

	char c[sizeof(inode)];

	FILE * disk2 = fopen(disk_file_name.c_str(), "rb");
	fread(c, sizeof(char), sizeof(inode), disk2);

	s2 = (inode *) c;
	std::cout << s2->file_name << std::endl;
	std::cout << s2->file_size << std::endl;
	fclose(disk2);

	for (j = 0 ; j < 12 ; j++) {
		std::cout << s2->direct_blocks[j] << std::endl;
	}
*/

	pthread_t p;
	int rc;
	for(int i = 2; i < argc; i++){
		rc = pthread_create(&p, NULL, read_file, (void*)argv[i]);
		if(rc){
			std::cout << "Unable to create thread." << std::endl;
			exit(0);
		}
	}
	pthread_exit(NULL);
}

std::fstream& go_to_line(std::fstream& file, unsigned int num){
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}

// File System startup scan below --------

void get_system_parameters() {
	std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

	std::string line;
	getline(disk, line, ' ');
	num_blocks = std::stoi(line);
	getline(disk, line, ' ');
	block_size = std::stoi(line);
	getline(disk, line, '\n');
	files_in_system = std::stoi(line);

	disk.close();
}

void build_inode_map() {
	std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

	std::string line;
	getline(disk, line, '\n');
	getline(disk, line, '\n');

	while (line.length() != 0) {
		std::string cur = line.substr(0, line.find(' '));

		int colon = cur.find(':');

		inode_map[cur.substr(0, colon)] = std::stoi(cur.substr(colon+1, cur.length()));
		line = line.substr(line.find(' ')+1, line.length());
	}

	disk.close();
}

void build_free_block_list() {
	std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

	free_block_list = "";

	std::string num;
	getline(disk, num, '\n');

	while (getline(disk, num, '\n') && num[0] != '\0') {
		free_block_list += num;
	}

	disk.close();
}

// Disk Ops below ------------------------

void deleteFile(std::string fileName){
    //Get the inode from the inode map using fileName as the key
	/*int targetBlock =
    //return the blocks to the freelist
	char * buffer = new char[block_size];
	int offset = (targetBlock-1)*block_size;
	FILE * pfile;
	const char * diskfile = disk_file_name.c_str();
	pfile= fopen(diskfile,"r");
	fseek(pfile,offset,SEEK_SET);
	fgets(buffer,block_size,pfile);*/
	inode myNode = inode_map[fileName];
	int fileSize = myNode.file_size;
	bool isempty = 1;//used for direct blocks, in an array
	int sum = 0;
	for(int i = 0; i<sizeof(myNode.direct_blocks);i++){
		sum+= myNode.direct_blocks[i];
	}
	if(sum != 0){
		isempty = 0;
	}
	if(!myNode.double_indirect_blocks.empty()){
		for(int i = 0; i<sizeof(myNode.double_indirect_blocks); i++){

			free_block_list.push_back(myNode.double_indirect_blocks[i]);
		}
	}
	if(!myNode.indirect_blocks.empty()){
		for(int i = 0; i<sizeof(myNode.indirect_blocks); i++){
			free_block_list.push_back(myNode.indirect_blocks[i]);
		}
	}
	if(isempty == 1){
			for(int i = 0; i<sizeof(myNode.direct_blocks); i++){
				free_block_list.push_back(myNode.direct_blocks[i]);
		}
	}
    //remove the inode from the inode map
	targetBlock = inodeMap[fileName].location;
	inode_map.erase(fileName);
	free_block_list.push_back(targetBlock);
}

void list(){
	//for each element in inodemap, display the inode->name and inode->size
	map<string,inode>::iterator it = inode_map.begin();
	string fileName;
	inode myNode;
	int fileSize;
		while(it!= inode_map.end()){
		fileName = it -> first;
		myNode = it -> second;
		fileSize = myNode.file_size;
		cout << "Name: " << fileName << "::Size: "<< fileSize << " bytes\n";
//
}

bool write(std::string fname, char to_write, int start_byte, int num_bytes){
//	inode inode = inode_map[fname];

	//check file size, return error if start_byte is out of bounds
//	int current_size = inode.file_size;
//	if(current_size < start_byte){
		return false;
//	}
	//check if file needs to be extended
//	if(current_size	< (start_byte + num_bytes)){
		if(sizeof(free_block_list) == 0){
			return false;
		}
		else{
		//expand the file
		}
//	}
}

void read(std::string fname, int start_byte, int num_bytes){
	//gotta find the block pointer

/*	inode inode = inode_map[fname];


	//construct the inode from the data in DISK
	*char * disk_name_c = new char [disk_file_name.length()+1]
	std::strcpy (cstr, disk_file_name.disk_name_c)
	FILE * open_disk = fopen(disk_name_c, "rb")
	fseek(open_disk, (inode_pos-1)*block_size, SEEK_SET)
	char * inode_raw = fscanf(open_disk,
	inode inode = (inode*)inode_raw;


	int current_size = inode.file_size;
	int real_read_length = num_bytes;
	if(current_size < start_byte){
		std::cout << "Start byte is out of range" << std::endl;
	}
	if((start_byte + num_bytes) > file_size){
		real_read_length =((start_byte + num_bytes) - file_size);
	}	
	
*/
}
int createFile(std::string fileName){
	if(inode_map.count(fileName)==1){
		std::cerr<< "create command failed, file named " << fileName << " already exists." << "\n";
		return(0);
	}else{
		if(!free_block_list.empty()){
			int targetblock = 0;
			for(int i = 0;i<free_block_list.size();++i){
				if(free_block_list[i] <= 262){
					int targetblock = free_block_list[i];
					break;
				}
			}
			if(!targetblock == 0){
				//write inode data to the targetblock

				inode myNode

			}else{
				std::cerr<<"create command failed, there is no room left on the inodeMap for " << fileName << "\n";
				return(0);
			}
		}else{
			std::cerr<<"create command failed, there is no room left on the disk for " << fileName << "\n";
			return(0);
		}
	}
	return(1);*/
}
void ssfsCat(std::string fileName){
	/*int targetBlock
	char * buffer = new char[block_size];
	int offset = (targetBlock-1)*block_size;
	FILE * pfile;
	const char * diskfile = disk_file_name.c_str();
	pfile= fopen(diskfile,"r");
	fseek(pfile,offset,SEEK_SET);
	fgets(buffer,block_size,pfile);*/
	inode myNode = inode_map[fileName];
	int fileSize = myNode.file_size;
	read(fileName, 0, fileSize);
}

void shutdown_globals() {
	std::ofstream disk(disk_file_name, std::ios::in | std::ios::out | std::ios::binary);

//	go_to_line(disk, 1);

//	disk.seekp(std::ios_base::beg);

	std::string num_blocks_s = std::to_string(num_blocks);
	std::string block_size_s = std::to_string(block_size);
	std::string files_in_system_s = std::to_string(files_in_system);

	disk.write(num_blocks_s.c_str(), num_blocks_s.length()*sizeof(char));
	disk.write(" ", sizeof(char));
	disk.write(block_size_s.c_str(), block_size_s.length()*sizeof(char));
	disk.write(" ", sizeof(char));
	disk.write(files_in_system_s.c_str(), files_in_system_s.length()*sizeof(char));
	
	disk.seekp(disk.tellp()+(block_size-(sizeof(char)*(num_blocks_s.length()+block_size_s.length()+files_in_system_s.length()+2))));

	int i;
	for (i = 0 ; i < num_blocks/block_size ; i++) {
		int j;
		for (j = 0 ; j < block_size-1 ; j++) {
			const char * b = free_block_list.c_str();
			disk.put(b[i*(block_size-1)+j]);
		}
		disk.write("\n", sizeof(char));
	}

	int left = num_blocks - (num_blocks/block_size)*(block_size-1);

	int j = 0;
	int loops = 0;
	while (j < num_blocks/block_size) {
		int i;
		for (i = 0 ; j < num_blocks/block_size && i < block_size-1 ; i++) {
			const char * a = free_block_list.c_str();
			std::cout << i+loops*(block_size-1) << std::endl;
			disk.put(a[num_blocks-(left-i-1)-1+(loops*(block_size-1))]);
			j++;
		}

		if (i == block_size-1) {
			disk.write("\n", sizeof(char));
			loops += 1;
		}
	}



	disk.close();
}
