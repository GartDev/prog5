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
	for (std::string line; std::getline(opfile, line);) {
        std::cout << line << std::endl;
    }
	opfile.close();
	pthread_exit(NULL);
}

int main(int argc, char **argv){
	pthread_t *threads = new pthread_t[argc-2];
	int rc;
	for(int i = 2; i < (argc-1); i++){
		rc = pthread_create(&threads[i], NULL, read_file, (void*)argv[i]);
		if(rc){
			std::cout << "Unable to create thread." << std::endl;
			exit(0);
		}
	}
	delete [] threads;

	pthread_exit(NULL);
}
