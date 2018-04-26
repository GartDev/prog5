#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
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
	for (std::string line; std::getline(opfile, line);) {
        std::cout << line << std::endl;
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
