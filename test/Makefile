log_test:
	gcc -o log_test mdb_log_test.c ../src/mdb_log.c ../src/mdb_tools.c -std=c99
sds_test:
	gcc -o sds_test mdb_sds_test.c ../src/mdb_log.c ../src/mdb_sds.c ../src/mdb_alloc.c -std=c99
tools_test:
	gcc -o tools_test mdb_tools_test.c ../src/mdb_log.c ../src/mdb_sds.c ../src/mdb_alloc.c ../src/mdb_tools.c -std=c99
list_test:
	gcc -o list_test mdb_list_test.c ../src/mdb_list.c ../src/mdb_log.c ../src/mdb_alloc.c ../src/mdb_tools.c -std=c99 -g
dict_test:
	gcc -Wall -o dict_test mdb_dict_test.c ../src/mdb_dict.c ../src/mdb_log.c ../src/mdb_alloc.c ../src/mdb_tools.c -std=c99 -g
intset_test:
	gcc -Wall -o intset_test mdb_intset_test.c ../src/mdb_intset.c ../src/mdb_log.c ../src/mdb_alloc.c ../src/mdb_tools.c -std=c99 -g

skiplist_test:
	gcc -Wall -o skiplist_test skiplist.c ../src/mdb_sds.c ../src/mdb_object.c ../src/mdb_alloc.c ../src/mdb_tools.c ../src/mdb_dict.c ../src/mdb_util.c ../src/mdb_log.c ../src/mdb_intset.c  \
	../src/mdb_list.c ../src/mdb_skiplist.c -std=c99 -g	
clean:
	rm -f *test