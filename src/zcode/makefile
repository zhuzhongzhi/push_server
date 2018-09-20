############################################################# 
# Makefile for shared library.
# 编译动态链接库
#############################################################
#set your own environment option
CC = g++
CC_FLAG = -g -std=c++11 -D_LINUX

#set your inc and lib
INC = -I./ -I../zbase -I../zcode/mqtt
LIB = -lpthread

#make target lib and relevant obj 
PRG1 = $(WORK_DIR)/lib/nouse1
SRC1 = ${wildcard ./mqtt/*.cpp}
OBJ1 = ${patsubst %.cpp, %.o, $(SRC1)}

PRG2 = $(WORK_DIR)/lib/nouse2
SRC2 = ${wildcard ./xmpp/*.cpp}
OBJ2 = ${patsubst %.cpp, %.o, $(SRC2)}

PRG3 = $(WORK_DIR)/lib/nouse3
SRC3 = ${wildcard ./customer/*.cpp}
OBJ3 = ${patsubst %.cpp, %.o, $(SRC3)}
 
#all target
all:$(PRG1) $(PRG3)

$(PRG1):$(OBJ1)
	@echo "make mqtt code ok"

$(PRG2):$(OBJ2)
	@echo "make xmpp code ok"

$(PRG3):$(OBJ3)
	@echo "make customer code ok"	
	
.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $(WORK_DIR)/src/obj/${notdir $*}.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	@current_dir=`pwd`
	@cd $(WORK_DIR)/src/obj
	rm -f $(OBJ1) $(OBJ2) $(OBJ3)
	@cd $(current_dir)