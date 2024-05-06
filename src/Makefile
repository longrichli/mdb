CC := gcc

SERVER_SRC_FILE = mdb.c \
				  mdb_alloc.c \
				  mdb_log.c	\
				  mdb_tools.c \
				  mdb_eventloop.c \
				  mdb_list.c \
				  mdb_sds.c \
				  mdb_intset.c \
				  mdb_dict.c \
				  mdb_util.c \
				  mdb_t_string.c \
				  mdb_t_list.c \
				  mdb_t_hash.c \
				  mdb_t_set.c \
				  mdb_t_zset.c \
				  mdb_object.c \
				  mdb_config.c \
				  
CLIENT_SRC_FILE = mdb_cli.c \
				  mdb_alloc.c \
				  mdb_cli_lib.c \
				  mdb_util.c \
				  mdb_log.c \
				  mdb_tools.c
mdb-server:${SERVER_SRC_FILE}
	${CC} -o mdb-server ${SERVER_SRC_FILE} -std=c99
mdb-cli:${CLIENT_SRC_FILE}
	${CC} -o mdb-cli ${CLIENT_SRC_FILE}

all: mdb-server mdb-cli

clean:
	rm -f mdb-server mdb-cli
