/* https://is2-ssl.mzstatic.com/image/thumb/Video/v4/ed/79/b0/ed79b0c0-7617-a714-15be-2378cdb58221/source/1200x630bb.jpg */

#include "inode.h"
#include <fstream>
#include <limits>
#include <stdlib.h>
#include <sstream>
#include <pthread.h>
#include <map>
#include <vector>

using namespace std;

pthread_cond_t fill, empty;
pthread_mutex_t mutex;

std::string disk_file_name;
int num_blocks;
int block_size;
int files_in_system;
std::map<std::string, inode> inode_map;
std::string free_block_list;

void get_system_parameters();
void build_inode_map();
void build_free_block_list();

int b60_to_decimal(const char * target);
const char * decimal_to_b60(int target);

std::fstream& go_to_line(std::fstream& file, unsigned int num);

int createFile(std::string fileName);
void deleteFile(std::string fileName);
//bool write(std::string fname, char to_write, int start_byte, int num_bytes);
//void read(std::string fname, int start_byte, int num_bytes);
void ssfsCat(std::string fileName);
void list();
int atCapacity(int lineNum,int flag);
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
	build_free_block_list();
	build_inode_map();

	std::cout << "Conversion: " << decimal_to_b60(19021) << std::endl;

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
//	std::cout<<"block size " << block_size << std::endl;
//	std::cout<<"num blocks " << num_blocks << std::endl;

	std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

	std::string line;
	//Skip the first 5 lines of the file from the super block
	
	for(int i = 1; i < (num_blocks/block_size + 2); i++){
		getline(disk, line, '\n');
	}

	//seekg beginning + num_blocks + block_size many characters
//	std::cout << "what: " << (block_size)*(3+(num_blocks/(block_size-1))) << std::endl;
	disk.seekg(block_size*(3+(num_blocks/(block_size-1)) - 1), std::ios::beg);
    int length = disk.tellg();

//	std::cout << "disk length " << length << std::endl;
	//getline(disk, line, '\n')
	//disk.seekg(disk.cur,num_blocks);

	int counter = 0;
	while (getline(disk, line, '\n') && line[0] != '\0' && counter < 256) {
			inode this_node;
			std::string token;
			std::istringstream line_stream(line);
			int data_num = 0;
			while(getline(line_stream,token,':')){
				std::stringstream ss;
				if(data_num == 0){
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
					this_node.file_name = token;
//					std::cout << "file name " << ": " << this_node.file_name << std::endl;
				}else if(data_num == 1){
					ss << std::hex << token;
					ss >> this_node.location;
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
//					std::cout << "location " << ": " << this_node.location << std::endl;
				}else if(data_num == 2){
					ss << std::hex << token;
					ss >> this_node.file_size;
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
//					std::cout << "size: " << this_node.file_size << std::endl;
				}else if(data_num == 3){
					std::istringstream token_stream(token);
					while(getline(token_stream,token,' ')){
						//std::cout << "sub token " << data_num <<  ": " << token << std::endl;
						std::stringstream hex_conv;
						int block;
						hex_conv << std::hex << token;
						hex_conv >> block;
//						std::cout << "block: " << block << std::endl;
						this_node.direct_blocks.push_back(block);
					}
//					std::cout << std::endl;
				}else if(data_num == 4){
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
					ss << std::hex << token;
					ss >> this_node.indirect_block;
//					std::cout << "iblock: " << this_node.indirect_block << std::endl;
				}else if(data_num == 5){
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
					ss << std::hex << token;
					ss >> this_node.double_indirect_block;
//					std::cout << "double iblock: " << this_node.double_indirect_block << std::endl;
				}
				data_num++;
			}
			counter++;
	}
	//std::cout << counter << std::endl;

	disk.close();
}

void build_free_block_list() {
	std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

	std::string num;
	getline(disk, num, '\n');

	free_block_list = "";

	while (getline(disk, num, '\n') && num[0] != '\0') {
		free_block_list += num;
	}

	disk.close();
}

// Disk Ops below ------------------------

