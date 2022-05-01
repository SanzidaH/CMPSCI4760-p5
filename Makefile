CC = gcc
  # compiler flags:
  #  -g     - this flag adds debugging information to the executable file
  #  -Wall  - this flag is used to turn on most compiler warnings
CFLAGS  = -g -Wall -std=gnu99
 
  # The build target 
MASTER = oss
SLAVE = process
#SLAVE = test
 
all: $(MASTER) $(SLAVE)
 
#$(MASTER): $(MASTER).o
#	$(CC) $(CFLAGS) $(MASTER).c -o $(MASTER)

#$(SLAVE): $(SLAVE).c
#	$(CC) $(CFLAGS) $(SLAVE).c -o $(SLAVE)

.SUFFIXES: .c .o
.PHONY: all clean

clean:
	/bin/rm -f *.o $(MASTER) $(SLAVE)

