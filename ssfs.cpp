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
std::string decimal_to_b60(int target);

std::fstream& go_to_line(std::fstream& file, unsigned int num);

int createFile(std::string fileName);
void deleteFile(std::string fileName);
int write(std::string fname, char to_write, int start_byte, int num_bytes);
void read(std::string fname, int start_byte, int num_bytes);
void ssfsCat(std::string fileName);
void list();
int atCapacity(int lineNum,int flag);
void shutdown_globals();
void import(std::string ssfs_file, std::string unix_file);

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
			createFile(ssfs_file);
		}else if(command == "IMPORT"){
			line_stream >> ssfs_file;
			std::string unix_file;
			line_stream >> unix_file;
			std::cout << "Importing unix file " << unix_file << " as \'" << ssfs_file << "\'" << std::endl;
			import(ssfs_file,unix_file);
		}else if(command == "CAT"){
			line_stream >> ssfs_file;
			std::cout << "Contents of " << ssfs_file << std::endl;
			ssfsCat(ssfs_file);
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
			list();
		}else if(command == "SHUTDOWN"){
			std::cout << "Saving and shutting down " << thread_name << "..." << std::endl;
			//shutdown();
			shutdown_globals();
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

	//read("sample2.txt", 381, 2519);



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

	return 0;
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
					free_block_list[this_node.location-1] = '1';
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
//					std::cout << "location " << ": " << this_node.location << std::endl;
				}else if(data_num == 2){
					ss << std::hex << token;
					ss >> this_node.file_size;
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
//					std::cout << "size: " << this_node.file_size << std::endl;
				}else if(data_num == 3){
					std::istringstream token_stream(token);
					int i = 0;
					while(getline(token_stream,token,' ')){
//						std::cout << "sub token " << data_num <<  ": " << token << std::endl;
						std::stringstream hex_conv;
						int block;
						hex_conv << std::hex << token;
						hex_conv >> block;
//						std::cout << "block: " << block << std::endl;
						this_node.direct_blocks[i] = block;
						if (this_node.direct_blocks[i] != 0) {
							free_block_list[this_node.direct_blocks[i]-1] = '1';
						}
						i++;
					}
//					std::cout << std::endl;
				}else if(data_num == 4){
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
					ss << std::hex << token;
					ss >> this_node.indirect_block;
					if (this_node.indirect_block != 0) {
						free_block_list[this_node.indirect_block-1] = '1';
					}
//					std::cout << "iblock: " << this_node.indirect_block << std::endl;
				}else if(data_num == 5){
					//std::cout << "token " << data_num <<  ": " << token << std::endl;
					ss << std::hex << token;
					ss >> this_node.double_indirect_block;
					if (this_node.double_indirect_block != 0) {
						free_block_list[this_node.double_indirect_block-1] = '1';
					}
//					std::cout << "double iblock: " << this_node.double_indirect_block << std::endl;
				}
				data_num++;
			}
			inode_map[this_node.file_name] = this_node;

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
	//create empty block to be written over indirect/double indirect
	char * toWrite = new char[block_size];
	char * zeroed = new char[4];
	const char * temp = decimal_to_b60(0).c_str();
	strcpy(zeroed,temp);
	for(int i = 0;i<(block_size-1);i++){
		strcat(toWrite,'\0');
	}
	toWrite[block_size-1]='\n';
	//empty direct
	int directsum = 0;
	if(inode_map.count(fileName)==0){
		std::cout << fileName << ": does not exist" << std::endl;
		return;
	}
	if(!inode_map[fileName].direct_blocks.empty()){
		for(int i = 0; i<inode_map[fileName].direct_blocks.size(); i++){
			if(inode_map[fileName].direct_blocks[i]!=0){
				directsum += 1;
				int freeIndex = inode_map[fileName].direct_blocks[i]-1;
				free_block_list[freeIndex]= '0';
			}
		}
	}
	//free all indirect/double indirect blocks
	if(directsum == 12){ //if direct_blocks are full
		if(atCapacity(inode_map[fileName].indirect_block,0) == 1){ //if indirect_blocks are full
			//Begin Reading line
			ifstream diskFile;
			diskFile.open(disk_file_name);
			ofstream writeFile;
			writeFile.open(disk_file_name);
			//empty double indirect
			int lineNum = inode_map[fileName].double_indirect_block;
			int pos = std::ios_base::beg + ((lineNum -1) * block_size);
			diskFile.seekg(pos);
			std::string dibLine; //dibLine is string containing block
			getline(diskFile,dibLine,'\n');
			//Finish Reading line into string
			int idremoveblock;
			char * target = new char [dibLine.length()+1];
  			strcpy (target, dibLine.c_str());
			const char * p = strtok(target," ");
			while(p!=NULL){ //for each number in double indirect
				p = strtok(NULL," ");
				idremoveblock = b60_to_decimal(p);
				int id_linenum = idremoveblock; //Number of each indirect_block in double
				int id_pos = std::ios_base::beg + ((id_linenum-1)*block_size);
				diskFile.seekg(id_pos);
				std::string ibLine;
				getline(diskFile,ibLine,'\n');
				int dblock;
				char * idtarget = new char [ibLine.length()+1];
	  			strcpy (idtarget, ibLine.c_str());
				const char * q = strtok(idtarget," ");
				while(q!=NULL){//for each numbre in indirect

					q = strtok(NULL," ");
					dblock = b60_to_decimal(q);
					//free direct block
					free_block_list[dblock] = 0;
				}
				//free each indirect_block and overwrite them
				free_block_list[idremoveblock-1] = 0;
				writeFile.seekp(id_pos);
				writeFile.write(toWrite, block_size);
			}
			free_block_list[inode_map[fileName].double_indirect_block-1] = 0;
			writeFile.seekp(pos);
			writeFile.write(toWrite, block_size);
			//finish emptying double indirect block
			//start emptying indirect_block
			int indirect_block_num = inode_map[fileName].indirect_block;
			int indirect_pos = std::ios_base::beg + ((indirect_pos-1)*block_size);
			int dblock;
			diskFile.seekg(indirect_pos);
			std::string indirect_line;
			getline(diskFile,indirect_line,'\n');
			char * id_line = new char[indirect_line.length()+1];
			strcpy (id_line, indirect_line.c_str());
			const char * blocknum = strtok(id_line," ");
			while(blocknum!=NULL){

				blocknum = strtok(NULL," ");
				dblock = b60_to_decimal(blocknum);
				free_block_list[dblock-1] = 0;
			}
			free_block_list[inode_map[fileName].indirect_block-1] = 0;
			writeFile.seekp(indirect_pos);
			writeFile.write(toWrite, block_size);
			diskFile.close();
			writeFile.close();
			//finish emptying indirect_block

		}else{ //if indirect_block isnt full
			ifstream diskFile;
			diskFile.open(disk_file_name);
			ofstream writeFile;
			writeFile.open(disk_file_name);
			int indirect_block_num = inode_map[fileName].indirect_block;
			int indirect_pos = std::ios_base::beg + ((indirect_pos-1)*block_size);
			int dblock;
			diskFile.seekg(indirect_pos);
			std::string indirect_line;
			getline(diskFile,indirect_line,'\n');
			char * id_line = new char[indirect_line.length()+1];
			strcpy (id_line, indirect_line.c_str());
			const char * blocknum = strtok(id_line," ");
			while(blocknum!=NULL){
				blocknum = strtok(NULL," ");
				dblock = b60_to_decimal(blocknum);
				free_block_list[dblock-1] = 0;
			}
			free_block_list[inode_map[fileName].indirect_block-1] = 0;
			writeFile.seekp(indirect_pos);
			writeFile.write(toWrite, block_size);
			diskFile.close();
			writeFile.close();
		}

	}
	free_block_list[inode_map[fileName].location] = 0;
	inode_map.erase(fileName);
	return;
}

