#include <stdio.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "inode.h"

//Write <SSFS file name> <char> <start byte> <num bytes>
bool write(std::string fname, char to_write, int start_byte, int num_bytes);

void read(std::string fname, int start_byte, int num_bytes);
