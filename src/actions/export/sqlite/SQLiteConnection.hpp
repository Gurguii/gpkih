#include <string>

#if __has_include("sqlite3.h")
#include <sqlite3.h>

class SQLiteConnection{
private:
	std::string dbPath;
	sqlite3 *conn = nullptr;
public:
	SQLiteConnection(std::string_view _dbPath);
	~SQLiteConnection();
	int exec(const char *query, int(*callback)(void *,int,char**,char**) = nullptr, void *cbFirstArg = nullptr, char **errmsg = nullptr);
	int close();
	sqlite3* const getConnection();
	const char *getError();
};

#endif