void list(){
	//for each element in inodemap, display the inode->name and inode->size
	map<string,inode>::iterator it;
	for(it = inode_map.begin(); it != inode_map.end(); it++){
		std::cout << it->second.file_name << " size: " << it->second.file_size << " bytes" << std::endl;
	}
}

int write(std::string fname, char to_write, int start_byte, int num_bytes){
	inode target = inode_map[fname];
	int control = 0;
	//check file size, return error if start_byte is out of bounds
	int current_size = target.file_size;
	if(current_size < start_byte){
		std::cout << "Start byte is out of range" << std::endl;
	}
	//check if file needs to be extended
	else if(current_size < (start_byte + num_bytes)){
		if(sizeof(free_block_list) == 0){
			return 1;
		}
		else{
			//expand file
			std::cout << "add_blocks() ain't work yet sorry" << std::endl;
			return 1;
		}
	}
	else{
		std::ofstream disk(disk_file_name, std::ofstream::out);

		int num_blocks = num_bytes / (block_size-1);
		int start_block = start_byte/ (block_size-1);
		int block = target.direct_blocks[start_block];
		int traverse = start_byte / (block_size-1);

		disk.seekp((block-1)*(block_size) + (start_byte%(block_size-1)), std::ios::beg);
		int written;
		int loops = 0;

		while(start_block < 12 && num_bytes > 0) {

			block = target.direct_blocks[start_block];
			if(loops != 0){
			disk.seekp((block-1)*(block_size), std::ios::beg);
			}
			written = 0;



			while((start_byte + written) < (block_size-1) ){
				disk.put(to_write);
				written++;
				num_bytes--;
				if(num_bytes == 0){break;}
				written++;
			}
			start_block++;
			loops++;
			traverse++;

		}
		if(num_bytes > 0){
			return 0;
		}
		else{
			control = 1;
		}
		// now we need to read the indirect block to keep going
		std::ifstream disk_in(disk_file_name, std::ios::in | std::ios::binary);
		int id_block = target.indirect_block;
		while(start_block >= 12 && start_block < (12+(block_size/4)) && num_bytes > 0) {


			disk_in.seekg((id_block-1)*(block_size), std::ios::beg);

			std::string line;
			getline(disk_in, line, '\n');

			int mini_traverse = traverse - 12;

			while (mini_traverse > 0) {
				line = line.substr(line.find(' ')+1, line.length());
				mini_traverse -= 1;
			}

			line = line.substr(0, line.find(' '));

			if (line == "000") {
				disk_in.close();
				disk.close();
				std::cout << "Reached end of file while writing" << std::endl;
				return 1;
			}

			int direct = b60_to_decimal(line.c_str());

			disk.seekp((direct-1)*(block_size) + ((start_byte%(block_size-1))), std::ios::beg);

			if(control == 0){
				written = 0;



				while((start_byte + written) < (block_size-1) ){
					disk.put(to_write);
					num_bytes--;
					if(num_bytes == 0){break;}
					written++;
				}
				start_block++;
				loops++;
				traverse++;

			}
			else{
				int i;
				for(i = 0; i < (block_size-1); i++){
					disk.put(to_write);
					num_bytes--;
					if(num_bytes == 0){break;}
					written++;
				}
				start_block++;
				loops++;
				traverse++;
			}

		}
		if(num_bytes == 0){disk_in.close();disk.close();return 0;}
	//	else if(){

//		}
	}
	return 1;
}

