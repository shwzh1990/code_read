OBJ_SERVER = server
OBJ_CLIENT = client
SRC = util.cpp
all: $(OBJ_CLIENT) $(OBJ_SERVER)
$(OBJ_SERVER): server.cpp $(SRC) 
	g++ $^ -o $(OBJ_SERVER)

$(OBJ_CLIENT): client.cpp $(SRC)
	g++ $^ -o $(OBJ_CLIENT)

.PHONY: clean
clean:
	rm server client

