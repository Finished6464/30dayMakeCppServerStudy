CXX = g++
# CXXFLAGS = -Wall -Werror -Wextra -pedantic -std=c++17 -g -fsanitize=address
CXXFLAGS	:= -std=c++17 -Wall -Wextra -g

RM = rm -f

VPATH = ../day06

SERVER = run_server
CLIENT = client
OBJECTS = run_server.o server.o client.o connection.o eventloop.o util.o epoll_x.o inetaddress.o socket_x.o channel.o

all: $(SERVER) $(CLIENT)
	@echo Executing 'all' complete!

$(SERVER): run_server.o server.o connection.o eventloop.o util.o epoll_x.o inetaddress.o socket_x.o channel.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

$(CLIENT): client.o connection.o socket_x.o util.o inetaddress.o channel.o epoll_x.o
	$(CXX) $(CXXFLAGS) $^ -o $@ 

run_server.o: server.h
server.o: server.h connection.h eventloop.h 
eventloop.o: eventloop.h channel.h
connection.o: connection.h socket_x.h eventloop.h util.h
client.o: connection.h
util.o : util.h
epoll_x.o : epoll_x.h util.h
inetaddress.o : inetaddress.h
socket_x.o : socket_x.h util.h inetaddress.h
channel.o: channel.h

.PHONY: clean
clean:
	-$(RM) $(SERVER)
	-$(RM) $(CLIENT)
	-$(RM) $(OBJECTS)
	@echo Cleanup complete!