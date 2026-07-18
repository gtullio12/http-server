# Specify the C compiler and flags
CFLAGS=-c -Wall -Wextra -fsanitize=address -g
CC=gcc
# Specify Object files
http_server_OBJ=http_server.o
connection_handler_OBJ=connection_handler.o
parse_header_OBJ=parse_header.o
parse_files_OBJ=parse_files.o
# Specify the executable
http_server_EXE=http_server
# Specify dependencies
http_server_DEPEND=http_server.c connection_handler.c parse_header.h
connection_handler_DEPEND=connection_handler.c parse_header.h
parse_header_DEPEND=parse_header.c parse_header.h
parse_files_DEPEND=parse_files.c parse_files.h

# Rule to build the object files
$(http_server_OBJ): $(http_server_DEPEND)
	@echo 'Building: $@'
	$(CC) $(CFLAGS) http_server.c -o http_server.o
$(connection_handler_OBJ): $(connection_handler_DEPEND)
	@echo 'Building: $@'
	$(CC) $(CFLAGS) connection_handler.c -o connection_handler.o
$(parse_header_OBJ): $(parse_header_DEPEND)
	@echo 'Building: $@'
	$(CC) $(CFLAGS) parse_header.c -o parse_header.o

$(parse_files_OBJ): $(parse_files_DEPEND)
	@echo 'Building: $@'
	$(CC) $(CFLAGS) parse_files.c -o parse_files.o


# Rule to build the executable
http_server: $(http_server_OBJ) $(connection_handler_OBJ) $(parse_header_OBJ) $(parse_files_OBJ)
	@echo 'Building: $@'
	$(CC) -fsanitize=address -g http_server.o parse_files.o connection_handler.o parse_header.o -o http_server

# Rule to clean the files
clean:
	rm -f *.o http_server
