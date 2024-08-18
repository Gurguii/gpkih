#include "SQLiteConnection.hpp"
#include "../../../libs/printing/printing.hpp" // DEBUGF()

SQLiteConnection::SQLiteConnection(std::string_view _dbPath):dbPath(_dbPath){
	DEBUGF(3, "SQLiteConnection({})", dbPath);
	if(sqlite3_open(dbPath.c_str(), &conn)){
		throw "Couldn't create sqlite3 db connection";
	};
};

SQLiteConnection::~SQLiteConnection(){
	if(conn != nullptr){
		sqlite3_close(conn);
	}
}

int SQLiteConnection::exec(const char *query, int(*callback)(void *,int,char**,char**), void *cbFirstArg, char **errmsg){
	if(sqlite3_exec(conn, query, callback, cbFirstArg, errmsg)){
		return 1;
	};
	return 0;
}

int SQLiteConnection::close(){
	return sqlite3_close(conn);
}

sqlite3* const SQLiteConnection::getConnection(){
	return conn;
}

const char *SQLiteConnection::getError(){
	return sqlite3_errmsg(conn);
}