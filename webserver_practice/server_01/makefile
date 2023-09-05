OBJ_SERVER = server
OBJ_CLIENT = client
all: $(OBJ_CLIENT) $(OBJ_SERVER)
$(OBJ_SERVER): server.c 
	gcc server.c -o $(OBJ_SERVER)

$(OBJ_CLIENT): client.c
	gcc client.c -o $(OBJ_CLIENT)

.PHONY: clean
clean:
	-rm server client