void read(std::string fname, int start_byte, int num_bytes){
	//gotta find the block pointer

	inode readme = inode_map[fname];
	int current_size = readme.file_size;

	if(current_size < start_byte){
		std::cout << "Start byte is out of range" << std::endl;

	} else {
		std::ifstream disk(disk_file_name, std::ios::in | std::ios::binary);

		std::string last = "";

		if((start_byte + num_bytes-1) > current_size){
			num_bytes = current_size - start_byte + 1;
		}

		start_byte -= 1;

		int traverse = start_byte / (block_size-1);

		int macro_traverse = 0;

		while (traverse < 12 and num_bytes > 0) {

			int block = readme.direct_blocks[traverse];

			disk.seekg((block-1)*(block_size) + (start_byte%(block_size-1)), std::ios::beg);

			std::string line;
			getline(disk, line, '\n');

			line = line.substr(0, std::min(num_bytes, block_size-(start_byte%(block_size-1))));

			if (start_byte < num_bytes) {
				num_bytes -= (block_size-start_byte-1);
			} else {
				num_bytes -= (block_size - ((1 + start_byte)%(block_size-1)));
			}

			last += line;
			traverse += 1;
			start_byte = 0;

		}

		while (traverse >= 12 and traverse < (12+(block_size/4)) and num_bytes > 0) {
			int id_block = readme.indirect_block;

			disk.seekg((id_block-1)*(block_size), std::ios::beg);

			std::cout << num_bytes << " " << start_byte << std::endl;

			std::string line;
			getline(disk, line, '\n');

			int mini_traverse = traverse - 12;

			while (mini_traverse > 0) {
				line = line.substr(line.find(' ')+1, line.length());
				mini_traverse -= 1;
			}

			line = line.substr(0, line.find(' '));

			if (line == "000") {
				break;
			}

			int direct = b60_to_decimal(line.c_str());

			disk.seekg((direct-1)*(block_size) + ((start_byte%(block_size-1))), std::ios::beg);
			getline(disk, line, '\n');

			std::cout << line << std::endl;

			line = line.substr(0, std::min(num_bytes, std::abs(block_size-start_byte)));

			if (num_bytes <= block_size-1) {
				last += line;
				break;
			} else if (start_byte <= num_bytes) {
				num_bytes -= (block_size-start_byte-1);
			} else {
				num_bytes -= (block_size-((1+start_byte)%(block_size-1)));
			}

			last += line;
			start_byte = 0;
			traverse += 1;

		} while (traverse >= (12+(block_size/4)) and num_bytes > 0) {
			int did_block = readme.double_indirect_block;

			disk.seekg((did_block-1)*(block_size), std::ios::beg);

			std::string line;
			getline(disk, line, '\n');

			int mini_traverse2 = traverse - (12+(block_size/4));

			if (mini_traverse2 % ((block_size/4)+1) == block_size/4) {
				macro_traverse += 1;
				mini_traverse2 = 0;

			} else {
				std::string get_block;
				getline(disk, get_block, '\n');

				int count = 0;
				while (mini_traverse2 > 0) {
					get_block = get_block.substr(get_block.find(' ')+1, get_block.length());
					mini_traverse2 -= 1;
					count += 1;
				}

				get_block = get_block.substr(0, get_block.find(' '));

				if (get_block == "000") {
					break;
				}

				int id_block = b60_to_decimal(get_block.c_str());

				disk.seekg((id_block-1)*(block_size), std::ios::beg);

				std::string line;
				getline(disk, line, '\n');

				int mini_traverse = traverse - 12;

				while (mini_traverse > 0) {
					line = line.substr(line.find(' ')+1, line.length());
					mini_traverse -= 1;
				}

				line = line.substr(0, line.find(' '));

				if (line == "000") {
					break;
				}

				int direct = b60_to_decimal(line.c_str());

				disk.seekg((direct-1)*(block_size) + ((start_byte%(block_size-1))), std::ios::beg);
				getline(disk, line, '\n');

				line = line.substr(0, std::min(num_bytes, std::abs(block_size-start_byte)));

				if (start_byte <= num_bytes) {
					num_bytes -= (block_size-start_byte-1);
				} else {
					num_bytes -= (block_size-((1+start_byte)%(block_size-1)));
				}

				last += line;
				start_byte = 0;
				traverse += 1;
				mini_traverse2 = count+1;

			}

		}

		std::cout << last << std::endl;

	}
}

