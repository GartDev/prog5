# "make" to build and run both executables in order
# "make disk" to build and run ssfs_mkdsk
# "make sys" to build and run ssfs
# "make clean" to delete all object and binary files

run : disk sys



disk : ssfs_mkdsk	

ssfs_mkdsk : mkdsk.o
	g++ -Wall -g mkdsk.o -o ssfs_mkdsk

mkdsk.o :   
	g++ -c mkdsk.cpp



sys : ssfs

ssfs : ssfs.o
	g++ -Wall -g ssfs.o -o ssfs

ssfs.o :
	g++ -c ssfs.cpp



clean :
	rm -f *.o ssfs_mkdsk ssfs
