SERVER_SRC_FILE = mdb.c \
				  mdb_alloc.c \
				  mdb_log.c	\
				  mdb_tools.c
CLIENT_SRC_FILE = mdb_cli.c
mdb-server:${SERVER_SRC_FILE}
	gcc -o mdb-server ${SERVER_SRC_FILE}
mdb-cli:${CLIENT_SRC_FILE}
	gcc -o mdb-cli ${CLIENT_SRC_FILE}

all: mdb-server mdb-cli
