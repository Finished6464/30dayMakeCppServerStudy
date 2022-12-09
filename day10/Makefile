CXX = g++
# CXXFLAGS = -Wall -Werror -Wextra -pedantic -std=c++17 -g -fsanitize=address
CXXFLAGS	:= -std=c++17 -Wall -Wextra -g

RM = rm -f

VPATH = ../day06

SERVER = run_server
CLIENT = client
OBJECTS = run_server.o server.o util.o epoll_x.o inetaddress.o socket_x.o channel.o

all: $(SERVER)
	@echo Executing 'all' complete!

$(SERVER): run_server.o server.o util.o epoll_x.o inetaddress.o socket_x.o channel.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

# $(CLIENT): client.o util.o inetaddress.o socket_x.o
# 	$(CXX) $(CXXFLAGS) $^ -o $@

run_server.o: server.h
server.o: server.h util.h epoll_x.h inetaddress.h socket_x.h channel.h
# client.o: socket_x.h util.h
util.o : util.h
epoll_x.o : epoll_x.h util.h
inetaddress.o : inetaddress.h
socket_x.o : socket_x.h util.h inetaddress.h
channel.o: channel.h

.PHONY: clean
clean:
	-$(RM) $(SERVER)
	-$(RM) $(OBJECTS)
	@echo Cleanup complete!