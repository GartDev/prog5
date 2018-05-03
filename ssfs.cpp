/* https://is2-ssl.mzstatic.com/image/thumb/Video/v4/ed/79/b0/ed79b0c0-7617-a714-15be-2378cdb58221/source/1200x630bb.jpg */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <pthread.h>

pthread_cond_t fill, empty;
pthread_mutex_t mutex;

*inode createFile(string fileName,int flag);
void deleteFile(string fileName);
void list();
bool insert(*inode lilwayne);

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
	}
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

*inode createFile(string fileName,int flag){ //Pass 0 for flag if creating a non-existant file, Pass 1 if creating a file based on a Unix file
	if(flag == 0){
		//if(!fblist.empty()){
		inode *mynode(fileName, 0);
    	return(mynode);
		//}
	}else if(flag == 1){
		//int numblocks = filesize/blocksize ceiling
	//	if(numblocks <= freeblocks){

	}
	}
}
bool insert(*inode lilwayne){

}
void deleteFile(string fileName){
    //Get the inode from the inode map using fileName as the key
	//inode target = inodeMap[fileName]
    //return the blocks to the freelist
	//target.indirectblocks clear
	//target.directblocks clear
    //remove the inode from the inode map
	//inodemap.erase(fileName)
}

void list(){
	//for each element in inodemap, display the inode->name and inode->size
	//map<string,int>::iterator it = map.begin();
	//string fileName;
	//int block;
		//while(it!= map.end(){
		//fileName = it -> first;
		//fileSize = it -> second;
		//cout << "Name: " << fileName << "::Size: "<< filesize << " bytes" << endl;
//}



bool write(std::string fname, char to_write, int start_byte, int num_bytes){
	//i need some way to search the disk for a file by name, that would go here
	//for now I'll treat this pointer boy as if its a pointer to a block object indexed by integers
	Block * the_block;
	//check block size, return error if start_byte is out of bounds

	//check if file needs to be extended


}

void read(std::string fname, int start_byte, int num_bytes){
	//gotta find the block pointer
}
