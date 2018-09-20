###########################################
#Makefile for simple programs
###########################################
LIB=-lpthread -lrt -ldl -lnsl -lACE -lmongoclient -lmysqlclient -lboost_thread-mt -lboost_regex -lboost_filesystem -lboost_system -lboost_program_options -lboost_thread -ludt -lssl -lcrypto -lsasl2
CC=g++

PRG=$(WORK_DIR)/bin/push_server
SUBDIRS=apns database manager service zbase zcode
SRC = ${notdir ${wildcard ./apns_manager/*.cpp}} \
${notdir ${wildcard ./apns/*.cpp}} \
${notdir ${wildcard ./database/*.cpp}} \
${notdir ${wildcard ./database/mongodb/*.cpp}} \
${notdir ${wildcard ./database/mysql/*.cpp}} \
${notdir ${wildcard ./manager/*.cpp}} \
${notdir ${wildcard ./service/*.cpp}} \
${notdir ${wildcard ./zbase/*.cpp}} \
${notdir ${wildcard ./zcode/mqtt/*.cpp}}

OBJ = ${patsubst %.cpp, ./obj/%.o, $(SRC)}

$(PRG):
	@for dir in $(SUBDIRS); do \
  	cd $$dir && make; cd .. > /dev/null; \
  done
	$(CC) -o $(PRG) $(OBJ) $(LIB)

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(WORK_DIR)/src/obj/*.o $(PRG)
