gcc -Wall -o myfuse myfuse.c -lfuse -ljson-c -lpthread -D_FILE_OFFSET_BITS=64