void deleteFile(std::string fileName){
/*	int directsum = 0;
	if(!inode_map[fileName].direct_blocks.empty()){
		for(int i = 0; i<inode_map[fileName].direct_blocks.size(); i++){
			if(inode_map[fileName].direct_blocks[i]!=0){
				sum += 1;
				int freeIndex = inode_map[fileName].direct_blocks[i]-1;
				free_block_list[freeIndex]= '0';
			}
		}
	}
	if(sum == 12){
		ofstream
		int indirectsum = 0;
			for(i)
	}
	/*if(!inode_map[fileName].double_indirect_blocks.empty()){
		for(int i = 0; i<inode_map[fileName].double_indirect_blocks.size(); i++){
			for(int j = 0; j<inode_map[fileName].double_indirect_blocks[i].size(); j++){
				int freeIndex = inode_map[fileName].double_indirect_blocks[i][j]-1;
				free_block_list[freeIndex]= '0';
			}
		}
	}
	if(!inode_map[fileName].indirect_blocks.empty()){
		for(int i = 0; i<inode_map[fileName].indirect_blocks.size(); i++){
			int freeIndex = inode_map[fileName].indirect_blocks[i]-1;
			free_block_list[freeIndex]= '0';
		}
	}
	int targetBlock = inode_map[fileName].location;
	inode_map.erase(fileName);
	free_block_list.push_back(targetBlock);
	*/
}

/*
void list(){
	//for each element in inodemap, display the inode->name and inode->size
	map<string,inode>::iterator it = inode_map.begin();
	string fileName;
	//inode myNode;
	int fileSize;
		while(it!= inode_map.end()){
		std::string fileName = it->first;
		fileSize = inode_map[fileName].file_size;
		std::cout << "Name: " << fileName << "::Size: "<< fileSize << " bytes\n";
//
}*/

/*bool write(std::string fname, char to_write, int start_byte, int num_bytes){
//	inode inode = inode_map[fname];
}

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
}*/

/*
void read(std::string fname, int start_byte, int num_bytes){
	//gotta find the block pointer

	inode myNode = inode_map[fname];

	int current_size = myNode.file_size;
	int real_read_length = num_bytes;
	if(current_size < start_byte){
		std::cout << "Start byte is out of range" << std::endl;
	}
	if((start_byte + num_bytes) > file_size){
		real_read_length =((start_byte + num_bytes) - file_size);
	}

}
*/

int createFile(std::string fileName){
/*
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
				inode myNode(fileName,0);
				inode_map[filename] = myNode;
				inode_map[fileName].location = targetblock;

			}else{
				std::cerr<<"create command failed, there is no room left on the inodeMap for " << fileName << "\n";
				return(0);
			}
		}else{
			std::cerr<<"create command failed, there is no room left on the disk for " << fileName << "\n";
			return(0);
		}
	}
	return(1);
	*/
}

