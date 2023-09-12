OBJ_SERVER = server
OBJ_CLIENT = client
SRC = util.cpp
SERVER_FILE = Epoll.cpp InetAddress.cpp Socket.cpp Channel.cpp
all: $(OBJ_CLIENT) $(OBJ_SERVER)
$(OBJ_SERVER): server.cpp $(SRC) $(SERVER_FILE)
	g++ $^ -o $(OBJ_SERVER)

$(OBJ_CLIENT): client.cpp $(SRC)
	g++ $^ -o $(OBJ_CLIENT)

.PHONY: clean
clean:
	rm server client

