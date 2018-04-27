#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <pthread.h>

pthread_cond_t fill, empty;
pthread_mutex_t mutex;

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
	while(!opfile.eof()){
		std::getline(opfile,line);
        std::cout << line << std::endl;
		istringstream line_stream(line);
		std::string command;
		line_stream >> command;
		std::string ssfs_file;
		if(command == "CREATE"){
			line_stream >> ssfs_file;
			std::string unix_file;
			line_stream >> unix_file;
			//disk.create(ssfs_file,unix_file)

		}else if(command == "IMPORT"){
			line_stream >> ssfs_file;

		}else if(command == "CAT"){
			line_stream >> ssfs_file;

		}else if(command == "DELETE"){
			line_stream >> ssfs_file;

		}else if(command == "WRITE"){
			line_stream >> ssfs_file;

		}else if(command == "READ"){
			line_stream >> ssfs_file;

		}else if(command == "LIST"){

		}else if(command == "SHUTDOWN"){
			std::cout << "Shutting down " << file_name << "..." << std::endl;
			pthread_exit(NULL);
		}else{
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
