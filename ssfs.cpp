/* https://is2-ssl.mzstatic.com/image/thumb/Video/v4/ed/79/b0/ed79b0c0-7617-a714-15be-2378cdb58221/source/1200x630bb.jpg */

#include "inode.h"

#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <pthread.h>
#include <map>

pthread_cond_t fill, empty;
pthread_mutex_t mutex;

std::string disk_file_name;
int num_blocks;
int block_size;
int files_in_system;
std::map<std::string, int> inode_map;
int free_block_list[512];

void get_system_parameters();
void build_inode_map();
void build_free_block_list();

int createFile(std::string fileName);
void deleteFile(std::string fileName);
bool write(std::string fname, char to_write, int start_byte, int num_bytes);
void read(std::string fname, int start_byte, int num_bytes);

void list();
//bool insert(*inode lilwayne);

void *read_file(void *arg){
	std::ifstream opfile;
	char* file_name = (char*)arg;
	opfile.open(file_name);
	if(!opfile){
		perror(file_name);
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
			//disk.create(ssfs_file);
		}else if(command == "IMPORT"){
			line_stream >> ssfs_file;
			std::string unix_file;
			line_stream >> unix_file;
			std::cout << "Importing unix file " << unix_file << " as \'" << ssfs_file << "\'" << std::endl;
			//disk.import(ssfs_file,unix_file);
		}else if(command == "CAT"){
			line_stream >> ssfs_file;
			std::cout << "Contents of " << ssfs_file << std::endl;
			//disk.cat(ssfs_file);
		}else if(command == "DELETE"){
			line_stream >> ssfs_file;
			std::cout << "Deleting " << ssfs_file << std::endl;
			//disk.delete(ssfs_file);
		}else if(command == "WRITE"){
			line_stream >> ssfs_file;
			char c;
			int start_byte, num_bytes;
			line_stream >> c;
			line_stream >> start_byte;
			line_stream >> num_bytes;
			std::cout << "Writing character '" << c << "' into "<< ssfs_file << " from byte " << start_byte << " to byte " << (start_byte + num_bytes) << std::endl;
			//disk.write(ssfs_file,character,start_byte,num_bytes);
		}else if(command == "READ"){
			line_stream >> ssfs_file;
			int start_byte, num_bytes;
			line_stream >> start_byte;
			line_stream >> num_bytes;
			std::cout << "Reading file " << ssfs_file << " from byte " << start_byte << " to byte " << (start_byte + num_bytes) << std::endl;
			//disk.read(ssfs_file,start_byte,num_bytes);
		}else if(command == "LIST"){
			std::cout << "Listing" << std::endl;
			//disk.list();
		}else if(command == "SHUTDOWN"){
			std::cout << "Shutting down " << file_name << "..." << std::endl;
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
//	build_inode_map();

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

// File System startup scan below --------

void get_system_parameters() {
	std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

	std::string line;
	getline(disk, line, ' ');
	num_blocks = std::stoi(line);
	getline(disk, line, ' ');
	block_size = std::stoi(line);
	getline(disk, line, '|');
	files_in_system = std::stoi(line);

	disk.close();
}

// Disk Ops below ------------------------


void deleteFile(std::string fileName){
    //Get the inode from the inode map using fileName as the key
	//int target = inodeMap[fileName];
    //return the blocks to the freelist
	//
	//target.indirectblocks clear
	//target.directblocks clear
    //remove the inode from the inode map
	//free_block_list.push_back(target);
	//inodemap.erase(fileName)
}

void list(){
	//for each element in inodemap, display the inode->name and inode->size
	//map<string,int>::iterator it = map.begin();
	//string fileName;
	//int inodeBlock;
	//int fileSize;
		//while(it!= map.end()){
		//fileName = it -> first;
		//inodeBlock = it -> second;
		//parse inodeblock for filesize = fileSize;
		//cout << "Name: " << fileName << "::Size: "<< fileSize << " bytes" << endl;
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

//	inode inode = INODE_MAP[fname]
//	int current_size = inode.file_size;
	int real_read_length = num_bytes;
//	if(current_size < start_byte){
		std::cout << "Start byte is out of range" << std::endl;
//	}
//	if((start_byte + num_bytes) > inode.file_size){
//		real_read_length =(num_bytes - ((start_byte + num_bytes) >
//		 inode.file_size));
//	}
}
int createFile(std::string fileName){
	/*if(inode_map.count(fileName)==1){
		cout<< "create command failed, file named " << fileName << " already exists." << endl;
	}else{
		if(!free_block_list.empty()){
			int targetblock == free_block_list.front()
			write inode data to the targetblock
			inode_map[fileName] = targetblock;
		}else{
		cout<<"create command failed, there is no room left on the disk for " << filename << endl;
		}
	}*/
}
