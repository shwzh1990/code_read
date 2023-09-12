OBJ_SERVER = server
OBJ_CLIENT = client
SRC = src/util.cpp
SERVER_FILE = src/EventLoop.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp src/Channel.cpp src/server.cpp
all: $(OBJ_CLIENT) $(OBJ_SERVER)
$(OBJ_SERVER): server.cpp $(SRC) $(SERVER_FILE)
	g++ $^ -o $(OBJ_SERVER)

$(OBJ_CLIENT): client.cpp $(SRC)
	g++ $^ -o $(OBJ_CLIENT)

.PHONY: clean
clean:
	rm server client

