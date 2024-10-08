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
	PostgreSQLConnection(std::string_view host, std::string_view user, std::string_view passwd, std::string_view port = "5432");

	~PostgreSQLConnection();
	int exec(std::string_view query);
	const PGconn* const getConnection();
	const char *getError();
	void close();
};