void ssfsCat(std::string fileName){
	/*
	int targetBlock;
	char * buffer = new char[block_size];
	int offset = (targetBlock-1)*block_size;
	FILE * pfile;
	const char * diskfile = disk_file_name.c_str();
	pfile= fopen(diskfile,"r");
	fseek(pfile,offset,SEEK_SET);
	fgets(buffer,block_size,pfile);
//	inode myNode = inode_map[fileName];
//	int fileSize = myNode.file_size;
	//read(fileName, 0, fileSize);
	*/
}
/*
int add_blocks(std::string fname, int num_blocks){
	inode target_inode = inode(fname, inode_map[fname].file_size);
	int not_taken = -1;
	int i;
	//this loop checks if we have direct blocks open and allocates
	while(num_blocks != 0){

		for(i = 0; i < 12; i++){
			if(target_inode.direct_blocks[i] == 0){
				not_taken = i;
				break;
			}
		}
		if(not_taken == -1){
			//that means that all the direct blocks are taken
			break;
		}
		else{

			target_inode.direct_blocks[not_taken] = block;
			//reading free block liist
			int block = -1;
			for(i = (int)(3+(num_blocks/(block_size-1)))+256; i < free_block_list.size(); i++){
				if(free_block_list[i] = 0){
					block = i-1;
					free_block_list[i] = 1;
					break;
				}
			}
			if(block == -1){
				return -1;
			}
			else{
			target_inode.direct_blocks[not_taken] = block;
			num_blocks--;
			}
		}
	}
	if(num_blocks == 0){
		return 0;
	}
	// this is the indirect block level
	//notes: block_size/sizeof(int)
	if(at_capacity(target_inode.indirect_blocks, 0) == 0){
		while(num_blocks != 0){
			int block = -1;
			for(i = (int)(3+(num_blocks/(block_size-1)))+256; i < free_block_list.size(); i++){
				if(free_block_list[i] = 0){
					block = i-1;
					free_block_list[i] = 1;
					break;
				}	
			}
			if(block == -1){
				return -1;
			}
			else{
			//writing to indirect block
				int ind_blk = target_inode.indirect_blocks;
				num_blocks--;
				std::ofstream ofs;
				std::ifstream ifs;
				ifs.open(disk_file_name, std::ifstream::in);
				seekp(std::ios_base::beg, (block-1)block_size);
				string line = getline();
			//	
			}
			num_blocks--;
					
			}
				else{break;}
		}
	}
	if(num_blocks == 0){
		return 0;
	}
	//this is the double indirect level
	if(at_capacity(target_inode.indirect_blocks, 0) == 0){
		while(num_blocks != 0){
			if(target_inode.double_indirect_blocks.size() < (block_size/sizeof(int))){
				int block = 2;
				if(target_inode.double_indirect_blocks.back().size() < (block_size/sizeof(int))){
					target_inode.
					free_block_list.pop_back();
					num_blocks--;
				}
			else{break;}
//return false;
			}
		}
	}
	return -1;
}
*/
void shutdown_globals() {
	std::ofstream disk(disk_file_name, std::ios::in | std::ios::out | std::ios::binary);

//	go_to_line(disk, 1);

	disk.seekp(std::ios_base::beg);

	std::string num_blocks_s = std::to_string(num_blocks);
	std::string block_size_s = std::to_string(block_size);
	std::string files_in_system_s = std::to_string(files_in_system);

	disk.write(num_blocks_s.c_str(), num_blocks_s.length()*sizeof(char));
	disk.write(" ", sizeof(char));
	disk.write(block_size_s.c_str(), block_size_s.length()*sizeof(char));
	disk.write(" ", sizeof(char));
	disk.write(files_in_system_s.c_str(), files_in_system_s.length()*sizeof(char));

	int pos = disk.tellp();

	disk.seekp(pos+(block_size-(sizeof(char)*(num_blocks_s.length()+block_size_s.length()+files_in_system_s.length()+2))));


	int left = num_blocks - (num_blocks/block_size)*(block_size-1);

	int j = 0;
	int loops = 0;
	int i;
	for (i = 0 ; i < num_blocks/block_size ; i++) {
		int j;
		for (j = 0 ; j < block_size-1 ; j++) {
			const char * b = free_block_list.c_str();
			disk.put(b[i*(block_size-1)+j]);
 		}

		disk.write("\n", sizeof(char));
	}


	while (j < num_blocks/block_size) {
		int i;
		for (i = 0 ; j < num_blocks/block_size && i < block_size-1 ; i++) {
			const char * a = free_block_list.c_str();
			disk.put(a[num_blocks-(left-i-1)-1+(loops*(block_size-1))]);
			j++;
		}

		if (i == block_size-1) {
			disk.write("\n", sizeof(char));
			loops += 1;
		}
	}

	left -= (loops*(block_size-1));

	disk.seekp(std::ios_base::beg + (loops+2)*block_size + num_blocks);

	inode sample;
	sample.file_name = "sample.txt";
	sample.file_size = 128;
	sample.location = (num_blocks/(block_size-1))+3;
	sample.direct_blocks[0] = 320;
	sample.direct_blocks[1] = 990;
	sample.direct_blocks[2] = 900;
	sample.double_indirect_block = 444;

	inode sample2;
	sample2.file_name = "sample2.txt";
	sample2.file_size = 256;
	sample2.location = sample.location+1;
	sample2.direct_blocks[0] = 333;
	sample2.direct_blocks[1] = 991;
	sample2.direct_blocks[2] = 1000;
	sample2.indirect_block = 902;

	inode_map["sample.txt"] = sample;
	inode_map["sample2.txt"] = sample2;

	std::map<std::string, inode>::iterator it;

	for (it = inode_map.begin() ; it != inode_map.end() ; it++) {

		int seek = 0;

		disk.write(it->second.file_name.c_str(), it->second.file_name.length()*sizeof(char));
		disk.write(":", sizeof(char));
		seek += it->second.file_name.length()*sizeof(char) + sizeof(char);

		char h[5];
		sprintf(h, "%x", it->second.location);

		disk.write(h, std::string(h).length()*sizeof(char));
		disk.write(":", sizeof(char));
		seek += std::string(h).length()*sizeof(char) + sizeof(char);

		sprintf(h, "%x", it->second.file_size);
		disk.write(h, std::string(h).length()*sizeof(char));
		disk.write(":", sizeof(char));
		seek += std::string(h).length()*sizeof(char) + sizeof(char);

		int i;
		for (i = 0 ; i < it->second.direct_blocks.size() ; i++) {
			sprintf(h, "%x", it->second.direct_blocks[i]);
			disk.write(h, std::string(h).length()*sizeof(char));

			if (i == 11) {
				disk.write(":", sizeof(char));
			} else {
				disk.write(" ", sizeof(char));
			}

			seek += std::string(h).length()*sizeof(char) + sizeof(char);
		}

		sprintf(h, "%x", it->second.indirect_block);
		disk.write(h, std::string(h).length()*sizeof(char));
		disk.write(":", sizeof(char));
		seek += std::string(h).length()*sizeof(char) + sizeof(char);

		sprintf(h, "%x", it->second.double_indirect_block);
		disk.write(h, std::string(h).length()*sizeof(char));
		seek += std::string(h).length()*sizeof(char);

		int pos = disk.tellp();

		disk.seekp(pos+(block_size-seek));
	}

	disk.close();
}