int createFile(std::string fileName){
	int freeblock = 0;
	if(inode_map.count(fileName) == 0){
		int start = (3+(num_blocks/(block_size-1)));
		//std::cout << "start " << start << std::endl;
		for(int i = start; i<start+256; i++){
			if(free_block_list[i]=='0'){
				freeblock = i;
				free_block_list[i] = '1';
				break;
			}
		}
		if(freeblock != 0){
		inode this_node;
		this_node.file_name = fileName;
		this_node.file_size = 0;
		this_node.location = freeblock;
		inode_map[fileName] = this_node;
		}else{
		std::cout << "There is no room in the inode map for " << fileName << std::endl;
		}
	}else{
		std::cout << fileName << ": already exists" << std::endl;
	}

}

/*STILL NEED TO ACCOUNT FOR IF THE UNIX FILE IS TOO LARGE*/
void import(std::string ssfs_file, std::string unix_file){
	std::ifstream unix_fstream (unix_file, std::ifstream::binary);
	if(!unix_fstream) perror(unix_file.c_str());

	unix_fstream.seekg(0,unix_fstream.end);
	int unix_bytesize = unix_fstream.tellg();
	unix_fstream.seekg(0,unix_fstream.beg);

	int start = (3+(num_blocks/(block_size-1)+256));
	int blocks_left = 0;
	for(int i = start; i < free_block_list.size(); i++){
		if(free_block_list[i] == '0'){
			blocks_left++;
		}
	}

	if(unix_bytesize > (blocks_left*block_size)){
		std::cerr << unix_file << ": File is too large" << std::endl;
		return;
	}

	if(inode_map.count(ssfs_file) == 0){
	//if the file doesn't exist, create it
		createFile(ssfs_file);
	}

	char ch;
	int curr_byte = 1;
	//std::cout << "begin import" << std::endl;
	while(unix_fstream >> noskipws >> ch){
		//std::cout << ch;
		//write(ssfs_file,ch,curr_byte,1);
		curr_byte++;
	}
	//std::cout << std::endl;

}

