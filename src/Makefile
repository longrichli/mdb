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
				  mdb_skiplist.c \
				  mdb_aof.c \
				  mdb_cli_lib.c \
				  
CLIENT_SRC_FILE = mdb_cli.c \
				  mdb_alloc.c \
				  mdb_cli_lib.c \
				  mdb_util.c \
				  mdb_log.c \
				  mdb_tools.c
CLIENT_SO_SRC_FILE = mdb_alloc.c \
					 mdb_cli_lib.c \
					 mdb_util.c \
					 mdb_log.c \
					 mdb_tools.c 
../bin/mdb-server:${SERVER_SRC_FILE}
	${CC} -Wall -o ../bin/mdb-server ${SERVER_SRC_FILE} -std=c99
../bin/mdb-cli:${CLIENT_SRC_FILE}
	${CC} -Wall -o ../bin/mdb-cli ${CLIENT_SRC_FILE}
../lib/libmdbcli.so:
	${CC} -Wall -shared -o ../lib/libmdbcli.so -fPIC ${CLIENT_SO_SRC_FILE}
all: ../bin/mdb-server ../bin/mdb-cli ../lib/libmdbcli.so

clean:
	rm -f ../bin/mdb-server ../bin/mdb-cli ../lib/libmdbcli.so
