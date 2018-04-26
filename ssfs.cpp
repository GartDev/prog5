#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *diskOp(void *arg){}

int main(int argc, char **argv){
	cond_t fill, empty;
	mutex_t mutex;
	//fstream opfiles;
}
