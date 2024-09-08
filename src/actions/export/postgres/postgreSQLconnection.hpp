#include <libpq-fe.h>

#include <string_view>

class PostgreSQLConnection{
private:
	PGconn *connection = nullptr;
	const char *lastError = nullptr;
	bool closed = false;
	
public:
	PostgreSQLConnection() = delete;
	PostgreSQLConnection(std::string_view dbUri);
	~PostgreSQLConnection();
	int exec(std::string_view query);
	const PGconn* const getConnection();
	const char *getError();
	void close();
};