void ssfsCat(std::string fileName){
	if(inode_map.count(fileName) != 0){
		read(fileName,1,inode_map[fileName].file_size);
	}else{
		std::cout << fileName << ": No such file" << std::endl;
	}
}

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
	if(atCapacity(target_inode.indirect_block, 0) == 0){
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
				int ind_blk = target_inode.indirect_block;
				ifstream idiskFile;
				ofstream odiskFile;
				idiskFile.open(disk_file_name);
				int pos = std::ios_base::beg + ((ind_blk -1) * block_size);
				idiskFile.seekg(pos);
				std::string ibLine;
				getline(idiskFile,ibLine,'\n');
				std::size_t found = ibLine.find("0",0);
				idiskFile.close();
				odiskFile.open(disk_file_name, std:: ofstream::out);
			//	this is the part I'm having a hard time with, gotta write
			//	to the disk here and getting that address and inserting
			//	the right number is what i was figuring out when my computer
			//	broke.
			//	seekp(pos + (int)found);
			//	ofs.write((itoa(block))),
				odiskFile.close();

			//
			}
			num_blocks--;


		}
	}
	if(num_blocks == 0){
		return 0;
	}
	//this is the double indirect level
	if(atCapacity(target_inode.indirect_block, 0) == 0){
	//	while(num_blocks != 0){
//return false;
	//		}
	//	}
	}
	return -1;
}

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

//	inode sample;
//	sample.file_name = "sample.txt";
//	sample.file_size = 128;
//	sample.location = (num_blocks/(block_size-1))+3;
//	sample.direct_blocks[0] = 320;
//	sample.direct_blocks[1] = 990;
//	sample.direct_blocks[2] = 900;
//	sample.double_indirect_block = 444;
//
//	inode sample2;
//	sample2.file_name = "sample2.txt";
//	sample2.file_size = 1574;
//	sample2.location = 3+(num_blocks/(block_size-1));
//	sample2.direct_blocks[0] = 333;
//	sample2.direct_blocks[1] = 991;
//	sample2.direct_blocks[2] = 1000;
//	sample2.direct_blocks[3] = 1004;
//	sample2.direct_blocks[4] = 500;
//	sample2.direct_blocks[5] = 501;
//	sample2.direct_blocks[6] = 599;
//	sample2.direct_blocks[7] = 903;
//	sample2.direct_blocks[8] = 999;
//	sample2.direct_blocks[9] = 1001;
//	sample2.direct_blocks[10] = 993;
//	sample2.direct_blocks[11] = 399;
//	sample2.indirect_block = 902;
//
//	inode_map["sample.txt"] = sample;
//	inode_map["sample2.txt"] = sample2;

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

std::string decimal_to_b60(int target) {
	std::string ret = "";
	int num;

	if (target >= 3600) {
		num = (target / 3600);

		if (num <= 9) {
			ret += std::to_string(num);

		} else {
			num += 55;

			if (num > 90) {
				num += 6;
			}

			ret += char(num);
		}
	} else {
		ret += "0";
	}

	target %= 3600;

	if (target >= 60) {
		num = (target / 60);

		if (num <= 9) {
			ret += std::to_string(num);
		} else {
			num += 55;

			if (num > 90) {
				num += 6;
			}

			ret += char(num);
		}
	} else {
		ret += "0";
	}

	target %= 60;

	num = target;

	if (num <= 9) {
		ret += std::to_string(num);
	} else {
		num += 55;

		if (num > 90) {
			num += 6;
		}

		ret += char(num);
	}

	return ret;

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
		const char * empty = decimal_to_b60(0).c_str();
		std::size_t found = ibLine.find(empty,0);
		if(found != std::string::npos){
			diskFile.close();
			return(0);
		}else{
			diskFile.close();
			return(1);
		}
	}else if(flag == 1){
		ifstream diskFile;
		diskFile.open(disk_file_name);
		int pos = std::ios_base::beg + ((lineNum -1) * block_size);
		diskFile.seekg(pos);
		std::string ibLine;
		getline(diskFile,ibLine,'\n');
		const char * empty = decimal_to_b60(0).c_str();
		std::size_t found = ibLine.find(empty,0);
		if(found != std::string::npos){
			diskFile.close();
			return(0);
		}else{
			int lastidblock;
			char * target = new char [ibLine.length()+1];
  			strcpy (target, ibLine.c_str());
			const char * p = strtok(target," ");
			while(p!=NULL){
				lastidblock = b60_to_decimal(p);
				p = strtok(NULL," ");
			}
			diskFile.close();
			atCapacity(lineNum,lastidblock);
		}
	}
}
