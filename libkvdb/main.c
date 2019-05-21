#include "kvdb.h"

int main() {
	kvdb_t db;
	
	kvdb_open(&db, "a.db");
	kvdb_put(&db,"Me","you");
	//kvdb_put(&db,"hello","world");
	//char *s = kvdb_get(&db,"hello");
	//printf("%s",s);

	kvdb_close(&db);
	
}
