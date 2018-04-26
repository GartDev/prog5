# "make" to build and run both executables in order
# "make disk" to build and run ssfs_mkdsk
# "make ssfs" to build and run ssfs
# "make clean" to delete all object and binary files

all : ssfs ssfs_mkdsk

disk : ssfs_mkdsk

ssfs_mkdsk : mkdsk.o
	g++ -Wall -g mkdsk.o -o ssfs_mkdsk

mkdsk.o : mkdsk.cpp
	g++ -c mkdsk.cpp

ssfs : ssfs.o
	g++ -Wall -g ssfs.o -pthread -o ssfs

ssfs.o : ssfs.cpp
	g++ -c ssfs.cpp

clean :
	rm -f *.o ssfs_mkdsk ssfs DISK

run: ssfs ssfs_mkdsk
	./ssfs_mkdsk 1024 128 DISK
	./ssfs DISK thread1.txt thread2.txt thread3.txt thread4.txt
