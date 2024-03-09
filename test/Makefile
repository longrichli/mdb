log_test:
	gcc -o log_test mdb_log_test.c ../src/mdb_log.c ../src/mdb_tools.c -std=c99
sds_test:
	gcc -o sds_test mdb_sds_test.c ../src/mdb_log.c ../src/mdb_sds.c ../src/mdb_alloc.c -std=c99
tools_test:
	gcc -o tools_test mdb_tools_test.c ../src/mdb_log.c ../src/mdb_sds.c ../src/mdb_alloc.c ../src/mdb_tools.c -std=c99
clean:
	rm -f *test