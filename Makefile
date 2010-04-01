#
# Makefile 
#
CFLAGS= -O3 
CC = gcc
 
SOURCE_DIR = .

CSOURCES=	common.c  porta.c  four_mot.c arith.c inout.c portsort.c largecalc.c mp.c log.c

COBJECTS=	common.o  porta.o  four_mot.o arith.o inout.o portsort.o largecalc.o mp.o log.o

 
all: xporta valid

xporta: $(COBJECTS)	 		
	$(CC)  $(COBJECTS)  -o xporta

valid: valid.o common.o arith.o inout.o	 		
	$(CC)  valid.o common.o arith.o inout.o log.o -o valid

valid.o: $(SOURCE_DIR)/valid.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/valid.c

common.o: $(SOURCE_DIR)/common.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/common.c
porta.o: $(SOURCE_DIR)/porta.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/porta.c
four_mot.o: $(SOURCE_DIR)/four_mot.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/four_mot.c
arith.o: $(SOURCE_DIR)/arith.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/arith.c
inout.o: $(SOURCE_DIR)/inout.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/inout.c
portsort.o: $(SOURCE_DIR)/portsort.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/portsort.c
largecalc.o: $(SOURCE_DIR)/largecalc.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/largecalc.c
mp.o: $(SOURCE_DIR)/mp.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/mp.c
log.o: $(SOURCE_DIR)/log.c
	$(CC)  $(CFLAGS) -c $(SOURCE_DIR)/log.c