int b60_to_decimal(const char * target) {
	int ret = 0;
	int i;
	for (i = 0 ; i < std::string(target).length() ; i++) {
		int cur;
		if (isdigit(target[i])) {
			cur = target[i]-'0';

		} else {
			char x = target[i];
			int value = int(x);

			if (value > 90) {
				value -= 6;
			}

			cur = value-55;
		}

		int set = 1;
		int j;
		for (j = i ; j < std::string(target).length()-1 ; j++) {
			set *= 60;
		}

		ret += cur*set;
	}

	return ret;
}

const char * decimal_to_b60(int target) {
	std::string first = "";
	std::string second = "";
	std::string third = "";	
	char f;
	char s;
	char t;

	int num;

	if (target >= 3600) {
		num = (target / 3600);

		if (num <= 9) {
			first = std::to_string(num);

		} else {
			num += 55;

			if (num > 90) {
				num += 6;
			}

			f = char(num);
			first = std::string(1, f);
		}
	}

	target %= 3600;

	if (target >= 60) {
		num = (target / 60);

		if (num <= 9) {
			second = std::to_string(num);
		} else {
			num += 55;

			if (num > 90) {
				num += 6;
			}

			s = char(num);
			second = std::string(1, s);
		}
	}

	target %= 60;

	num = target;

	if (num <= 9) {
		third = std::to_string(num);
	} else {
		num += 55;

		if (num > 90) {
			num += 6;
		}

		t = char(num);
		third = std::string(1, t);
	}

	std::string ret = first + second + third;

	return ret.c_str();

}

int atCapacity(int lineNum,int flag){
	if(flag == 0){
		int capCount = 0;
		int capacity = block_size / sizeof(int);
		ifstream diskFile;
		diskFile.open(disk_file_name);
		int pos = std::ios_base::beg + ((lineNum -1) * block_size);
		diskFile.seekg(pos);
		std::string ibLine;
		getline(diskFile,ibLine,'\n');
		std::size_t found = ibLine.find("0",0);
		if(found != std::string::npos){
			return(0);
		}else{
			return(1);
		}
	}else if(flag == 1){
		int capCount = 0;
		int capacity = block_size / sizeof(int);
		ifstream diskFile;
		diskFile.open(disk_file_name);
		int pos = std::ios_base::beg + ((lineNum -1) * block_size);
		diskFile.seekg(pos);
		std::string ibLine;
		getline(diskFile,ibLine,'\n');
		std::size_t found = ibLine.find("0",0);
		if(found != std::string::npos){
			return(0);
		}else{
			std::stringstream ss(ibLine);
			int lastidblock;
			while(1) {
			   ss >> lastidblock;
			   if(!ss)
			      break;
			}
			atCapacity(lineNum,lastidblock);
		}
	}

}
