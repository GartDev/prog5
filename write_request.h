#include <stdio.h>
#include <iostream>

class write_request {
    public:
	write_request() {
		this->file_name = "";
		this->to_write = '\0';
		this->start_byte = 0;
		this->num_bytes = 0;
	}

	write_request(std::string file_name, int start_byte, int num_bytes) {
		this->file_name = file_name;
		this->start_byte = start_byte;
		this->num_bytes = num_bytes;
		this->to_write;

	}

        std::string file_name;
	std::string to_write;
        int start_byte;
        int num_bytes;
};
