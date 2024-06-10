CC := gcc

exec_cmd:exec_cmd.c
	${CC} -o exec_cmd exec_cmd.c -lmdbcli -std=c99
handle_num:handle_num.c
	${CC} -o handle_num handle_num.c -lmdbcli -std=c99
all: handle_num exec_cmd
clean: 
	rm -f handle_num exec_cmd
