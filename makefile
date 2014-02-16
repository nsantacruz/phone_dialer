#Noah Santacruz
#Prof Kirtman 2013 ECE 161

CC = gcc
CFLAGS = -wall

all: kirtstuffn
	./kirtstuffn
	
kirtstuffn: kirtstuffn.o
	$(CC) $(CFLAGS) -o kirtstuffn kirtstuffn.o
	
	

