############################################################# 
# Makefile for shared library.
# 编译动态链接库
#############################################################
#set your own environment option
CC = g++
CC_FLAG = -g -std=c++11 -fPIC -Wall -Wsign-compare -Wno-deprecated -D_LINUX

#set your inc and lib
INC = -I./
LIB = -lpthread

#make target lib and relevant obj 
PRG = $(WORK_DIR)/lib/nouse
SRC = ${wildcard *.cpp}
OBJ = ${patsubst %.cpp, %.o, $(SRC)}

#all target
all:$(PRG)

$(PRG):$(OBJ)
	@echo "make xbase ok"

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $(WORK_DIR)/src/obj/${notdir $*}.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	@current_dir=`pwd`
	@cd $(WORK_DIR)/src/obj
	rm -f $(OBJ)
	@cd $(current_dir)