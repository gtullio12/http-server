# Specify the C compiler and flags
CFLAGS=-c -Wall -Wextra
CC=gcc

# Specify Object files
http_server_OBJ=http_server.o
connection_handler_OBJ=connection_handler.o

# Specify the executable
http_server_EXE=http_server

# Specify dependencies
http_server_DEPEND=http_server.c connection_handler.c

# Rule to build the object files
$(http_server_OBJ): $(http_server_DEPEND)
	@echo 'Building: $@'
	$(CC) $(CFLAGS) http_server.c -o http_server.o

$(connection_handler_OBJ): connection_handler.c
	@echo 'Building: $@'
	$(CC) $(CFLAGS) connection_handler.c -o connection_handler.o

# Rule to build the executable
http_server: $(http_server_OBJ) $(connection_handler_OBJ) 
	@echo 'Building: $@'
	$(CC) http_server.o connection_handler.o -o http_server

# Rule to clean the files
clean:
	rm -f *.o http_server
