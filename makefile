CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

# List of source files
SRCS_SERVER = server.c
SRCS_USER = user.c
SRCS_COMMON = common.h

# Object files
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_USER = $(SRCS_USER:.c=.o)

# Executable names
EXEC_SERVER = server
EXEC_USER = user

# Default target
all: $(EXEC_SERVER) $(EXEC_USER)

# Compile server program
$(EXEC_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) -o $(EXEC_SERVER)

# Compile user program
$(EXEC_USER): $(OBJS_USER)
	$(CC) $(CFLAGS) $(OBJS_USER) -o $(EXEC_USER)

# Compile server object files
%.o: %.c $(SRCS_COMMON)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile user object files
%.o: %.c $(SRCS_COMMON)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean command
clean:
	rm -f $(OBJS_SERVER) $(OBJS_USER) $(EXEC_SERVER) $(EXEC_USER)
