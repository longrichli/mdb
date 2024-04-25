CC := gcc

SERVER_SRC_FILE = mdb.c \
				  mdb_alloc.c \
				  mdb_log.c	\
				  mdb_tools.c \
				  mdb_eventloop.c 
CLIENT_SRC_FILE = mdb_cli.c
mdb-server:${SERVER_SRC_FILE}
	${CC} -o mdb-server ${SERVER_SRC_FILE} -std=c99
mdb-cli:${CLIENT_SRC_FILE}
	${CC} -o mdb-cli ${CLIENT_SRC_FILE}

all: mdb-server mdb-cli
