#


P = fileSenderServer

OBJECTS = csapp.o \
		logger.o \
		main.o \
		packetUtils.o
		
CFLAGS = 
LDFLAGS = -lpthread
CC=gcc
RM = rm -f

all: $(P)

$(P): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	
	
.PHONY : clean
clean :
	$(RM) $(P)
	$(RM) *.o
	
clear :
	$(RM) *.log
	$(RM) *.jpg
	
clean_all : clean clear